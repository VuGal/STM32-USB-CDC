/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

/* Extern variables ------------------------------------------------------------------*/
extern uint8_t dataToSend[100];
extern uint8_t messageLength;
extern uint8_t receivedData[40];
extern uint8_t loopContinueFlag;

/* Function bodies ---------------------------------------------------------*/
void TestCircuitCommandHandler() {

	// If the message seems to turn the LED off (further string check is evaluated later)
	if (strstr(receivedData, "LED OFF")) {

		// If "RED LED OFF" message received, turns the test circuit red LED off
		if (!strcmp(receivedData, "RED LED OFF\0")) {
			HAL_GPIO_WritePin(LED_Red_Test_Circuit_GPIO_Port,
			LED_Red_Test_Circuit_Pin, GPIO_PIN_RESET);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend,
					"Red LED has been turned off!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}
		// If "BLUE LED OFF" message received, turns the test circuit blue LED off
		else if (!strcmp(receivedData, "BLUE LED OFF\0")) {
			HAL_GPIO_WritePin(LED_Blue_Test_Circuit_GPIO_Port,
			LED_Blue_Test_Circuit_Pin, GPIO_PIN_RESET);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend,
					"Blue LED has been turned off!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}
		// If "GREEN LED OFF" message received, turns the test circuit green LED off
		else if (!strcmp(receivedData, "GREEN LED OFF\0")) {
			HAL_GPIO_WritePin(LED_Green_Test_Circuit_GPIO_Port,
			LED_Green_Test_Circuit_Pin, GPIO_PIN_RESET);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend,
					"Green LED has been turned off!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}
		// If "YELLOW LED OFF" message received, turns the test circuit yellow LED off
		else if (!strcmp(receivedData, "YELLOW LED OFF\0")) {
			HAL_GPIO_WritePin(LED_Yellow_Test_Circuit_GPIO_Port,
			LED_Yellow_Test_Circuit_Pin, GPIO_PIN_RESET);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend,
					"Yellow LED has been turned off!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}

	}

	// If the message seems to turn the LED on (further string check is evaluated later)
	else if (strstr(receivedData, "LED ON")) {

		// If "RED LED ON" message received, turns the test circuit red LED on
		if (!strcmp(receivedData, "RED LED ON\0")) {
			HAL_GPIO_WritePin(LED_Red_Test_Circuit_GPIO_Port,
			LED_Red_Test_Circuit_Pin, GPIO_PIN_SET);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend,
					"Red LED has been turned on!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}

		// If "BLUE LED ON" message received, turns the test circuit blue LED on
		else if (!strcmp(receivedData, "BLUE LED ON\0")) {
			HAL_GPIO_WritePin(LED_Blue_Test_Circuit_GPIO_Port,
			LED_Blue_Test_Circuit_Pin, GPIO_PIN_SET);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend,
					"Blue LED has been turned on!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}

		// If "GREEN LED ON" message received, turns the test circuit green LED on
		else if (!strcmp(receivedData, "GREEN LED ON\0")) {
			HAL_GPIO_WritePin(LED_Green_Test_Circuit_GPIO_Port,
			LED_Green_Test_Circuit_Pin, GPIO_PIN_SET);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend,
					"Green LED has been turned on!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}

		// If "YELLOW LED ON" message received, turns the test circuit yellow LED on
		else if (!strcmp(receivedData, "YELLOW LED ON\0")) {
			HAL_GPIO_WritePin(LED_Yellow_Test_Circuit_GPIO_Port,
			LED_Yellow_Test_Circuit_Pin, GPIO_PIN_SET);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend,
					"Yellow LED has been turned on!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}

	}

	// If the message seems to turn the ALARM on/off (further string check is evaluated later)
	else if (strstr(receivedData, "ALARM")) {

		if (!strcmp(receivedData, "ALARM OFF\0")) {
			HAL_GPIO_WritePin(Buzzer_GPIO_Port,
			Buzzer_Pin, GPIO_PIN_RESET);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend, "Alarm has been turned off!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}

		else if (!strcmp(receivedData, "ALARM ON\0")) {
			HAL_GPIO_WritePin(Buzzer_GPIO_Port,
			Buzzer_Pin, GPIO_PIN_SET);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend, "Alarm has been turned on!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}

	}

	// Checks for the "GET COMMANDS" command and executes it, if it matches
	else if (!strcmp(receivedData, "GET COMMANDS\0")) {

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

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;
	}

	// If none of the previous checks passed, the command is incorrect
	else {

		// Displays a message to the user terminal
		messageLength = sprintf(dataToSend, "Incorrect command.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;
	}

}
