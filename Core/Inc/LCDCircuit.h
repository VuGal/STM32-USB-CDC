#ifndef _LCDCIRCUIT_H_
#define _LCDCIRCUIT_H_

/* Exported types ------------------------------------------------------------*/

// buttonState enum and Button struct are used for button debouncing
enum buttonState {
	none, pressed, released
};

typedef struct {
	enum buttonState currentState;
	uint32_t pressTime;
	uint32_t releaseTime;
} Button;

/* Exported functions prototypes ---------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void ButtonTrigger(void);
void LCDCircuitCommandHandler(void);

#endif
