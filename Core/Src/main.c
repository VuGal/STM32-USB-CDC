/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "usbd_cdc_if.h"	// User interface file for USB controller
#include "SSD1331.h"		// Library which handles OLED display
#include "LiquidCrystal.h"	// Library which handles 2x16 LCD display
#include "TestCircuit.h"	// Library which handles Test Circuit related actions
#include "LCDCircuit.h"		// Library which handles LCD Circuit related actions
#include "OLEDCircuit.h"	// Library which handles OLED Circuit related actions
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;	// SPI is used by the OLED display

TIM_HandleTypeDef htim10;	// Timer is used by the Green LED in LCD Circuit

/* USER CODE BEGIN PV */

// extern variables are set to their default states during circuit switch [ SwitchCircuit() function ]
extern uint8_t startScreenDisplayedFlag;
extern uint8_t endScreenDisplayedFlag;
extern uint8_t receivedTextMessages[10][33];
extern uint8_t buttonTriggerFlag;

extern enum difficultyLevel {
	easy, medium, hard
} gameDifficultyLevel;

extern GameState snakeGameState;


uint8_t dataToSend[100];		// Array containing the sent data
uint8_t messageLength = 0;		// Sent message length

uint8_t receivedData[40];		// Array containing the received data
uint8_t receivedDataFlag = 0;	// Flag is set when MCU received a data from PC

