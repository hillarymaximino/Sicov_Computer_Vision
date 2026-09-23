/******************************************************************************/
/**
 * @file    main.cpp
 * @addtogroup MAIN
 * @brief   Ponto de entrada do sistema SICOV.
 * @author  Gonçalves
 * @details
 * \n <b>Ferramentas:</b>
 * - C++17, OpenCV.
 *
 * \n <b>Dependencias:</b>
 * - Orchestrator.
 *
 * \n <b>Observacoes:</b>
 * - Inicializa o motor de visao, configura parametros globais de processamento
 * (como o numero de threads do OpenCV) e inicia a orquestracao principal.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Orchestrator.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>

/*******************************************************************************
 * FUNCAO PRINCIPAL
 ******************************************************************************/

/******************************************************************************/
/** @brief  Ponto de entrada da aplicacao.
 *  @retval 0 em caso de sucesso, -1 em caso de falha critica na inicializacao.
 *  @details Configura as threads do OpenCV, define os caminhos dos recursos
 * (camera e modelo) e entrega o controle para o Orchestrator.
 ******************************************************************************/
int main(void) 
{
    // Aqui eu libero 8 nucleos do processador para o motor de visao processar em paralelo.
    // Isso e util para acelerar a inferencia
    cv::setNumThreads(8);

    const std::string cameraUrl = "rtsp://admin:admin123@192.168.11.15/cam/realmonitor?channel=1&subtype=0";
    const std::string modelPath = "models/yolo11n.onnx";

    if (!Orchestrator_Init(cameraUrl, modelPath)) 
    {
        std::cerr << "Falha critica na inicializacao." << std::endl;
        return -1; 
    }

    Orchestrator_Run();

    std::cout << "Sistema encerrado com sucesso." << std::endl;
    return 0;
}

/** @} DOXYGEN GROUP TAG END OF FILE */