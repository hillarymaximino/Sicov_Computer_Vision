/******************************************************************************/
/**
 * @file    Orchestrator.cpp
 * @addtogroup ORCHESTRATOR
 * @brief   Modulo principal de orquestracao e interface grafica.
 * @author  Gonçalves
 * @details
 * \n <b>Ferramentas:</b>
 * - C++17, OpenCV.
 *
 * \n <b>Dependencias:</b>
 * - Bsp (Camera/Hardware).
 * - VisionModule (Inteligencia Artificial).
 *
 * \n <b>Observacoes:</b>
 * - Coordena a inicializacao da captura de video e da IA.
 * - Gerencia o laco principal (main loop), desenha as interfaces (ROI e deteccoes).
 * - Exibe os resultados em tempo real em uma janela do OpenCV.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Orchestrator.hpp"
#include "Bsp.hpp"          // Precisa conhecer a câmera
#include "VisionModule.hpp" // Precisa conhecer a IA
#include <iostream>

/*******************************************************************************
 * DEFINES LOCAIS E CONSTANTES
 ******************************************************************************/
// Nao ha defines locais neste modulo.

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
// Nao ha estruturas de dados locais neste modulo.

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/
// Nao ha prototipos locais neste modulo.

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief  Inicializa os subsistemas na ordem correta.
 * @param  streamUrl: Caminho ou link RTSP/HTTP da camera.
 * @param  modelPath: Caminho do arquivo de modelo ONNX.
 * @retval true se todos os modulos iniciarem com sucesso, false caso contrario.
 * @details Tenta iniciar primeiro a camera (BSP). Se falhar, aborta.
 * Em seguida, inicia a IA. Se a IA falhar, desliga a camera antes
 * de abortar para evitar vazamento de memoria ou threads orfas.
 ******************************************************************************/
bool Orchestrator_Init(const std::string& streamUrl, const std::string& modelPath) 
{
    std::cout << "[ORCHESTRATOR] Inicializando subsistemas..." << std::endl;

    // Liga a câmera. Se der erro, aborta o sistema inteiro (return false).
    if (!Bsp_InitCamera(streamUrl)) {
        return false;
    }

    // Liga o cérebro que é o vision.
    if (!Vision_Init(modelPath)) {
        // Se a câmera ligou, mas a IA falhou, precisamos desligar a câmera antes de abortar.
        Bsp_ReleaseCamera(); 
        return false;
    }

    std::cout << "[ORCHESTRATOR] Modulos inicializados com sucesso." << std::endl;
    return true;
}

/******************************************************************************/
/** @brief  Loop principal que mantem o sistema vivo.
 * @details Cria a janela de exibicao, coordena a troca de frames entre
 * o BSP e o VisionModule, aplica as sobreposicoes (ROI, bounding boxes,
 * textos) e gerencia o encerramento seguro pelo usuario (tecla ESC).
 ******************************************************************************/
void Orchestrator_Run() 
{
    cv::Mat frame;
    bool isRunning = true;

    // 1. Cria a janela redimensionável
    cv::namedWindow("SICOV - Monitoramento de celulares", cv::WINDOW_NORMAL);

    // 2. Redimensiona para metade da largura (1920 / 2 = 960) e altura total (1080)
    cv::resizeWindow("SICOV - Monitoramento de celulares", 960, 1080);

    // 3. Posiciona a janela colada na lateral esquerda (x = 0, y = 0)
    cv::moveWindow("SICOV - Monitoramento de celulares", 0, 0);

    Vision_Start();

    while (isRunning) 
    {
        if (!Bsp_GetFrame(frame)) 
        {
            std::cerr << "[ORCHESTRATOR] Conexao com a camera perdida." << std::endl;
            break;
        }
        
        if (frame.empty()) 
        {
            cv::waitKey(10);
            continue;
        }

        Vision_SubmitFrame(frame); // não bloqueia, só entrega o frame mais novo

        std::vector<cv::Rect> boxes;
        int cellCount = 0;
        Vision_GetOverlay(boxes, cellCount); // pega o último resultado pronto

        cv::Rect roi(static_cast<int>(frame.cols * 0.25), static_cast<int>(frame.rows * 0.25),
                     static_cast<int>(frame.cols * 0.5), static_cast<int>(frame.rows * 0.5));

        cv::rectangle(frame, roi, cv::Scalar(255, 0, 0), 2);

        for (const auto& box : boxes) 
        {
            cv::Point center(box.x + box.width / 2, box.y + box.height / 2);
            cv::rectangle(frame, box, cv::Scalar(0, 255, 255), 2);
            cv::circle(frame, center, 4, cv::Scalar(0, 0, 255), -1);
        }

        cv::putText(frame, "Celulares detectados: " + std::to_string(cellCount),
                    cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

        cv::imshow("SICOV - Monitoramento de celulares", frame);

        if (static_cast<char>(cv::waitKey(1)) == 27) 
        {
            isRunning = false;
        }
    }

    Vision_Stop();
    Bsp_ReleaseCamera();
    cv::destroyAllWindows();
}

/** @} DOXYGEN GROUP TAG END OF FILE */