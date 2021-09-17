/* Includes ------------------------------------------------------------------*/
#include <inttypes.h>
#include "main.h"
#include "OLEDCircuit.h"
#include "SSD1331.h"

/* Extern variables ------------------------------------------------------------------*/
extern uint8_t dataToSend[100];
extern uint8_t receivedData[40];
extern uint8_t loopContinueFlag;
extern uint8_t messageLength;

/* Variables ------------------------------------------------------------------*/

// Current apple position - X and Y coordinates
uint8_t currentAppleXPos;
uint8_t currentAppleYPos;

// Checks if the start screen / end screen was already displayed. If not, prints the adequate message.
uint8_t startScreenDisplayedFlag;
uint8_t endScreenDisplayedFlag;

// Current apple position in 1-D version (positions are added to the array row by row)
uint16_t currentApplePosIndex;

// Delay between consecutive snake moves - the higher the game difficulty, the shorter the delay
uint16_t gameTimeDelay;

typedef enum {
	right, up, left, down
} Direction;

// Current snake move direction
Direction moveDirection;

// Pointer to the snake head (single linked list head)
SnakeBodyElement *snakeHead = NULL;

// Current game state - start screen / game on / end screen
GameState snakeGameState;

// Game difficulty level
enum difficultyLevel {
	easy, medium, hard
} gameDifficultyLevel;


/* Function bodies ---------------------------------------------------------*/
void StartGame(SnakeBodyElement **headRef) {

	// Clears the OLED display and sets the default snake move direction to right
	ssd1331_clear_screen(BLACK);
	moveDirection = right;

	// Sets the snake move delay according to the difficulty level
	switch(gameDifficultyLevel) {
	case easy:
		gameTimeDelay = 200;
		break;
	case medium:
		gameTimeDelay = 100;
		break;
	case hard:
		gameTimeDelay = 50;
		break;
	}

	// Creates a snake consisting of 3 body elements
	(*headRef) = (SnakeBodyElement*) malloc(
			sizeof(SnakeBodyElement));
	(*headRef)->xPos = 49;
	(*headRef)->yPos = 32;
	(*headRef)->nextBodyElement = (SnakeBodyElement*) malloc(
			sizeof(SnakeBodyElement));
	(*headRef)->nextBodyElement->xPos = 48;
	(*headRef)->nextBodyElement->yPos = 32;
	(*headRef)->nextBodyElement->nextBodyElement = (SnakeBodyElement*) malloc(
			sizeof(SnakeBodyElement));
	(*headRef)->nextBodyElement->nextBodyElement->xPos = 47;
	(*headRef)->nextBodyElement->nextBodyElement->yPos = 32;
	(*headRef)->nextBodyElement->nextBodyElement->nextBodyElement = NULL;

	// Draws the wall, snake and apple on the OLED display
	DrawWall();
	DrawSnake((*headRef));
	CreateNewApple((*headRef));

}

void AddSnakeBodyElement(SnakeBodyElement *head) {

	SnakeBodyElement *currentBodyElement = head;

	// Iterates through the snake body elements to get to the snake tail
	while (currentBodyElement->nextBodyElement != NULL) {
		currentBodyElement = currentBodyElement->nextBodyElement;
	}

	// Adds the snake body element after the snake tail
	currentBodyElement->nextBodyElement = (SnakeBodyElement*) malloc(
			sizeof(SnakeBodyElement));
	currentBodyElement->nextBodyElement->xPos = currentBodyElement->xPos; // Because previous body element will move forwards and the new element will take its position
	currentBodyElement->nextBodyElement->yPos = currentBodyElement->yPos;
	currentBodyElement->nextBodyElement->nextBodyElement = NULL;

}

