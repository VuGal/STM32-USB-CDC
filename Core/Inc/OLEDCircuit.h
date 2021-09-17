#ifndef _OLEDCIRCUIT_H_
#define _OLEDCIRCUIT_H_

/* Exported types ------------------------------------------------------------*/

// Element of a snake body - linked list element with its data (xPos and yPos) and a pointer
// to the next element (*nextBodyElement)
typedef struct snakeBodyElement {
	uint8_t xPos;
	uint8_t yPos;
	struct snakeBodyElement *nextBodyElement;
} SnakeBodyElement;

// Indicates the current OLED circuit state
typedef enum {
	startGameScreen, gameOn, endGameScreen
} GameState;

/* Exported functions prototypes ---------------------------------------------*/
void OLEDCircuitCommandHandler(void);

#endif
