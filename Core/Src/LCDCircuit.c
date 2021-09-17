/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "stm32f4xx_hal.h"
#include "LiquidCrystal.h"
#include "main.h"
#include "LCDCircuit.h"

/* Extern variables ------------------------------------------------------------------*/
extern uint8_t dataToSend[100];
extern uint8_t receivedData[40];
extern uint8_t loopContinueFlag;
extern uint8_t messageLength;
extern TIM_HandleTypeDef htim10;

/* Variables ------------------------------------------------------------------*/

// Array storing unread received messages (receivedData) [32 characters + '\0')
uint8_t receivedTextMessages[10][33];

// Used for picking the oldest received message (using receivedTextMessages array)
uint8_t receivedTextMessagesLeft = 0;

// Flag is set when any button in LCD circuit has been pressed
uint8_t buttonTriggerFlag = 0;

// Used to determine the position to generate a character and blink the cursor
uint8_t currentCursorPositionRow = 0;
uint8_t currentCursorPositionColumn = 0;

// Determines the index of a character in its character group array (lower case letters, upper case letters, numbers)
uint8_t currentSignGroupIndex = 0;

// Flag is set when the received message is incorrect for some reason
uint8_t wrongMessageFlag = 0;

// Determines the current time in ms
uint32_t currentTime = 0;


// Messages shown in two LCD rows
char firstLCDLineMessage[16];
char secondLCDLineMessage[16];

// Sign to print on the current cursor on the LCD display. 0th element changes accordingly to the
// chosen character, while 1st element is always '\0'
char signToPrint[2] = { ' ', '\0' };

// Message picked from the receivedData array along with the pointers to its start and end elements
char message[32];
char *messageBegin;
char *messageEnd;

// Characters available to pick. Groups are switched by pressing the buttons
// (buttonLeft to switch between small and capital letters, buttonRight to toggle number insertion)
const char lowerCaseLetters[27] = { ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
		'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'w',
		'x', 'y', 'z' };
const char upperCaseLetters[27] = { ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'W',
		'X', 'Y', 'Z' };