uint8_t loopContinueFlag = 0;	// Flag forces the finish of current main loop iteration

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM10_Init(void);
/* USER CODE BEGIN PFP */
void ClearBuffers(void);
void SwitchCircuit(void);
void GetAllCommands(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Clears the sent data and received data and resets the loopContinueFlag flag
void ClearBuffers(void) {

	memset(receivedData, 0, sizeof receivedData);
	memset(dataToSend, 0, sizeof dataToSend);
	loopContinueFlag = 0;

}

// Switches the used circuit (available circuits: Test, LCD, OLED)
void SwitchCircuit(void) {

	if (!strcmp(receivedData, "SWITCH CIRCUIT TEST\0")) {

		// If the Test Circuit is already activated, informs the user about it via terminal message
		// and quits the SwitchCircuit() function
		if (currentCircuit == testCircuit) {
			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend, "Test Circuit is already active!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);
			return;
		}

		// Clears the LCD and OLED displays and turns off the LCD circuit' LEDs
		clear();
		noBlink();
		ssd1331_clear_screen(BLACK);
		HAL_GPIO_WritePin(LED_Red_LCD_Circuit_GPIO_Port,
				LED_Red_LCD_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Yellow_LCD_Circuit_GPIO_Port,
				LED_Yellow_LCD_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Green_LCD_Circuit_GPIO_Port,
				LED_Green_LCD_Circuit_Pin, GPIO_PIN_RESET);

		currentCircuit = testCircuit;

		// Displays a message to the user terminal
		messageLength = sprintf(dataToSend, "Switched to the Test Circuit!\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;

	} else if (!strcmp(receivedData, "SWITCH CIRCUIT LCD\0")) {

		// If the Test Circuit is already activated, informs the user about it via terminal message
		// and quits the SwitchCircuit() function
		if (currentCircuit == LCDCircuit) {
			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend, "LCD Circuit is already active!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);
			return;
		}

		// Clears the OLED display and turns off the test circuit' LEDs and buzzer
		ssd1331_clear_screen(BLACK);
		HAL_GPIO_WritePin(LED_Red_Test_Circuit_GPIO_Port,
		LED_Red_Test_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Blue_Test_Circuit_GPIO_Port,
		LED_Blue_Test_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Green_Test_Circuit_GPIO_Port,
		LED_Green_Test_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Yellow_Test_Circuit_GPIO_Port,
		LED_Yellow_Test_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin,
				GPIO_PIN_RESET);

		// Clears the received messages array
		memset(receivedTextMessages, 0,
				sizeof receivedTextMessages);

		currentCircuit = LCDCircuit;

		// Displays the welcome message
		setCursor(0, 0);
		print("Welcome to the");
		setCursor(0, 1);
		print("USB comm device!");

		// Displays a message to the user terminal
		messageLength = sprintf(dataToSend, "Switched to the LCD Circuit!\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;

	} else if (!strcmp(receivedData, "SWITCH CIRCUIT OLED\0")) {

		// If the Test Circuit is already activated, informs the user about it via terminal message
		// and quits the SwitchCircuit() function
		if (currentCircuit == OLEDCircuit) {
			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend, "OLED Circuit is already active!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);
			return;
		}

		// Clears the LCD display and turns off the LCD circuit LEDs along with the test circuit' LEDs and buzzer
		clear();
		noBlink();
		HAL_GPIO_WritePin(LED_Red_LCD_Circuit_GPIO_Port,
						LED_Red_LCD_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Yellow_LCD_Circuit_GPIO_Port,
						LED_Yellow_LCD_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Green_LCD_Circuit_GPIO_Port,
						LED_Green_LCD_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Red_Test_Circuit_GPIO_Port,
				LED_Red_Test_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Blue_Test_Circuit_GPIO_Port,
				LED_Blue_Test_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Green_Test_Circuit_GPIO_Port,
				LED_Green_Test_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_Yellow_Test_Circuit_GPIO_Port,
				LED_Yellow_Test_Circuit_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin,
						GPIO_PIN_RESET);

		// Resets the OLED circuit variables to their default states
		currentCircuit = OLEDCircuit;
		startScreenDisplayedFlag = 0;
		endScreenDisplayedFlag = 0;
		gameDifficultyLevel = easy;
		snakeGameState = startGameScreen;

		// Clears the OLED display
		ssd1331_clear_screen(BLACK);

		// Displays a message to the user terminal
		messageLength = sprintf(dataToSend, "Switched to the OLED Circuit!\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;

	}

	else {

		// Displays a message to the user terminal
		messageLength = sprintf(dataToSend, "Incorrect command.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;
	}

}

void GetAllCommands(void) {

	if (!strcmp(receivedData, "GET ALL COMMANDS\0")) {

		// Displays a message to the user terminal. The message contains all available commands along with their descriptions. Delays are added to not transfer a few messages in one data buffer.
		messageLength = sprintf(dataToSend, "Commands available in the Test Circuit:\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "RED LED OFF / RED LED ON - switch the red LED off/on.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "BLUE LED OFF / BLUE LED ON - switch the blue LED off/on.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "GREEN LED OFF / GREEN LED ON - switch the green LED off/on.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "YELLOW LED OFF / YELLOW LED ON - switch the yellow LED off/on.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "ALARM OFF / ALARM ON - turn the buzzer off/on.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "Commands available in the LCD Circuit:\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "SEND '[message]' - sends [message] message to LCD circuit.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "The message must be up to 32 characters long and needs to be put between single quotation marks.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "Any message that doesn't follow strict convention (SEND '[up_to_32_char_long_message]') will be discarded.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "Commands available in the OLED Circuit:\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "START - starts the game (works only if currently in start game screen or end game screen.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "w - changes the snake move direction to up (work only if currently in game)\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "a - changes the snake move direction to left (work only if currently in game)\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "s - changes the snake move direction to down (work only if currently in game)\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);
		messageLength = sprintf(dataToSend, "d - changes the snake move direction to right (work only if currently in game)\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);
		HAL_Delay(1);

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;
	}

	else {
		// Displays a message to the user terminal.
		messageLength = sprintf(dataToSend, "Incorrect command.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;
	}

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
	SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	srand(time(NULL));
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USB_DEVICE_Init();
	MX_SPI1_Init();
	MX_TIM10_Init();
  /* USER CODE BEGIN 2 */

	// Sets up Timer in one-pulse mode
	HAL_TIM_Base_Start_IT(&htim10);
	HAL_TIM_OnePulse_Start_IT(&htim10, TIM_CHANNEL_1);

	// Initializes the OLED display
	ssd1331_init();
	ssd1331_clear_screen(BLACK);

	// Initializes the LCD display
	LiquidCrystal(GPIOB, GPIO_PIN_9, GPIO_PIN_12, GPIO_PIN_8, GPIO_PIN_7,
					GPIO_PIN_6, GPIO_PIN_5, GPIO_PIN_4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		// Checks if a button was pressed (does nothing if the circuit is set to Test or OLED)
		if (buttonTriggerFlag) {
			ButtonTrigger();
		}

		// Skips further main loop execution in this iteration to save time
		if (loopContinueFlag) { ClearBuffers(); continue; }



		// Performs an operation on the OLED display, if it's active
		if (currentCircuit == OLEDCircuit) {
			OLEDCircuitAction();
		}

		// Skips further main loop execution in this iteration to save time
		if (loopContinueFlag) { ClearBuffers(); continue; }



		// If some data was received from the PC
		if (receivedDataFlag == 1) {

			// Resets the flag
			receivedDataFlag = 0;

			// If the message seems to switch the circuit (further string check is evaluated later)
			if (strstr(receivedData, "SWITCH CIRCUIT")) {

				// Switches the circuit
				SwitchCircuit();

				// Skips further main loop execution in this iteration to save time
				if (loopContinueFlag) { ClearBuffers(); continue; }

			}

			else if (currentCircuit == testCircuit) {

				// Handles the Test Circuit command (or rejects it if it's incorrect)
				TestCircuitCommandHandler();

				// Skips further main loop execution in this iteration to save time
				if (loopContinueFlag) { ClearBuffers(); continue; }

			}

			else if (currentCircuit == LCDCircuit) {

				// Handles the LCD Circuit command (or rejects it if it's incorrect)
				LCDCircuitCommandHandler();

				// Skips further main loop execution in this iteration to save time
				if (loopContinueFlag) { ClearBuffers(); continue; }

			}

			else if (currentCircuit == OLEDCircuit) {

				// Handles the OLED Circuit command (or rejects it if it's incorrect)
				OLEDCircuitCommandHandler();

				// Skips further main loop execution in this iteration to save time
				if (loopContinueFlag) { ClearBuffers(); continue; }

			}


			// Checks for the "GET ALL COMMANDS" command and executes it, if it matches
			GetAllCommands();

			// Skips further main loop execution in this iteration to save time
			if (loopContinueFlag) { ClearBuffers(); continue; }

		}

	}
}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 9999;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 14399;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim10, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_ACTIVE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_Green_LCD_Circuit_Pin|LED_Yellow_LCD_Circuit_Pin|LED_Red_LCD_Circuit_Pin|CS_Pin
                          |LED_Red_Test_Circuit_Pin|LED_Blue_Test_Circuit_Pin|LED_Green_Test_Circuit_Pin|LED_Yellow_Test_Circuit_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DC_Pin|D7_Pin|D6_Pin|D5_Pin
                          |D4_Pin|E_Pin|RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RES_GPIO_Port, RES_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Button_Read_Write_Pin Button_Left_Pin Button_OK_Send_Pin Button_Right_Pin */
  GPIO_InitStruct.Pin = Button_Read_Write_Pin|Button_Left_Pin|Button_OK_Send_Pin|Button_Right_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_Green_LCD_Circuit_Pin LED_Yellow_LCD_Circuit_Pin LED_Red_LCD_Circuit_Pin CS_Pin
                           LED_Red_Test_Circuit_Pin LED_Blue_Test_Circuit_Pin LED_Green_Test_Circuit_Pin LED_Yellow_Test_Circuit_Pin */
  GPIO_InitStruct.Pin = LED_Green_LCD_Circuit_Pin|LED_Yellow_LCD_Circuit_Pin|LED_Red_LCD_Circuit_Pin|CS_Pin
                          |LED_Red_Test_Circuit_Pin|LED_Blue_Test_Circuit_Pin|LED_Green_Test_Circuit_Pin|LED_Yellow_Test_Circuit_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : DC_Pin D7_Pin D6_Pin D5_Pin
                           D4_Pin E_Pin RS_Pin */
  GPIO_InitStruct.Pin = DC_Pin|D7_Pin|D6_Pin|D5_Pin
                          |D4_Pin|E_Pin|RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RES_Pin */
  GPIO_InitStruct.Pin = RES_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RES_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Buzzer_Pin */
  GPIO_InitStruct.Pin = Buzzer_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Buzzer_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Button_Backspace_Pin */
  GPIO_InitStruct.Pin = Button_Backspace_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Button_Backspace_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