void MoveForwards(SnakeBodyElement *head, Direction dir) {

	/* The algorithm used here allows to move any element position to its previous element position without using doubly linked list.
	 * It uses two helper positions to do so, which save two consecutive elements positions. The algorithm uses helper positions alternately.
	 * After the element position is saved in one of the helper positions, it is set to the another helper position and so on.
	 *
	 *
	 * Pseudocode:
	 *
	 * helpPos1 = 1st element position
	 * helpPos2 = 2nd element position
	 * 2nd element position = helpPos1
	 * helpPos1 = 3rd element position
	 * 3rd element position = helpPos2
	 * helpPos2 = 4th element position
	 * 4th element position = 3rd element position
	 *
	 * and so on...
	 *
	 */

	SnakeBodyElement *currentBodyElement = head;

	uint8_t helperCounter = 0;

	// Initializes both helper positions to head position
	uint8_t helperXPos1 = currentBodyElement->xPos;
	uint8_t helperYPos1 = currentBodyElement->yPos;

	uint8_t helperXPos2 = currentBodyElement->xPos;
	uint8_t helperYPos2 = currentBodyElement->yPos;

	// Move head position according to the current move direction
	switch (dir) {
	case right:
		++(head->xPos);
		break;
	case up:
		--(head->yPos);
		break;
	case left:
		--(head->xPos);
		break;
	case down:
		++(head->yPos);
		break;
	}

	// If a new position is equal to the wall positions
	if (head->xPos == 0 || head->yPos == 0 || head->xPos == 95 || head->yPos == 63) {

		// Moves head backwards to properly display the head-wall collision
		switch (dir) {
			case right:
				--(head->xPos);
				break;
			case up:
				++(head->yPos);
				break;
			case left:
				++(head->xPos);
				break;
			case down:
				--(head->yPos);
				break;
		}

		// Displays a message to the user terminal
		messageLength = sprintf(dataToSend, "You lost!\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);

		// Finishes the game
		GameOver(&head, endGameScreen);
		return;

	}

	// Sets the new head position pixel to green
	ssd1331_draw_point(head->xPos, head->yPos, GREEN);


	// If a new head position equals any body element position except tail, lose the game

	// Omits head itself
	currentBodyElement = currentBodyElement->nextBodyElement;

	// Iterates through snake body elements except tail
	while (currentBodyElement->nextBodyElement != NULL) {

		if (head->xPos == currentBodyElement->xPos
				&& head->yPos == currentBodyElement->yPos) {

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend, "You lost!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the game
			GameOver(&head, endGameScreen);
			return;
		}

		currentBodyElement = currentBodyElement->nextBodyElement;

	}

	// Checks if the new head position equals the apple position
	uint8_t appleEatenFlag = (head->xPos == currentAppleXPos
			&& head->yPos == currentAppleYPos);

	// If apple eaten - generate a new apple on the map and add a body element
	if (appleEatenFlag) {
		CreateNewApple(head);
		AddSnakeBodyElement(head);
	}

	// Omits the head, because its position was already changed
	currentBodyElement = head->nextBodyElement;

	while (true) {

		// Iterates through snake body elements except tail
		if (currentBodyElement->nextBodyElement != NULL) {

			// Performs the algorithm described at the beginning of this function
			if (helperCounter % 2 == 0) {

				helperXPos1 = currentBodyElement->xPos;
				helperYPos1 = currentBodyElement->yPos;

				currentBodyElement->xPos = helperXPos2;
				currentBodyElement->yPos = helperYPos2;

			} else {

				helperXPos2 = currentBodyElement->xPos;
				helperYPos2 = currentBodyElement->yPos;

				currentBodyElement->xPos = helperXPos1;
				currentBodyElement->yPos = helperYPos1;

			}

			++helperCounter;
			currentBodyElement = currentBodyElement->nextBodyElement;

		// Move the current tail forwards using the same algorithm as the previous body elements. If the apple was not eaten, clear the current tail pixel on the OLED display.
		} else {

			if (!appleEatenFlag) {
				ssd1331_draw_point(currentBodyElement->xPos, currentBodyElement->yPos, BLACK);
			}

			if (helperCounter % 2 == 0) {

				helperXPos1 = currentBodyElement->xPos;
				helperYPos1 = currentBodyElement->yPos;

				currentBodyElement->xPos = helperXPos2;
				currentBodyElement->yPos = helperYPos2;

			} else {

				helperXPos2 = currentBodyElement->xPos;
				helperYPos2 = currentBodyElement->yPos;

				currentBodyElement->xPos = helperXPos1;
				currentBodyElement->yPos = helperYPos1;

			}

			break;

		}

	}

}

void CreateNewApple(SnakeBodyElement *head) {

	// Array corresponding to all 96x64 OLED pixels, element value set to 1 if the position is occupied by an object (snake, apple, wall), otherwise set to 0
	uint16_t gameMap[6144];

	// Array corresponding to the possible new apple position ( 6144 pixels - wall pixels - minimal snake length (3 pixels) = 5825 [array length] )
	uint16_t helperArr[5825];

	// Indicates the last helperArr index storing the potential new apple position
	uint16_t lastHelperArrPosition = 0;


	// Sets all gameMap elements to 0 (default value)
	for (int i = 0; i < 6144; ++i) {
		gameMap[i] = 0;
	}

	// Sets all wall pixels to 1

	// Upper wall
	for (int j = 0; j < 96; ++j) {
		gameMap[j] = 1;
	}

	// Side walls
	for (int k = 1; k < 63; ++k) {
		gameMap[k * 96] = 1;
		gameMap[95 + k * 96] = 1;
	}

	// Lower wall
	for (int l = 6048; l < 6144; ++l) {
		gameMap[l] = 1;
	}


	// Iterate through snake body elements and set the gameMap elements corresponding to their positions to 1

	SnakeBodyElement *currentBodyElement = head;

	while (currentBodyElement->nextBodyElement != NULL) {

		gameMap[currentBodyElement->yPos * 96 + currentBodyElement->xPos] = 1;
		currentBodyElement = currentBodyElement->nextBodyElement;

	}

	gameMap[currentBodyElement->yPos * 96 + currentBodyElement->xPos] = 1;


	// Iterates through gameMap elements free of objects and adds their indices to the helperArr

	for (int l = 0; l < 6144; ++l) {

		if (gameMap[l] == 0) {
			++lastHelperArrPosition;
			helperArr[lastHelperArrPosition - 1] = l;
		}

	}


	// Changes the current apple position to the one drawed from available positions

	// Draws the position - 1-D array
	currentApplePosIndex = helperArr[rand() % lastHelperArrPosition];

	// Calculates the drawed X position
	currentAppleXPos = currentApplePosIndex % 96;

	// Calculates the drawed Y position
	currentAppleYPos = currentApplePosIndex / 96;

	// Draws the red pixel on the drawed position
	ssd1331_draw_point(currentAppleXPos, currentAppleYPos, RED);

}

void GameOver(SnakeBodyElement **headRef, GameState nextGameState) {

	// Blinks the snake body 4 times
	for (int i = 0; i < 4; ++i) {
		EraseSnake(*headRef);
		HAL_Delay(200);
		DrawSnake(*headRef);
		HAL_Delay(200);
	}

	// Clears the OLED display
	ssd1331_clear_screen(BLACK);


	// Iterates through the snake body and frees the memory associated with it
	SnakeBodyElement *currentBodyElement = *headRef;
	SnakeBodyElement *nextBodyElement;

	while (currentBodyElement != NULL) {

		nextBodyElement = currentBodyElement->nextBodyElement;
		free(currentBodyElement);
		currentBodyElement = nextBodyElement;

	}

	// Sets the head pointer back to NULL
	*headRef = NULL;

	// The game returns to the start screen or goes to the end screen
	snakeGameState = nextGameState;

}

void DrawWall(void) {

	// Draws the white rectangle representing the wall
	ssd1331_draw_rect(0, 0, 95, 63, WHITE);
	ssd1331_draw_point(95, 63, WHITE);		// Because ssd1331_draw_rect function omits this point...

}

void DrawSnake(SnakeBodyElement *head) {

	// Iterates through all snake body elements and sets the pixels corresponding to their positions to green

	SnakeBodyElement *currentBodyElement = head;

	while (currentBodyElement->nextBodyElement != NULL) {

		ssd1331_draw_point(currentBodyElement->xPos, currentBodyElement->yPos, GREEN);
		currentBodyElement = currentBodyElement->nextBodyElement;
	}

	ssd1331_draw_point(currentBodyElement->xPos, currentBodyElement->yPos, GREEN);

}

void EraseSnake(SnakeBodyElement *head) {

	// Iterates through all snake body elements and clears the pixels corresponding to their positions

	SnakeBodyElement *currentBodyElement = head;

	while (currentBodyElement->nextBodyElement != NULL) {

		ssd1331_draw_point(currentBodyElement->xPos, currentBodyElement->yPos, BLACK);
		currentBodyElement = currentBodyElement->nextBodyElement;

	}

	ssd1331_draw_point(currentBodyElement->xPos, currentBodyElement->yPos, BLACK);

}

void OLEDCircuitAction() {

	// If the game is on, moves the snake forwards after the delay corresponding to set difficulty level
	if (snakeGameState == gameOn) {

		HAL_Delay(gameTimeDelay);
		MoveForwards(snakeHead, moveDirection);

	}

	// If on the start game screen, prints the welcome message, if it wasn't printed before
	else if (snakeGameState == startGameScreen) {

		if (!startScreenDisplayedFlag) {

			ssd1331_display_string(0, 8, "Send 'START'", FONT_1608, WHITE);
			ssd1331_display_string(18, 24, "to play", FONT_1608, WHITE);
			ssd1331_display_string(24, 44, "SNAKE!", FONT_1608, GREEN);

			startScreenDisplayedFlag = 1;

		}

	}

	// If on the end game screen, prints the end game message, if it wasn't printed before
	else {

		if (!endScreenDisplayedFlag) {

			ssd1331_display_string(12, 0, "You lost!", FONT_1608, WHITE);
			ssd1331_display_string(0, 16, "Send 'START'", FONT_1608, WHITE);
			ssd1331_display_string(16, 32, "to play", FONT_1608, WHITE);
			ssd1331_display_string(22, 48, "again!", FONT_1608, WHITE);

			endScreenDisplayedFlag = 1;

		}

	}

}

void OLEDCircuitCommandHandler(void) {

	// If the game is on, change the move direction according to the read character (snake move is controlled by WSAD characters)
	// or stop the game if "STOP" message was received. Caps Lock needs to be disabled to avoid evaluating two conditions in every
	// move and optimise the program response speed - the user is informed via terminal message if the Caps Lock's on.
	if (snakeGameState == gameOn) {

		if (!strcmp(receivedData, "w\0")) {

			// Blocks the direction change if the new direction is opposite of the current direction
			if(moveDirection != down) {
				moveDirection = up;
			}

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;

		}
		else if (!strcmp(receivedData, "s\0")) {

			// Blocks the direction change if the new direction is opposite of the current direction
			if(moveDirection != up) {
				moveDirection = down;
			}

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;

		}
		else if (!strcmp(receivedData, "a\0")) {

			// Blocks the direction change if the new direction is opposite of the current direction
			if(moveDirection != right) {
				moveDirection = left;
			}

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;

		}
		else if (!strcmp(receivedData, "d\0")) {

			// Blocks the direction change if the new direction is opposite of the current direction
			if(moveDirection != left) {
				moveDirection = right;
			}

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;

		}
		else if (!strcmp(receivedData, "STOP\0")) {

			// Ends the game and returns the game to the start screen
			GameOver(&snakeHead, startGameScreen);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend, "Game aborted!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;

		}
		else if ( (!strcmp(receivedData, "W\0")) || (!strcmp(receivedData, "S\0")) || (!strcmp(receivedData, "A\0")) || (!strcmp(receivedData, "D\0")) ) {

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend, "Disable Caps Lock!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;

		}

	}

	// If currently on the start / end game screen, game can be started with "START" command, or the difficulty can be set with "SET DIFFICULTY [EASY/MEDIUM/HARD] command"
	else if (snakeGameState == startGameScreen || snakeGameState == endGameScreen) {

		if (!strcmp(receivedData, "START\0")) {

			startScreenDisplayedFlag = 0;
			snakeGameState = gameOn;
			StartGame(&snakeHead);

			// Displays a message to the user terminal
			messageLength = sprintf(dataToSend, "Game started!\r\n");
			CDC_Transmit_FS(dataToSend, messageLength);

			// Finishes the main loop iteration
			loopContinueFlag = 1;
			return;
		}
		else if (strstr(receivedData, "DIFFICULTY")) {
			if (!strcmp(receivedData, "SET DIFFICULTY EASY\0")) {
				gameDifficultyLevel = easy;

				// Displays a message to the user terminal
				messageLength = sprintf(dataToSend, "Set game difficulty to Easy!\r\n");
				CDC_Transmit_FS(dataToSend, messageLength);

				// Finishes the main loop iteration
				loopContinueFlag = 1;
				return;
			}
			else if (!strcmp(receivedData, "SET DIFFICULTY MEDIUM\0")) {
				gameDifficultyLevel = medium;

				// Displays a message to the user terminal
				messageLength = sprintf(dataToSend, "Set game difficulty to Medium!\r\n");
				CDC_Transmit_FS(dataToSend, messageLength);

				// Finishes the main loop iteration
				loopContinueFlag = 1;
				return;
			}
			else if (!strcmp(receivedData, "SET DIFFICULTY HARD\0")) {
				gameDifficultyLevel = hard;

				// Displays a message to the user terminal
				messageLength = sprintf(dataToSend, "Set game difficulty to Hard!\r\n");
				CDC_Transmit_FS(dataToSend, messageLength);

				// Finishes the main loop iteration
				loopContinueFlag = 1;
				return;
			}
			else if (!strcmp(receivedData, "GET DIFFICULTY\0")) {

				switch (gameDifficultyLevel) {
				case easy:
					// Displays a message to the user terminal
					messageLength = sprintf(dataToSend, "Current difficulty: Easy\r\n");
					CDC_Transmit_FS(dataToSend, messageLength);
					break;
				case medium:
					// Displays a message to the user terminal
					messageLength = sprintf(dataToSend, "Current difficulty: Medium\r\n");
					CDC_Transmit_FS(dataToSend, messageLength);
					break;
				case hard:
					// Displays a message to the user terminal
					messageLength = sprintf(dataToSend, "Current difficulty: Hard\r\n");
					CDC_Transmit_FS(dataToSend, messageLength);
					break;
				}

				// Finishes the main loop iteration
				loopContinueFlag = 1;
				return;
			}
			else {
				// Displays a message to the user terminal
				messageLength = sprintf(dataToSend,
						"Incorrect command.\r\n");
				CDC_Transmit_FS(dataToSend, messageLength);

				// Finishes the main loop iteration
				loopContinueFlag = 1;
				return;
			}
		}

	}

	// Checks for the "GET COMMANDS" command and executes it, if it matches
	if(!strcmp(receivedData, "GET COMMANDS\0")) {

		// Displays a message to the user terminal. The message contains all available commands along with their descriptions. Delays are added to not transfer a few messages in one data buffer.
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

		// Displays a message to the user terminal
		messageLength = sprintf(dataToSend, "Incorrect command.\r\n");
		CDC_Transmit_FS(dataToSend, messageLength);

		// Finishes the main loop iteration
		loopContinueFlag = 1;
		return;
	}

}
