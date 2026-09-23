/******************************************************************************/
/**
 * @file    Orchestrator.hpp
 * @addtogroup ORCHESTRATOR
 * @brief   Interface do modulo principal de orquestracao e interface grafica.
 * @author  Gonçalves
 * @{
 ******************************************************************************/

#ifndef ORCHESTRATOR_HPP
#define ORCHESTRATOR_HPP

/*******************************************************************************
 * INCLUDES NECESSARIOS
 ******************************************************************************/
#include <string>

/*******************************************************************************
 * PROTOTIPOS DE FUNCOES
 ******************************************************************************/

/**
 * @brief Inicializa os modulos do sistema na ordem correta (Camera e IA).
 * @param streamUrl Caminho ou link RTSP/HTTP da camera de monitoramento.
 * @param modelPath Caminho do arquivo de modelo ONNX (YOLO).
 * @return true se a camera e a IA iniciarem corretamente, false em caso de erro.
 */
bool Orchestrator_Init(const std::string& streamUrl, const std::string& modelPath);

/**
 * @brief Inicia o loop infinito principal do sistema (main loop).
 * @details Mantem o programa vivo, consumindo frames de captura, solicitando 
 * processamento da IA e renderizando as caixas delimitadoras (bounding 
 * boxes) na interface grafica do OpenCV.
 */
void Orchestrator_Run(void);

#endif /* ORCHESTRATOR_HPP */
/** @} DOXYGEN GROUP TAG END OF FILE */