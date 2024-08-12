/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern void boot_UserDFU(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TOF_RST_Pin GPIO_PIN_10
#define TOF_RST_GPIO_Port GPIOC
#define PNL_3V3_EN_Pin GPIO_PIN_1
#define PNL_3V3_EN_GPIO_Port GPIOD
#define MIC_CK_Pin GPIO_PIN_3
#define MIC_CK_GPIO_Port GPIOB
#define OTG_FS_DP_Pin GPIO_PIN_12
#define OTG_FS_DP_GPIO_Port GPIOA
#define MIC_WS_Pin GPIO_PIN_15
#define MIC_WS_GPIO_Port GPIOA
#define PNL_1V8_EN_Pin GPIO_PIN_0
#define PNL_1V8_EN_GPIO_Port GPIOD
#define PS_ALS_SDA_Pin GPIO_PIN_7
#define PS_ALS_SDA_GPIO_Port GPIOB
#define OTG_FS_DM_Pin GPIO_PIN_11
#define OTG_FS_DM_GPIO_Port GPIOA
#define MIC_SD_Pin GPIO_PIN_5
#define MIC_SD_GPIO_Port GPIOB
#define IMU_RST_Pin GPIO_PIN_8
#define IMU_RST_GPIO_Port GPIOB
#define LT7911_CFG_SDA_Pin GPIO_PIN_9
#define LT7911_CFG_SDA_GPIO_Port GPIOC
#define ALS_INT_Pin GPIO_PIN_8
#define ALS_INT_GPIO_Port GPIOC
#define ALS_INT_EXTI_IRQn EXTI9_5_IRQn
#define LT7911_CFG_SCL_Pin GPIO_PIN_8
#define LT7911_CFG_SCL_GPIO_Port GPIOA
#define TEST1_Pin GPIO_PIN_6
#define TEST1_GPIO_Port GPIOD
#define PS_ALS_SCL_Pin GPIO_PIN_6
#define PS_ALS_SCL_GPIO_Port GPIOB
#define PNL_R_NSS_Pin GPIO_PIN_4
#define PNL_R_NSS_GPIO_Port GPIOE
#define TEST5_Pin GPIO_PIN_13
#define TEST5_GPIO_Port GPIOD
#define PNL_6V6_N_EN_Pin GPIO_PIN_2
#define PNL_6V6_N_EN_GPIO_Port GPIOD
#define TEST2_Pin GPIO_PIN_7
#define TEST2_GPIO_Port GPIOD
#define TEST6_Pin GPIO_PIN_14
#define TEST6_GPIO_Port GPIOD
#define TEST4_Pin GPIO_PIN_12
#define TEST4_GPIO_Port GPIOD
#define TEST3_Pin GPIO_PIN_11
#define TEST3_GPIO_Port GPIOD
#define PNL_L_XCLR_Pin GPIO_PIN_0
#define PNL_L_XCLR_GPIO_Port GPIOB
#define PS_INT_Pin GPIO_PIN_0
#define PS_INT_GPIO_Port GPIOC
#define PS_INT_EXTI_IRQn EXTI0_IRQn
#define PNL_R_XCLR_Pin GPIO_PIN_1
#define PNL_R_XCLR_GPIO_Port GPIOB
#define CAM_RST_Pin GPIO_PIN_11
#define CAM_RST_GPIO_Port GPIOB
#define SW_BRG_2D3D_Pin GPIO_PIN_3
#define SW_BRG_2D3D_GPIO_Port GPIOA
#define ALS_RST_Pin GPIO_PIN_2
#define ALS_RST_GPIO_Port GPIOA
#define PNL_L_NSS_Pin GPIO_PIN_12
#define PNL_L_NSS_GPIO_Port GPIOB
#define LT7911_INT_Pin GPIO_PIN_1
#define LT7911_INT_GPIO_Port GPIOA
#define LT7911_RSTN_Pin GPIO_PIN_2
#define LT7911_RSTN_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
void delay_us(uint32_t udelay);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
