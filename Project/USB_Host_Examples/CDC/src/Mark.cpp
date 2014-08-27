//------------------------------------------------------
//------------------------------------------------------
#include	<stdio.h>
#include	<string.h>

#include	"Mark.h"
//------------------------------------------------------
//extern TLCD			LCD					;
//------------------------------------------------------
//#define GPIO_KEYA						GPIOC
//#define Pin_KEYA						GPIO_Pin_13
//#define GPIO_KEYB						GPIOB
//#define Pin_KEYB						GPIO_Pin_2
//------------------------------------------------------
#define	KEYA_STATE	GPIO_ReadInputDataBit(GPIO_KEYA,Pin_KEYA)
#define	KEYB_STATE	GPIO_ReadInputDataBit(GPIO_KEYB,Pin_KEYB)
//------------------------------------------------------
#define	KEYA_STATE_DN					Bit_RESET
#define	KEYB_STATE_DN					Bit_SET
//------------------------------------------------------
void	GPIO_Enable(GPIO_TypeDef *GPIO_X)
{
	  if(GPIO_X == GPIOA) RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
 else if(GPIO_X == GPIOB) RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
 else if(GPIO_X == GPIOC) RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
 else if(GPIO_X == GPIOD) RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
 else if(GPIO_X == GPIOE) RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
 else if(GPIO_X == GPIOF) RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
}
//====================================================
void	GPIO_PinConfig(GPIO_TypeDef *GPIO_X,uint16_t Pin,GPIOMode_TypeDef Mode,GPIOSpeed_TypeDef Speed)
{GPIO_InitTypeDef		GPIO_InitStructure			;
 GPIO_InitStructure.GPIO_Pin   = Pin				;
 GPIO_InitStructure.GPIO_Mode  = Mode				;
 GPIO_InitStructure.GPIO_Speed = Speed				;
 GPIO_Init(GPIO_X, &GPIO_InitStructure)				;
}
//====================================================
void MARK_Init(void)
{uint16_t	Pin_MARK  = Pin_MARK0
#ifdef Pin_MARK1
 					  | Pin_MARK1
#endif
#ifdef Pin_MARK2					  
 					  | Pin_MARK2
#endif
#ifdef Pin_MARK3
 					  | Pin_MARK3
#endif
#ifdef Pin_MARK4
 					  | Pin_MARK4
#endif
#ifdef Pin_MARK5
 					  | Pin_MARK5
#endif			
										;
// uint16_t	Pin_LED = Pin_LED1 | Pin_LED2			;

// RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
 GPIO_Enable(GPIO_MARK)								;
// GPIO_Enable(GPIO_LED)								;
#ifdef	Pin_KEYA
 GPIO_Enable(GPIO_KEYA)								;
#endif
#ifdef	Pin_KEYB
 GPIO_Enable(GPIO_KEYB)								;
#endif
 
 GPIO_PinConfig(GPIO_MARK,Pin_MARK,GPIO_Mode_OUT)	;
// GPIO_PinConfig(GPIO_LED ,Pin_LED ,GPIO_Mode_OUT)	;
#ifdef	Pin_KEYA
 GPIO_PinConfig(GPIO_KEYA,Pin_KEYA,GPIO_Mode_IN_FLOATING);
#endif
#ifdef	Pin_KEYB
 GPIO_PinConfig(GPIO_KEYB,Pin_KEYB,GPIO_Mode_IN_FLOATING);
#endif 
// Это для контроля TIM3_CH1 !!!! это запуск ADC3.
// GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE)	;
// GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE)	;
// GPIO_PinConfig(GPIOB,GPIO_Pin_4,GPIO_Mode_AF_PP)		;

// GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15		;
// GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_	;
// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz	;
// GPIO_Init(GPIOA, &GPIO_InitStructure)				;
}
//------------------------------------------------------
#ifdef	Pin_KEYA
#define	Pin_KEY_AB
#endif
#ifdef	Pin_KEYB
#define	Pin_KEY_AB
#endif

#ifdef	Pin_KEY_AB
//------------------------------------------------------
volatile int	flKeyA = -10,flKeyB = -10	;
volatile int	flKeyA_UP=0,flKeyB_UP=0		;
volatile int	flKeyA_DN=0,flKeyB_DN=0		;
//------------------------------------------------------
EVENT_TYPE	OnEvent_KeyAB(TEvent* Event)
{
 if(Event->Type == evGetEvent){
   if(flKeyA_DN)     { flKeyA_DN = 0	;
     Event->Type = evKeyA_DN	;}
   else if(flKeyB_DN){ flKeyB_DN = 0	;
     Event->Type = evKeyB_DN			;}
 }
 return Event->Type	;}
//------------------------------------------------------
void		OnTimer_KeyAB(void)
{
#ifdef Pin_KEYA
  if(KEYA_STATE == KEYA_STATE_DN){
    if(flKeyA>0) 		  flKeyA = 0	;
	else if(flKeyA >-10){ flKeyA--		;
	  if(flKeyA == -10)   flKeyA_DN = 1	;}}
  else{
    if(flKeyA<0) 		  flKeyA = 0	;
	else if(flKeyA < 10){ flKeyA++		;
	  if(flKeyA == 10)    flKeyA_UP = 1	;}}
#endif
#ifdef Pin_KEYB
  if(KEYB_STATE == KEYB_STATE_DN){
    if(flKeyB>0) 		  flKeyB = 0	;
	else if(flKeyB >-10){ flKeyB--		;
	  if(flKeyB == -10)   flKeyB_DN = 1	;}}
  else{
    if(flKeyB<0)		  flKeyB = 0	;
	else if(flKeyB < 10){ flKeyB++		;
	  if(flKeyB == 10)	  flKeyB_UP = 1	;}}
#endif
}
//------------------------------------------------------
#endif
//------------------------------------------------------
