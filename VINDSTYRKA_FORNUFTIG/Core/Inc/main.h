/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "I2C_Sniffer.h"
#include "stdbool.h"
#include "Com.h"
#include "Events.h"


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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SW_POS_IN_1_Pin GPIO_PIN_3
#define SW_POS_IN_1_GPIO_Port GPIOA
#define SW_POS_IN_1_EXTI_IRQn EXTI3_IRQn
#define SW_POS_IN_2_Pin GPIO_PIN_4
#define SW_POS_IN_2_GPIO_Port GPIOA
#define SW_POS_IN_2_EXTI_IRQn EXTI4_IRQn
#define SW_POS_IN_3_Pin GPIO_PIN_5
#define SW_POS_IN_3_GPIO_Port GPIOA
#define SW_POS_IN_3_EXTI_IRQn EXTI9_5_IRQn
#define SW_POS_IN_4_Pin GPIO_PIN_6
#define SW_POS_IN_4_GPIO_Port GPIOA
#define SW_POS_IN_4_EXTI_IRQn EXTI9_5_IRQn
#define SCL_Pin_Pin GPIO_PIN_0
#define SCL_Pin_GPIO_Port GPIOB
#define SCL_Pin_EXTI_IRQn EXTI0_IRQn
#define SDA_Pin_Pin GPIO_PIN_1
#define SDA_Pin_GPIO_Port GPIOB
#define SDA_Pin_EXTI_IRQn EXTI1_IRQn
#define SW_POS_OUT_1_Pin GPIO_PIN_12
#define SW_POS_OUT_1_GPIO_Port GPIOB
#define SW_POS_OUT_2_Pin GPIO_PIN_13
#define SW_POS_OUT_2_GPIO_Port GPIOB
#define SW_POS_OUT_3_Pin GPIO_PIN_14
#define SW_POS_OUT_3_GPIO_Port GPIOB
#define SW_POS_OUT_4_Pin GPIO_PIN_15
#define SW_POS_OUT_4_GPIO_Port GPIOB
#define Mode_Led_Pin GPIO_PIN_6
#define Mode_Led_GPIO_Port GPIOB
#define Mode_Switch_1_Pin GPIO_PIN_7
#define Mode_Switch_1_GPIO_Port GPIOB
#define Mode_Switch_1_EXTI_IRQn EXTI9_5_IRQn
#define Mode_Switch_2_Pin GPIO_PIN_8
#define Mode_Switch_2_GPIO_Port GPIOB
#define Mode_Switch_2_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
