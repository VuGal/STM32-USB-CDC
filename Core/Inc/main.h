/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

// Indicates the current active circuit
enum circuit {
	testCircuit, LCDCircuit, OLEDCircuit
} currentCircuit;

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
#define Button_Read_Write_Pin GPIO_PIN_4
#define Button_Read_Write_GPIO_Port GPIOE
#define Button_Read_Write_EXTI_IRQn EXTI4_IRQn
#define Button_Left_Pin GPIO_PIN_5
#define Button_Left_GPIO_Port GPIOE
#define Button_Left_EXTI_IRQn EXTI9_5_IRQn
#define LED_Green_LCD_Circuit_Pin GPIO_PIN_13
#define LED_Green_LCD_Circuit_GPIO_Port GPIOC
#define LED_Yellow_LCD_Circuit_Pin GPIO_PIN_14
#define LED_Yellow_LCD_Circuit_GPIO_Port GPIOC
#define LED_Red_LCD_Circuit_Pin GPIO_PIN_15
#define LED_Red_LCD_Circuit_GPIO_Port GPIOC
#define SCK_CLK_Pin GPIO_PIN_5
#define SCK_CLK_GPIO_Port GPIOA
#define MOSI_DIN_Pin GPIO_PIN_7
#define MOSI_DIN_GPIO_Port GPIOA
#define CS_Pin GPIO_PIN_5
#define CS_GPIO_Port GPIOC
#define DC_Pin GPIO_PIN_1
#define DC_GPIO_Port GPIOB
#define RES_Pin GPIO_PIN_7
#define RES_GPIO_Port GPIOE
#define LED_Red_Test_Circuit_Pin GPIO_PIN_6
#define LED_Red_Test_Circuit_GPIO_Port GPIOC
#define LED_Blue_Test_Circuit_Pin GPIO_PIN_7
#define LED_Blue_Test_Circuit_GPIO_Port GPIOC
#define LED_Green_Test_Circuit_Pin GPIO_PIN_8
#define LED_Green_Test_Circuit_GPIO_Port GPIOC
#define LED_Yellow_Test_Circuit_Pin GPIO_PIN_9
#define LED_Yellow_Test_Circuit_GPIO_Port GPIOC
#define Buzzer_Pin GPIO_PIN_13
#define Buzzer_GPIO_Port GPIOA
#define Button_Backspace_Pin GPIO_PIN_3
#define Button_Backspace_GPIO_Port GPIOB
#define Button_Backspace_EXTI_IRQn EXTI3_IRQn
#define D7_Pin GPIO_PIN_4
#define D7_GPIO_Port GPIOB
#define D6_Pin GPIO_PIN_5
#define D6_GPIO_Port GPIOB
#define D5_Pin GPIO_PIN_6
#define D5_GPIO_Port GPIOB
#define D4_Pin GPIO_PIN_7
#define D4_GPIO_Port GPIOB
#define E_Pin GPIO_PIN_8
#define E_GPIO_Port GPIOB
#define RS_Pin GPIO_PIN_9
#define RS_GPIO_Port GPIOB
#define Button_OK_Send_Pin GPIO_PIN_0
#define Button_OK_Send_GPIO_Port GPIOE
#define Button_OK_Send_EXTI_IRQn EXTI0_IRQn
#define Button_Right_Pin GPIO_PIN_1
#define Button_Right_GPIO_Port GPIOE
#define Button_Right_EXTI_IRQn EXTI1_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
