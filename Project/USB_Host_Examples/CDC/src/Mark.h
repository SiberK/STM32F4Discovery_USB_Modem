//------------------------------------------------------
//------------------------------------------------------
#ifndef	MARK_H
#define	MARK_H
//------------------------------------------------------
//#include "stm32f10x.h"
#include <stm32f4xx.h>

#include "EventGUI.h"
//------------------------------------------------------
#define	GPIO_LED					GPIOB
#define Pin_LED1				GPIO_Pin_0
#define Pin_LED2				GPIO_Pin_1

#define	GPIO_MARK					GPIOE
#define Pin_MARK0				GPIO_Pin_0
#define Pin_MARK1				GPIO_Pin_1
#define Pin_MARK2				GPIO_Pin_2
#define Pin_MARK3				GPIO_Pin_3
#define Pin_MARK4				GPIO_Pin_4
#define Pin_MARK5				GPIO_Pin_5

// #define GPIO_Pin_PWR_SW				GPIO_Pin_12
// #define GPIO_Pin_PWR_ON				GPIO_Pin_15
// #define	GPIO_PWR_ON					GPIOB
// #define	GPIO_PWR_SW					GPIOB
// #define	RCC_APB2Periph_GPIO_PWR_SW	RCC_APB2Periph_GPIOB
//------------------------------------------------------
//------------------------------------------------------
#ifdef Pin_MARK0
#define	MARK_0H   GPIO_SetBits  (GPIO_MARK,Pin_MARK0);
#define	MARK_0L   GPIO_ResetBits(GPIO_MARK,Pin_MARK0);
#define	MARK_0X	  {if(GPIO_ReadInputDataBit(GPIO_MARK,Pin_MARK0)) MARK_0L  else MARK_0H  }
#endif
#ifdef Pin_MARK1
#define	MARK_1H   GPIO_SetBits  (GPIO_MARK,Pin_MARK1);
#define	MARK_1L   GPIO_ResetBits(GPIO_MARK,Pin_MARK1);
#define	MARK_1X	  {if(GPIO_ReadInputDataBit(GPIO_MARK,Pin_MARK1)) MARK_1L  else MARK_1H  }
#endif
#ifdef Pin_MARK2
#define	MARK_2H   GPIO_SetBits  (GPIO_MARK,Pin_MARK2);
#define	MARK_2L   GPIO_ResetBits(GPIO_MARK,Pin_MARK2);
#define	MARK_2X	  {if(GPIO_ReadInputDataBit(GPIO_MARK,Pin_MARK2)) MARK_2L  else MARK_2H  }
#endif
#ifdef Pin_MARK3
#define	MARK_3H   GPIO_SetBits  (GPIO_MARK,Pin_MARK3);
#define	MARK_3L   GPIO_ResetBits(GPIO_MARK,Pin_MARK3);
#define	MARK_3X	  {if(GPIO_ReadInputDataBit(GPIO_MARK,Pin_MARK3)) MARK_3L  else MARK_3H  }
#endif
#ifdef Pin_MARK4
#define	MARK_4H   GPIO_SetBits  (GPIO_MARK,Pin_MARK4);
#define	MARK_4L   GPIO_ResetBits(GPIO_MARK,Pin_MARK4);
#define	MARK_4X	  {if(GPIO_ReadInputDataBit(GPIO_MARK,Pin_MARK4)) MARK_4L  else MARK_4H  }
#endif
#ifdef Pin_MARK5
#define	MARK_5H   GPIO_SetBits  (GPIO_MARK,Pin_MARK5);
#define	MARK_5L   GPIO_ResetBits(GPIO_MARK,Pin_MARK5);
#define	MARK_5X	  {if(GPIO_ReadInputDataBit(GPIO_MARK,Pin_MARK5)) MARK_5L  else MARK_5H  }
#endif
//------------------------------------------------------
//------------------------------------------------------
#define	LED1_ON   GPIO_SetBits  (GPIO_LED,Pin_LED1);
#define	LED1_OFF  GPIO_ResetBits(GPIO_LED,Pin_LED1);
#define	LED2_ON   GPIO_SetBits  (GPIO_LED,Pin_LED2);
#define	LED2_OFF  GPIO_ResetBits(GPIO_LED,Pin_LED2);
//------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------
void		GPIO_Enable(GPIO_TypeDef *GPIO_X)	;
void		GPIO_PinConfig(GPIO_TypeDef *GPIO_X,uint16_t Pin,GPIOMode_TypeDef Mode,GPIOSpeed_TypeDef Speed=GPIO_Speed_50MHz);
void		MARK_Init(void)						;
EVENT_TYPE	OnEvent_KeyAB(TEvent* Event)		;
void		OnTimer_KeyAB(void)					;
// void 	PowerSwitch_Init(void)	;
// void	PowerOn(void)			;
// void	PowerOff(void)			;
// int 	PowerSwDown(int vDly)	;
//------------------------------------------------------
extern volatile uint32_t 	Timer1k				;/* 1kHz increment timer */
//------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------
#endif
//------------------------------------------------------
