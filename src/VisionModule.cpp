/******************************************************************************/
/**
 * @file    VisionModule.cpp
 * @addtogroup VISION
 * @brief   Modulo de processamento de imagem e Inteligencia Artificial.
 * @author  Gonçalves
 * @details
 * \n <b>Ferramentas:</b>
 * - C++17, OpenCV (DNN).
 *
 * \n <b>Dependencias:</b>
 * - Arquivo do modelo ONNX (yolov8n.onnx).
 *
 * \n <b>Observacoes:</b>
 * - Roda a inferencia do YOLO em uma thread dedicada (Consumidor).
 * - Utiliza OpenVINO para aceleracao otimizada via CPU.
 * - Sincroniza entrega de frames sem bloquear a thread principal usando 
 *   Condition Variables e Mutex.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "VisionModule.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>

/*******************************************************************************
 * DEFINES LOCAIS E CONSTANTES
 ******************************************************************************/

/// Dimensoes do tensor de entrada exigidas pelo modelo exportado
static constexpr int dINPUT_SIZE = 640;

/// IDs de classe do dataset COCO aceitas pelo sistema
static constexpr int dCLASS_ID_CELLPHONE = 73; // Caderno/Celular

/// Limiares de confianca e sobreposicao (NMS)
static constexpr float dCONFIDENCE_THRESHOLD = 0.30f;
static constexpr float dNMS_THRESHOLD = 0.30f;

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/

/// Variaveis internas do modulo de Visao Computacional
static struct
{
    /// Rede neural carregada via OpenCV DNN
    cv::dnn::Net net;

    /// Buffer de entrada compartilhado com o Orchestrator (Produtor)
    cv::Mat inputFrame;
    std::mutex inputMutex;
    std::condition_variable inputCv;
    bool hasNewFrame{false};

    /// Buffer de saida com os resultados mais recentes da IA
    std::vector<cv::Rect> resultBoxes;
    int resultCount{0};
    std::mutex resultMutex;

    /// Controle de ciclo de vida da thread de inferencia
    std::atomic<bool> isRunning{false};
    std::thread visionThread;
} visionCtx;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

static void RunInference(const cv::Mat& frame, std::vector<cv::Rect>& outBoxes, int& outCount);
static void VisionLoop(void);

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief  Inicializa a rede neural e prepara o motor de inferencia.
 *  @param  modelPath: Caminho absoluto ou relativo para o arquivo .onnx.
 *  @retval true se a rede foi carregada com sucesso, false caso contrario.
 *  @details Tenta alocar o backend OpenVINO para CPU. Executa um "warm-up"
 *           com um frame vazio para forcar a compilacao dos grafos pela engine
 *           e evitar gargalos no primeiro frame real do sistema. Em caso de 
 *           falha, faz fallback seguro para o backend padrao do OpenCV.
 ******************************************************************************/
bool Vision_Init(const std::string& modelPath) 
{
    try 
    {
        visionCtx.net = cv::dnn::readNetFromONNX(modelPath);
        visionCtx.net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        visionCtx.net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);

        // --- WARM-UP ---
        cv::Mat dummy = cv::Mat::zeros(dINPUT_SIZE, dINPUT_SIZE, CV_8UC3);
        cv::Mat blob;
        cv::dnn::blobFromImage(dummy, blob, 1.0 / 255.0, cv::Size(dINPUT_SIZE, dINPUT_SIZE), cv::Scalar(), true, false);
        visionCtx.net.setInput(blob);
        std::vector<cv::Mat> warmupOut;
        visionCtx.net.forward(warmupOut, visionCtx.net.getUnconnectedOutLayersNames());

        std::cout << "[VISION] IA carregada com motor CUDA (RTX 4050 ativada)." << std::endl;
        return true;

    } 
    catch (const cv::Exception& e) 
    {
        std::cerr << "[VISION] CUDA falhou (" << e.what() << "). Tentando fallback para CPU..." << std::endl;

        try 
        {
            visionCtx.net = cv::dnn::readNetFromONNX(modelPath);
            visionCtx.net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            visionCtx.net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

            cv::Mat dummy = cv::Mat::zeros(dINPUT_SIZE, dINPUT_SIZE, CV_8UC3);
            cv::Mat blob;
            cv::dnn::blobFromImage(dummy, blob, 1.0 / 255.0, cv::Size(dINPUT_SIZE, dINPUT_SIZE), cv::Scalar(), true, false);
            visionCtx.net.setInput(blob);
            std::vector<cv::Mat> warmupOut;
            visionCtx.net.forward(warmupOut, visionCtx.net.getUnconnectedOutLayersNames());

            std::cout << "[VISION] Fallback para OpenCV Backend padrao (warm-up concluido)." << std::endl;
            return true;

        } 
        catch (const cv::Exception& ex) 
        {
            std::cerr << "[VISION] Falha critica ao carregar modelo: " << ex.what() << std::endl;
            return false;
        }
    }
}

