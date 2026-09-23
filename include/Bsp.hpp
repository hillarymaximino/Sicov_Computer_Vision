/******************************************************************************/
/**
 * @file    Bsp.hpp
 * @addtogroup BSP
 * @brief   Board Support Package - Driver de captura de vídeo e hardware.
 * @{
 ******************************************************************************/

#ifndef BSP_HPP
#define BSP_HPP

/*******************************************************************************
 * INCLUDES NECESSARIOS
 ******************************************************************************/
#include <opencv2/opencv.hpp>
#include <string>
#include <stdbool.h>

/*******************************************************************************
 * DEFINICOES E ESTRUTURAS
 ******************************************************************************/

/**
 * @brief Estrutura que agrupa o estado de telemetria e controle do BSP.
 */
typedef struct {
    volatile bool isCameraRunning; /**< Flag que indica se a thread de captura está ativa */
} Bsp_t;

extern Bsp_t bsp;

/*******************************************************************************
 * PROTOTIPOS DE FUNCOES
 ******************************************************************************/

/**
 * @brief Inicializa o stream de vídeo (RTSP/HTTP) e dispara a thread de captura.
 * @param streamUrl URL ou caminho RTSP do dispositivo de captura.
 * @return true se a conexão foi estabelecida com sucesso, false caso contrário.
 */
bool Bsp_InitCamera(const std::string& streamUrl);

/**
 * @brief Copia o frame mais recente adquirido pelo pipeline de captura.
 * @note  Função thread-safe protegida por mutex interno.
 * @param[out] outFrame Matriz do OpenCV onde a imagem atual será injetada.
 * @return true se a conexão permanecer ativa, false se o stream caiu.
 */
bool Bsp_GetFrame(cv::Mat& outFrame);

/**
 * @brief Finaliza com segurança a thread de captura e libera os descritores de hardware.
 */
void Bsp_ReleaseCamera(void);

#endif /* BSP_HPP */
/** @} */