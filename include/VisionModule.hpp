/******************************************************************************/
/**
 * @file    VisionModule.hpp
 * @addtogroup VISION
 * @brief   Interface do modulo de Visao Computacional e IA (YOLOv8).
 * @author  Gonçalves
 * @{
 ******************************************************************************/

#ifndef VISION_MODULE_HPP
#define VISION_MODULE_HPP

/*******************************************************************************
 * INCLUDES NECESSARIOS
 ******************************************************************************/
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

/*******************************************************************************
 * PROTOTIPOS DE FUNCOES
 ******************************************************************************/

/**
 * @brief Inicializa a rede neural carregando o modelo ONNX e executando warm-up.
 * @param modelPath Caminho do arquivo .onnx do YOLOv8.
 * @return true se o modelo carregou e compilou os grafos, false caso contrario.
 */
bool Vision_Init(const std::string& modelPath);

/**
 * @brief Dispara a thread dedicada da IA para processamento em background.
 */
void Vision_Start(void);

/**
 * @brief Encerra a thread da IA de forma segura, aguardando a conclusao do ciclo.
 */
void Vision_Stop(void);

/**
 * @brief Envia um frame novo para a fila de inferencia (nao-bloqueante).
 * @param frame Imagem capturada para analise.
 */
void Vision_SubmitFrame(const cv::Mat& frame);

/**
 * @brief Recupera o resultado mais recente das deteccoes e a contagem.
 * @param[out] boxes Vetor preenchido com as coordenadas dos objetos.
 * @param[out] vehicleCount Total de objetos validados dentro da ROI.
 */
void Vision_GetOverlay(std::vector<cv::Rect>& boxes, int& vehicleCount);

#endif /* VISION_MODULE_HPP */
/** @} DOXYGEN GROUP TAG END OF FILE */