/******************************************************************************/
/** @brief  Inicia a thread de processamento da IA.
 *  @details Muda a flag de execucao e dispara a funcao VisionLoop em background.
 ******************************************************************************/
void Vision_Start(void) 
{
    visionCtx.isRunning = true;
    visionCtx.visionThread = std::thread(VisionLoop);
}

/******************************************************************************/
/** @brief  Para a thread da IA de forma segura.
 *  @details Altera a flag de execucao, acorda a thread bloqueada e aguarda
 *           o join().
 ******************************************************************************/
void Vision_Stop(void) 
{
    visionCtx.isRunning = false;
    visionCtx.inputCv.notify_all(); 
    
    if (visionCtx.visionThread.joinable()) 
    {
        visionCtx.visionThread.join();
    }
}

/******************************************************************************/
/** @brief  Submete um novo frame capturado para ser processado pela IA.
 *  @param  frame: Matriz da imagem recem-capturada pela camera.
 *  @details Funcao nao-bloqueante. Se a IA ainda estiver ocupada processando
 *           um frame anterior, o frame velho no buffer sera sobrescrito pelo 
 *           novo, garantindo tempo real.
 ******************************************************************************/
void Vision_SubmitFrame(const cv::Mat& frame) 
{
    std::lock_guard<std::mutex> lock(visionCtx.inputMutex);
    frame.copyTo(visionCtx.inputFrame);
    visionCtx.hasNewFrame = true;
    visionCtx.inputCv.notify_one(); 
}

/******************************************************************************/
/** @brief  Recupera as ultimas detecoes geradas pela rede neural.
 *  @param  boxes: Vetor por referencia que recebera os retangulos detectados.
 *  @param  vehicleCount: Variavel por referencia que recebera o total de alvos.
 *  @details O acesso as variaveis e protegido por Mutex para nao corromper
 *           a memoria da tela de exibicao (Orchestrator).
 ******************************************************************************/
