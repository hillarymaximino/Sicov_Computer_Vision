/******************************************************************************/
/**
 * @file    Bsp.cpp
 * @addtogroup BSP
 * @brief   Camada de abstracao de Captura e Hardware (Board Support Package).
 * @author  Guilherme Gonçalves
 * @details
 * \n <b>Ferramentas:</b>
 * - C++17, OpenCV.
 *
 * \n <b>Dependencias:</b>
 * - OpenCV (videoio, imgproc).
 * - STL (thread, mutex, atomic).
 *
 * \n <b>Observacoes:</b>
 * - Este arquivo encapsula a captura de video (RTSP/HTTP/Webcam).
 * - Uma thread de background e instanciada para evitar bloqueios no laco principal.
 * - O acesso ao frame compartilhado e protegido por Mutex.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Bsp.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/

/// Variaveis internas da BSP agrupadas para manter o padrao de isolamento
/// Estrutura nomeada para evitar erro de construtor deletado (deleted function)
struct BspContext
{
    cv::VideoCapture camera;
    cv::Mat latestFrame;
    std::mutex frameMutex;
    std::atomic<bool> isCameraRunning;
    std::thread captureThread;

   
    BspContext() : isCameraRunning(false) {}
};

/// Instância global das variáveis internas
static BspContext bspCtx;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

static void CaptureLoop(void);

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief  Inicializacao da BSP e conexao com a camera.
 *  @param  streamUrl: String contendo o link RTSP/HTTP da camera ou ID do USB.
 *  @retval true se conectou e iniciou a thread, false caso contrario.
 *  @details Abre a porta de video e, em caso de sucesso, dispara a funcao
 *           CaptureLoop() em uma thread de background separada.
 ******************************************************************************/
bool Bsp_InitCamera(const std::string& streamUrl)
{//Pega o link RTSP e faz o que o openCV tente estabelecer uma coneção
    bspCtx.camera.open(streamUrl);
    
    if (!bspCtx.camera.isOpened()) 
    {
        std::cerr << "[BSP] Falha ao conectar: " << streamUrl << std::endl;
        return false;
    }
    
    bspCtx.isCameraRunning.store(true);
    bspCtx.captureThread = std::thread(CaptureLoop);
    
    std::cout << "[BSP] Camera conectada (Modo Multi-Thread Ativado)." << std::endl;
    return true;
}

/******************************************************************************/
/** @brief  Copia a imagem mais recente recebida da camera.
 *  @param  outFrame: Referencia da matriz onde a foto sera injetada.
 *  @retval true se a camera ainda esta online, false se a conexao caiu.
 *  @details O acesso a imagem global e protegido por lock_guard para evitar
 *           que a IA leia a imagem pela metade enquanto ela e atualizada.
 ******************************************************************************/
bool Bsp_GetFrame(cv::Mat& outFrame)
{
    if (!bspCtx.isCameraRunning.load())
    {
        return false; // Aborta se a conexao caiu
    }
    
    std::lock_guard<std::mutex> lock(bspCtx.frameMutex);
    if (!bspCtx.latestFrame.empty()) 
    {
        bspCtx.latestFrame.copyTo(outFrame);
    }
    
    return true;
}

/******************************************************************************/
/** @brief  Encerra a conexao e limpa os recursos da BSP.
 *  @details Sinaliza para a thread parar, aguarda sua finalizacao com join()
 *           e libera o objeto de captura do OpenCV da memoria.
 ******************************************************************************/
void Bsp_ReleaseCamera(void)
{
    bspCtx.isCameraRunning.store(false);
    
    if (bspCtx.captureThread.joinable()) 
    {
        bspCtx.captureThread.join(); 
    }
    
    if (bspCtx.camera.isOpened()) 
    {
        bspCtx.camera.release();
    }
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/******************************************************************************/
/** @brief  Loop infinito executado pela thread em background (Worker 1).
 *  @details Fica preso lendo o buffer da camera sem parar. Quando uma foto
 *           nova chega, adquire o Mutex e substitui a foto velha pela nova.
 *           Isso garante que a IA sempre consuma o frame mais atual.
 ******************************************************************************/
static void CaptureLoop(void)
{
    cv::Mat tempFrame;
    
    while (bspCtx.isCameraRunning.load()) 
    {
        if (bspCtx.camera.read(tempFrame)) 
        {
            std::lock_guard<std::mutex> lock(bspCtx.frameMutex);
            tempFrame.copyTo(bspCtx.latestFrame);
        } 
        else 
        {
            // Se nao conseguir ler, a conexao foi perdida (ex: Wi-Fi caiu)
            bspCtx.isCameraRunning.store(false); 
        }
    }
}

/** @} DOXYGEN GROUP TAG END OF FILE */