const char numbers[11] =
		{ ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

// Determines the communication mode - reading received messages or writing messages to PC
enum LCDCircuitCommDirection {
	receive, transmit
} commDirection;

// Determines the current character group
enum signGroup {
	smallLetters, capitalLetters, nums
} currentSignGroup;


// Buttons available on the board
Button buttonReadWrite = { none, 0, 0 };
Button buttonBackspace = { none, 0, 0 };
Button buttonLeft = { none, 0, 0 };
Button buttonRight = { none, 0, 0 };
Button buttonOkSend = { none, 0, 0 };


/* Function bodies ---------------------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	// Interrupt turns off the green LED after 2 seconds
	if (htim->Instance == TIM10) {
		HAL_GPIO_WritePin(LED_Green_LCD_Circuit_GPIO_Port, LED_Green_LCD_Circuit_Pin, GPIO_PIN_RESET);
	}

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	// buttonTriggerFlag must be 0 here to only run interrupt when the whole button handler has been completed
	if (buttonTriggerFlag == 0 && currentCircuit == LCDCircuit) {

		if (GPIO_Pin == Button_Read_Write_Pin) {

			// Button handle was completed, so the current state is equal to "none"
			// This should run only if a button was already released earlier or is pressed for the first time.
			if (buttonReadWrite.currentState == none) {

				currentTime = HAL_GetTick();

				// Checks if sufficient amount of time has passed from the last button release (for debouncing purposes).
				// If yes, marks button as pressed and saves the press time, otherwise the interrupt is ignored.
				if (currentTime - buttonReadWrite.releaseTime > 20) {
					buttonReadWrite.pressTime = currentTime;
					buttonReadWrite.currentState = pressed;
				}
			}

			// If the button was pressed and not released yet
			else if (buttonReadWrite.currentState == pressed) {

				currentTime = HAL_GetTick();

				// 20 ms must pass from press to release to acknowledge the correct button press and save the release time.
				// Otherwise the interrupt is ignored, as it's treated as a button bouncing glitch.
				if (currentTime - buttonReadWrite.pressTime > 20) {
					buttonReadWrite.releaseTime = currentTime;
					buttonTriggerFlag = 1;
					buttonReadWrite.currentState = released;
				}

			}

		}

		else if (GPIO_Pin == Button_Backspace_Pin) {

			// Button handle was completed, so the current state is equal to "none"
			// This should run only if a button was already released earlier or is pressed for the first time.
			if (buttonBackspace.currentState == none) {

				currentTime = HAL_GetTick();

				// Checks if sufficient amount of time has passed from the last button release (for debouncing purposes).
				// If yes, marks button as pressed and saves the press time, otherwise the interrupt is ignored.
				if (currentTime - buttonBackspace.releaseTime > 20) {
					buttonBackspace.pressTime = currentTime;
					buttonBackspace.currentState = pressed;
				}
			}

			// If the button was pressed and not released yet
			else if (buttonBackspace.currentState == pressed) {

				currentTime = HAL_GetTick();

				// 20 ms must pass from press to release to acknowledge the correct button press and save the release time.
				// Otherwise the interrupt is ignored, as it's treated as a button bouncing glitch.
				if (currentTime - buttonBackspace.pressTime > 20) {
					buttonBackspace.releaseTime = currentTime;
					buttonTriggerFlag = 1;
					buttonBackspace.currentState = released;
				}

			}

		}

		else if (GPIO_Pin == Button_Left_Pin) {

			// Button handle was completed, so the current state is equal to "none"
			// This should run only if a button was already released earlier or is pressed for the first time.
			if (buttonLeft.currentState == none) {

				currentTime = HAL_GetTick();

				// Checks if sufficient amount of time has passed from the last button release (for debouncing purposes).
				// If yes, marks button as pressed and saves the press time, otherwise the interrupt is ignored.
				if (currentTime - buttonLeft.releaseTime > 20) {
					buttonLeft.pressTime = currentTime;
					buttonLeft.currentState = pressed;
				}
			}

			// If the button was pressed and not released yet
			else if (buttonLeft.currentState == pressed) {

				currentTime = HAL_GetTick();

				// 20 ms must pass from press to release to acknowledge the correct button press and save the release time.
				// Otherwise the interrupt is ignored, as it's treated as a button bouncing glitch.
				if (currentTime - buttonLeft.pressTime > 20) {
					buttonLeft.releaseTime = currentTime;
					buttonTriggerFlag = 1;
					buttonLeft.currentState = released;
				}

			}

		}

		else if (GPIO_Pin == Button_Right_Pin) {

			// Button handle was completed, so the current state is equal to "none"
			// This should run only if a button was already released earlier or is pressed for the first time.
			if (buttonRight.currentState == none) {

				currentTime = HAL_GetTick();

				// Checks if sufficient amount of time has passed from the last button release (for debouncing purposes).
				// If yes, marks button as pressed and saves the press time, otherwise the interrupt is ignored.
				if (currentTime - buttonRight.releaseTime > 20) {
					buttonRight.pressTime = currentTime;
					buttonRight.currentState = pressed;
				}
			}

			// If the button was pressed and not released yet
			else if (buttonRight.currentState == pressed) {

				currentTime = HAL_GetTick();

				// 20 ms must pass from press to release to acknowledge the correct button press and save the release time.
				// Otherwise the interrupt is ignored, as it's treated as a button bouncing glitch.
				if (currentTime - buttonRight.pressTime > 20) {
					buttonRight.releaseTime = currentTime;
					buttonTriggerFlag = 1;
					buttonRight.currentState = released;
				}

			}

		}

		else if (GPIO_Pin == Button_OK_Send_Pin) {

			// Button handle was completed, so the current state is equal to "none"
			// This should run only if a button was already released earlier or is pressed for the first time.
			if (buttonOkSend.currentState == none) {

				currentTime = HAL_GetTick();

				// Checks if sufficient amount of time has passed from the last button release (for debouncing purposes).
				// If yes, marks button as pressed and saves the press time, otherwise the interrupt is ignored.
				if (currentTime - buttonOkSend.releaseTime > 20) {
					buttonOkSend.pressTime = currentTime;
					buttonOkSend.currentState = pressed;
				}
			}

			// If the button was pressed and not released yet
			else if (buttonOkSend.currentState == pressed) {

				currentTime = HAL_GetTick();

				// 20 ms must pass from press to release to acknowledge the correct button press and save the release time.
				// Otherwise the interrupt is ignored, as it's treated as a button bouncing glitch.
				if (currentTime - buttonOkSend.pressTime > 20) {
					buttonOkSend.releaseTime = currentTime;
					buttonTriggerFlag = 1;
					buttonOkSend.currentState = released;
				}

			}

		}

	}

}


void ButtonTrigger() {

	if (buttonReadWrite.currentState == released) {

		// Resets other buttons
		buttonBackspace.currentState = none;
		buttonLeft.currentState = none;
		buttonRight.currentState = none;
		buttonOkSend.currentState = none;

		// Short press (under 2 seconds) - reads the oldest unread message
		if (buttonReadWrite.releaseTime - buttonReadWrite.pressTime
				< 2000) {

			// Yellow LED is turned off while in receive mode
			commDirection = receive;
			HAL_GPIO_WritePin(LED_Yellow_LCD_Circuit_GPIO_Port,
			LED_Yellow_LCD_Circuit_Pin, GPIO_PIN_RESET);

			// Informs the user that all received messages has been read
			if (receivedTextMessagesLeft == 0) {
				clear();
				noBlink();
				print("No more received");
				setCursor(0, 1);
				print("messages!");
			}

			// Displays the oldest unread message
			else {

				// Decrements the unread messages counter
				--receivedTextMessagesLeft;

				// Turns the red LED off if no unread messages left
				if (receivedTextMessagesLeft == 0) {
					HAL_GPIO_WritePin(LED_Red_LCD_Circuit_GPIO_Port,
					LED_Red_LCD_Circuit_Pin, GPIO_PIN_RESET);
				}

				int ch = 0;

				// Assembles the first text line to display
				while (ch < 16) {

					firstLCDLineMessage[ch] =
							receivedTextMessages[0][ch]; 	// 0th index - the oldest received message
					++ch;

				}

				// Assembles the second text line to display
				while (ch < 33) {
					secondLCDLineMessage[ch - 16] =
							receivedTextMessages[0][ch];	// 0th index - the oldest received message
					++ch;
				}

				// Displays the message
				clear();
				noBlink();
				print(firstLCDLineMessage);
				setCursor(0, 1);
				print(secondLCDLineMessage);

				// Drops the oldest message and moves messages in the unread messages queue
				for (int i = 0; i < receivedTextMessagesLeft; ++i) {
					strcpy(receivedTextMessages[i],
							receivedTextMessages[i + 1]);
				}

				// Clears the memory on the queue top after moving the newest unread message down the queue
				memset(receivedTextMessages[receivedTextMessagesLeft],
						0, 32);
			}

		}

		// Long press (above 2 seconds) - switches into a transmit mode
		else {

			// If currently in receive mode, switches to transmit mode; otherwise does nothing
			if (commDirection != transmit) {

				commDirection = transmit;
				currentCursorPositionRow = 0;
				currentCursorPositionColumn = 0;
				currentSignGroup = smallLetters;	// Small letters are the default character group
				currentSignGroupIndex = 0;

				HAL_GPIO_WritePin(LED_Yellow_LCD_Circuit_GPIO_Port,
				LED_Yellow_LCD_Circuit_Pin, GPIO_PIN_SET);

				// Clears the screen and enables cursor blinking
				clear();
				blink();

			}

		}

		// Button handle is complete - restore the button its default state
		buttonReadWrite.currentState = none;

	}

	else if (buttonBackspace.currentState == released) {

		// Resets other buttons
		buttonReadWrite.currentState = none;
		buttonLeft.currentState = none;
		buttonRight.currentState = none;
		buttonOkSend.currentState = none;

		// Works only in transmit mode, does nothing in receive mode
		if (commDirection == transmit) {

			// Short press (under 2 seconds) - deletes one character (Backspace mode)
			if ((buttonBackspace.releaseTime - buttonBackspace.pressTime
					< 2000)
					&& (!(currentCursorPositionRow == 0
							&& currentCursorPositionColumn == 0))) {

				// If some character is currently displayed, but not yet approved, deletes it,
				// but doesn't change the cursor position
				if (currentSignGroupIndex != 0) {

					currentSignGroupIndex = 0;
					setCursor(currentCursorPositionColumn,
							currentCursorPositionRow);
					print(" ");
					setCursor(currentCursorPositionColumn,
							currentCursorPositionRow);

				// Otherwise goes back one position and deletes the character that was displayed there earlier
				} else {

					currentSignGroupIndex = 0;

					if (!(currentCursorPositionRow == 1
							&& currentCursorPositionColumn == 0)) {
						--currentCursorPositionColumn;
					} else {
						currentCursorPositionRow = 0;
						currentCursorPositionColumn = 15;
					}
					setCursor(currentCursorPositionColumn,
							currentCursorPositionRow);
					print(" ");
					setCursor(currentCursorPositionColumn,
							currentCursorPositionRow);
				}

			}

			// Long press (above 2 seconds) - deletes all characters and returns to the first position (0th row, 0th column)
			else {
				clear();
				currentCursorPositionRow = 0;
				currentCursorPositionColumn = 0;
				currentSignGroupIndex = 0;
			}

		}

		// Button handle is complete - restore the button its default state
		buttonBackspace.currentState = none;

	}

	else if (buttonLeft.currentState == released) {

		// Resets other buttons
		buttonReadWrite.currentState = none;
		buttonBackspace.currentState = none;
		buttonRight.currentState = none;
		buttonOkSend.currentState = none;

		// Works only in transmit mode, does nothing in receive mode
		if (commDirection == transmit) {

			// Flag is reset when signToPrint[0] == ' ' to avoid useless print
			uint8_t printFlag = 1;

			// Short press (under 2 seconds) - changes the current character to the previous one in its group
			if (buttonLeft.releaseTime - buttonLeft.pressTime < 2000) {

				if (currentSignGroupIndex != 0) {

					--currentSignGroupIndex;

					if (currentSignGroup == smallLetters) {
						signToPrint[0] =
								lowerCaseLetters[currentSignGroupIndex];
					} else if (currentSignGroup == capitalLetters) {
						signToPrint[0] =
								upperCaseLetters[currentSignGroupIndex];
					} else if (currentSignGroup == nums) {
						signToPrint[0] = numbers[currentSignGroupIndex];
					}

				}

				// signToPrint[0] == ' '
				else {
					printFlag = 0;
				}

			}

			// Long press (above 2 seconds) - switches from small letters to capital letters and vice versa (Caps Lock mode)
			// Does nothing if numbers are a current character group
			else {
				if (currentSignGroup == smallLetters) {
					currentSignGroup = capitalLetters;
					signToPrint[0] =
							upperCaseLetters[currentSignGroupIndex];
				} else if (currentSignGroup == capitalLetters) {
					currentSignGroup = smallLetters;
					signToPrint[0] =
							lowerCaseLetters[currentSignGroupIndex];
				}
			}


			if (printFlag) {

				// Stops blinking for a short while for a smoother character change display
				noBlink();

				// Prints the new character and returns the cursor back, because its position increments in print() function
				print(signToPrint);
				HAL_Delay(10);
				setCursor(currentCursorPositionColumn,
						currentCursorPositionRow);

				// Starts blinking again
				blink();

			}

		}

		// Button handle is complete - restore the button its default state
		buttonLeft.currentState = none;

	}

	else if (buttonRight.currentState == released) {

		// Resets other buttons
		buttonReadWrite.currentState = none;
		buttonBackspace.currentState = none;
		buttonLeft.currentState = none;
		buttonOkSend.currentState = none;

		// Works only in transmit mode, does nothing in receive mode
		if (commDirection == transmit) {

			// Short press (under 2 seconds) - changes the current character to the next one in its group
			if (buttonRight.releaseTime - buttonRight.pressTime
					< 2000) {

				if (currentSignGroup != nums) {

					if (currentSignGroupIndex != 25) {

						++currentSignGroupIndex;

						if (currentSignGroup == smallLetters) {

							signToPrint[0] =
									lowerCaseLetters[currentSignGroupIndex];

						} else {

							signToPrint[0] =
									upperCaseLetters[currentSignGroupIndex];

						}
					}

				}

				// currentSignGroup == nums
				else {

					if (currentSignGroupIndex != 10) {

						++currentSignGroupIndex;
						signToPrint[0] = numbers[currentSignGroupIndex];

					}

				}

			}

			// Long press (above 2 seconds) - toggles between letters and numbers insertion.
			// When switching from numbers to letters, it chooses the small letters group.
			else {

				if (currentSignGroup != nums) {
					currentSignGroup = nums;
					currentSignGroupIndex = 0;
					signToPrint[0] = numbers[0];
				} else {
					currentSignGroup = smallLetters;
					currentSignGroupIndex = 0;
					signToPrint[0] = lowerCaseLetters[0];
				}

			}

			// Stops blinking for a short while for a smoother character change display
			noBlink();

			// Prints the new character and returns the cursor back, because its position increments in print() function
			print(signToPrint);
			HAL_Delay(10);
			setCursor(currentCursorPositionColumn,
					currentCursorPositionRow);

			// Starts blinking again
			blink();

		}

		// Button handle is complete - restore the button its default state
		buttonRight.currentState = none;

	}

	else if (buttonOkSend.currentState == released) {

		// Resets other buttons
		buttonReadWrite.currentState = none;
		buttonBackspace.currentState = none;
		buttonLeft.currentState = none;
		buttonRight.currentState = none;

		// Works only in transmit mode, does nothing in receive mode
		if (commDirection == transmit) {

			// Short press (under 2 seconds) - approves the current character and increments the cursor position
			if (buttonOkSend.releaseTime - buttonOkSend.pressTime
					< 2000) {

				if (currentSignGroup == smallLetters) {
					message[16 * currentCursorPositionRow
							+ currentCursorPositionColumn] =
							lowerCaseLetters[currentSignGroupIndex];
				}

				else if (currentSignGroup == capitalLetters) {
					message[16 * currentCursorPositionRow
							+ currentCursorPositionColumn] =
							upperCaseLetters[currentSignGroupIndex];
				}

				// currentSignGroup == nums
				else {
					message[16 * currentCursorPositionRow
							+ currentCursorPositionColumn] =
							numbers[currentSignGroupIndex];
				}

				// Returns the current character to the first character in its group (blank space)
				currentSignGroupIndex = 0;

				// Moves the cursor to the next field. Cursor disappears if no fields left.
				if (currentCursorPositionColumn != 15) {
					setCursor(++currentCursorPositionColumn,
							currentCursorPositionRow);
				} else if (currentCursorPositionRow == 0) {
					currentCursorPositionRow = 1;
					currentCursorPositionColumn = 0;
					setCursor(0, 1);
				} else {
					++currentCursorPositionColumn;
					noBlink();
				}

			}

			// Long press (above 2 seconds) - sends the message and resets the display to the initial state.
			// Also turns on the green LED for 2 seconds to indicate the sending of a message.
			else {

				// Displays a message to the user terminal
				messageLength = sprintf(dataToSend,
						"RECEIVED MESSAGE:\r\n");
				CDC_Transmit_FS(dataToSend, messageLength);
				memset(dataToSend, 0, sizeof receivedData);
				HAL_Delay(1);

				// Displays a message sent by the user from LCD circuit on the terminal
				messageLength = sprintf(dataToSend, message);
				CDC_Transmit_FS(dataToSend, messageLength);

				// Resets the initial state of the circuit
				clear();
				currentCursorPositionRow = 0;
				currentCursorPositionColumn = 0;
				blink();
				memset(receivedData, 0, sizeof receivedData);
				memset(message, 0, sizeof message);

				// Turns on the green LED and launches the pulse of a timer set into one-pulse mode (2 seconds long)
				HAL_GPIO_WritePin(LED_Green_LCD_Circuit_GPIO_Port, LED_Green_LCD_Circuit_Pin, GPIO_PIN_SET);
				HAL_TIM_Base_Start_IT(&htim10);
				HAL_TIM_OnePulse_Start_IT(&htim10, TIM_CHANNEL_1);
			}

		}

		// Button handle is complete - restore the button its default state
		buttonOkSend.currentState = none;

	}

	// Clears the button trigger flag, so the new button press can be handled
	buttonTriggerFlag = 0;
}

void LCDCircuitCommandHandler(void) {

	if (strstr(receivedData, "SEND '")) {

		// Checks for the "SEND" command correctness
		if (!(receivedData[0] == 'S' && receivedData[1] == 'E'
				&& receivedData[2] == 'N' && receivedData[3] == 'D'
				&& receivedData[4] == ' ' && receivedData[5] == '\'')) {

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend,
					"Incorrect command.\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}

		// Saves the message begin and message end positions
		messageBegin = strchr(receivedData, '\'');
		messageEnd = strchr(messageBegin + 1, '\'');

		// Checks if the message has a closing bracket or if it's too long to be displayed
		if (messageEnd == NULL || messageEnd - messageBegin > 33) {

			// Displays a message to the user terminal
			messageLength =
					sprintf(dataToSend,
							"Incorrect command - the closing bracket was not detected (or the message was too long).\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}

		wrongMessageFlag = 0;

		// Checks if any characters are located after the closing bracket. If yes, disregards the message and prints the error.
		for (int i = messageEnd - (char*) receivedData; i < 40;
				++i) {

			if (isalnum(receivedData[i]) != 0) {

				// Displays a message to the user terminal
				messageLength =
						sprintf(dataToSend,
								"Incorrect command - detected characters after the closing bracket.\r\n");
				CDC_Transmit_FS(dataToSend, messageLength);
				memset(dataToSend, 0, sizeof dataToSend);
				wrongMessageFlag = 1;
				break;
			}

		}

		if (wrongMessageFlag) {
			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}

		// Increments the unread received messages number
		++receivedTextMessagesLeft;
		strncpy(receivedTextMessages[receivedTextMessagesLeft - 1],
				messageBegin + 1, messageEnd - messageBegin - 1);

		// Displays a message to the user terminal
		messageLength = sprintf(dataToSend, "Message '%s' sent.\r\n", receivedTextMessages[receivedTextMessagesLeft - 1]);
		CDC_Transmit_FS(dataToSend, messageLength);

		// Turns on the red LED to indicate a new received message
		HAL_GPIO_WritePin(LED_Red_LCD_Circuit_GPIO_Port,
		LED_Red_LCD_Circuit_Pin, GPIO_PIN_SET);

	}

	// Displays the number of the unread received messages
	else if(!strcmp(receivedData, "GET UNREAD MESSAGES NUMBER\0")) {

		// Displays a message to the user terminal
		messageLength = sprintf(dataToSend, "%d unread messages left.\r\n", receivedTextMessagesLeft);
		CDC_Transmit_FS(dataToSend, messageLength);

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;
	}

	// Checks for the "GET COMMANDS" command and executes it, if it matches
	else if(!strcmp(receivedData, "GET COMMANDS\0")) {

		// Displays a message to the user terminal. The message contains all available commands along with their descriptions. Delays are added to not transfer a few messages in one data buffer.
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

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;
	}

}
