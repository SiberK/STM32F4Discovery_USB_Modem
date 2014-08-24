/* Includes ------------------------------------------------------------------*/
#include 	"UsbhCore.h"
#include	"usart_GSM.h"
#include	"Log.h"
//--------------------------------------------------------------
void	InitUSART(void);
void	TIM_MS_Init(void);
//--------------------------------------------------------------
Led_TypeDef				LEDind = LED3	;
TEvent					FEvent			;
TUsbhCore				UsbhCore		;
TUsartGSM				UsartGSM		;
//--------------------------------------------------------------
void		InitAll(void)
{
 InitUSART()		;
 UsbhCore.Init()	;
 UsartGSM.Init()	;
 TIM_MS_Init()		;
}
//--------------------------------------------------------------
int main(void)
{uint32_t 			i = 0				;
 TEvent*			Event = &FEvent		;
 EVENT_TYPE			prevType = 0		;
    
 InitAll()			;
 
 for(;1;){
   if(prevType == Event->Type || !Event->Type){
     Event->Type = evGetEvent				;}
   prevType = Event->Type					;
   
   UsbhCore.OnEvent(Event)					;
   UsartGSM.OnEvent(Event)					;
   
   if(Event->Type == evDbgMsg1 || Event->Type == evDbgMsg2){
     Log.d(Event->strData[0])				;
	 Event->Type = evGetEvent				;
   }
    
   if (i++ >= 0x10000){ i = 0				;
     STM_EVAL_LEDToggle(LEDind)				;}      
 }
}
//--------------------------------------------------------------


//--------------------------------------------------------------
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
  while (1)
  {}
}
#endif
//--------------------------------------------------------------
//------------------------------------------------------
#define	TIM_MS					TIM6
#define	RCC_APB1Periph_TIM_MS 	RCC_APB1Periph_TIM6
#define	TIM_MS_IRQn				TIM6_DAC_IRQn
#define	TIM_MS_IRQHandler		TIM6_IRQHandler
//------------------------------------------------------
// таймер для милисекундных отсчётов
void	TIM_MS_Init(void)
{TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure	;
// TIM_OCInitTypeDef			TIM_OCInitStructure		;
// TIM_BDTRInitTypeDef		TIM_BDTRInitStructure	;
 NVIC_InitTypeDef NVIC_InitStructure;

   /* TIM_MS clock enable */
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM_MS ,ENABLE);

  /* Enable the TIM_MS gloabal Interrupt */
 NVIC_InitStructure.NVIC_IRQChannel = TIM_MS_IRQn;
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

 NVIC_Init(&NVIC_InitStructure);

  /* TIM_MS Peripheral Configuration */ 
 TIM_DeInit(TIM_MS);

  /* Time Base configuration */
 TIM_TimeBaseStructure.TIM_Prescaler         = SystemCoreClock/1000000-1;
 TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
 TIM_TimeBaseStructure.TIM_Period            = 999;
 TIM_TimeBaseStructure.TIM_ClockDivision     = 0x0;
 TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0;

 TIM_TimeBaseInit(TIM_MS,&TIM_TimeBaseStructure);
  /* TIM IT enable */
 TIM_ITConfig(TIM_MS,TIM_IT_Update, ENABLE)		;

  /* TIM_MS counter enable */
 TIM_Cmd(TIM_MS,ENABLE);

}
//------------------------------------------------------
#ifdef __cplusplus
 extern "C" 
#endif
void TIM_MS_IRQHandler(void)
{
 if(TIM_GetITStatus(TIM_MS,TIM_IT_Update) != RESET){
   TIM_ClearITPendingBit(TIM_MS,TIM_IT_Update)		;
   
   TUsartGSM::OnTimer()	;
 }
}
//------------------------------------------------------
void	InitUSART(void)
{
 USART_InitTypeDef USART_InitStructure;

 USART_InitStructure.USART_BaudRate 	= 115200;
 USART_InitStructure.USART_WordLength 	= USART_WordLength_8b;
 USART_InitStructure.USART_StopBits 	= USART_StopBits_1;
 USART_InitStructure.USART_Parity 		= USART_Parity_No;
 USART_InitStructure.USART_Mode 		= USART_Mode_Rx | USART_Mode_Tx;
 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

 STM_EVAL_COMInit(COM2, &USART_InitStructure);
}
//--------------------------------------------------------------
#ifdef __cplusplus
 extern "C" 
#endif
int fputc(int ch, FILE *f)
{
#define ITM_Port8(n) (*((volatile unsigned char *)(0xE0000000+4*n)))
 ITM_Port8(0) = (uint8_t)ch; /* displays value in ASCII */
 USART_SendData(EVAL_COM2, (uint8_t) ch);

 while (USART_GetFlagStatus(EVAL_COM2, USART_FLAG_TC) == RESET);
 while (ITM_Port8(0) == 0);

 return ch;
}
/**************END OF FILE******/