void Vision_GetOverlay(std::vector<cv::Rect>& boxes, int& vehicleCount) 
{
    std::lock_guard<std::mutex> lock(visionCtx.resultMutex);
    boxes = visionCtx.resultBoxes;
    vehicleCount = visionCtx.resultCount;
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/******************************************************************************/
/** @brief  Executa o processamento do tensor matematico da imagem (Forward).
 *  @param  frame: Imagem de origem da camera.
 *  @param  outBoxes: Vetor onde os retangulos filtrados serao salvos.
 *  @param  outCount: Contador do numero de objetos detectados na zona.
 *  @details Realiza redimensionamento, inferencia, correcao de transposta 
 *           de matriz e aplicacao de limites geometricos e de confianca (NMS).
 ******************************************************************************/
static void RunInference(const cv::Mat& frame, std::vector<cv::Rect>& outBoxes, int& outCount) 
{
    cv::Rect roi(
        static_cast<int>(frame.cols * 0.25),
        static_cast<int>(frame.rows * 0.25),
        static_cast<int>(frame.cols * 0.5),
        static_cast<int>(frame.rows * 0.5));

    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1.0 / 255.0, cv::Size(dINPUT_SIZE, dINPUT_SIZE), cv::Scalar(), true, false);
    visionCtx.net.setInput(blob);

    std::vector<cv::Mat> outputs;
    visionCtx.net.forward(outputs, visionCtx.net.getUnconnectedOutLayersNames());

    cv::Mat raw = outputs[0];

    int numAttrs, numBoxes;
    if (raw.dims == 3) {
        numAttrs = raw.size[1];
        numBoxes = raw.size[2];
    } else if (raw.dims == 2) {
        numAttrs = raw.size[0];
        numBoxes = raw.size[1];
    } else {
        std::cerr << "[VISION] Formato de saida inesperado, dims=" << raw.dims << std::endl;
        outCount = 0;
        outBoxes.clear();
        return;
    }

    cv::Mat outMat(numAttrs, numBoxes, CV_32F, raw.ptr<float>());
    cv::Mat predictions = outMat.t();

    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    float x_factor = frame.cols / static_cast<float>(dINPUT_SIZE);
    float y_factor = frame.rows / static_cast<float>(dINPUT_SIZE);
    int numClasses = numAttrs - 4;

    double bestScoreOverall = 0.0;
    int bestClassOverall = -1;

    for (int i = 0; i < predictions.rows; i++) 
    {
        float* row = predictions.ptr<float>(i);
        float* classesScores = row + 4;

        cv::Mat scores(1, numClasses, CV_32F, classesScores);
        cv::Point classIdPoint;
        double maxScore;
        cv::minMaxLoc(scores, 0, &maxScore, 0, &classIdPoint);

        if (maxScore > dCONFIDENCE_THRESHOLD) 
        {
            int classId = classIdPoint.x; 
        
            if (classId == 73 || classId == 67 || classId == 65) 
            {
                float cx = row[0] * x_factor;
                float cy = row[1] * y_factor;
                float w  = row[2] * x_factor;
                float h  = row[3] * y_factor;
                int left = static_cast<int>(cx - w / 2);
                int top  = static_cast<int>(cy - h / 2);
                
                boxes.push_back(cv::Rect(left, top, static_cast<int>(w), static_cast<int>(h)));
                confidences.push_back(static_cast<float>(maxScore));
            }
        }
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, dCONFIDENCE_THRESHOLD, dNMS_THRESHOLD, indices);

    outBoxes.clear();
    outCount = 0;
    
    for (int idx : indices) 
    {
        cv::Rect box = boxes[idx];
        cv::Point center(box.x + box.width / 2, box.y + box.height / 2);
        
        if (roi.contains(center)) 
        {
            outCount++;
            outBoxes.push_back(box);
        }
    }
}

/******************************************************************************/
/** @brief  Laco principal da thread de IA.
 *  @details Aguarda ate que um frame novo seja injetado e acordado via
 *           Condition Variable. Calcula o tempo de inferencia para auditoria
 *           de performance e publica os resultados da rede.
 ******************************************************************************/
static void VisionLoop(void) 
{
    while (visionCtx.isRunning) 
    {
        cv::Mat frame;

        {
            std::unique_lock<std::mutex> lock(visionCtx.inputMutex);
            visionCtx.inputCv.wait(lock, [] { return visionCtx.hasNewFrame || !visionCtx.isRunning; });
            
            if (!visionCtx.isRunning) break; 

            frame = visionCtx.inputFrame.clone();
            visionCtx.hasNewFrame = false;
        }

        auto t0 = std::chrono::steady_clock::now();

        std::vector<cv::Rect> boxes;
        int count = 0;
        RunInference(frame, boxes, count);

        auto t1 = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::cout << "[VISION] Inferencia: " << ms << " ms" << std::endl;

        std::lock_guard<std::mutex> lock(visionCtx.resultMutex);
        visionCtx.resultBoxes = std::move(boxes);
        visionCtx.resultCount = count;
    }
}

/** @} DOXYGEN GROUP TAG END OF FILE */