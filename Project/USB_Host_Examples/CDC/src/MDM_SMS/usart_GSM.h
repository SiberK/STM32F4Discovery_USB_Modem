#ifndef	USART_GSM_H
#define	USART_GSM_H
//*******************************************************************
#include 		<stm32f4xx.h>
#include		<stdint.h>
#include		"EventGUI.h"
#include		"FiFo.h"
//#include		"Store.h"
//*******************************************************************
typedef char*		(*TGetString)(char*,int)		;
typedef	uint32_t	(*TGetUInt32Value)(void)		;
typedef	void		(*TSetUInt32Value)(uint32_t)	;
//*******************************************************************
#pragma	pack(push,1)
//-----------------------------------------------------
//struct	TStoreRecordGSM:public TStoreRecord{
// char					MasterNmbr[16]		;
// char					Dummy1[128-16]		;
//											 // 128
// TStoreRecordGSM(void){ Sign = SIGN_GSM		;
//						Len = sizeof(*this)	;
//						Len = ((Len+3)/4)*4	;}
//};
//-----------------------------------------------------
#pragma pack(pop)
//==================================================
//typedef enum{
// stgNone 		= 0,
// stgOFF  		= 1,
// stgPwrON 	= 2
//} TGSM_Stage	;
//==================================================
//*******************************************************************
class	TUsartGSM/*:public TUsart*/{
// TStoreFlash			StoreFlash				;
// TStoreRecordGSM		StoreRec				;
 int					timOperate				;
 int					timOut,timRepeat		;
 int					timGuardSMS				;// защитная пауза от повтора СМС
 int					timTxPause				;// защитная пауза, чтоб UART GSM модема успел переключится на прием 
 short					FLenSMS					;
 u8						FCntMemSMS,FTtlMemSMS	;
 u8						FIxInSMS,FIxMemSMS		;
 u8						FIxRdSMS,FIxDelSMS		;// СМС, которую надо прочитать, удалить
 u8						FPrsSMS,FReadAll		;
 u8						NeedSendSMS				;
 char					SttPhase,prSttPhase		;
 char					PhoneNmbrCall[20]		;
 char					PhoneNmbrSMS[20]		;
 char					PhoneNmbrOut[20]		;
 char					TextInSMS[200]			;
 uint32_t				PswGSM					;
 char*					strRcv					;
 int					cntRcv					;
 uint16_t				flEventNeed,flEventMsg	;
 short					flValueNeed				;
 short					flWaitSMS				;// ожидаем текст СМС
 char					flInCall,flInSMS		;

 TFiFo					FifoRx		;
 TFiFo					FifoTx		;

public:
 int					flMdmPresent				;
 int/*TGSM_State*/		State,StateTrg,prStateTrg	;// текущее и целевое состояние
// TGSM_Stage				Stage,StageTrg			;// текущая и целевая стадия
// int					StgPhase					;
 int					MsgAwt						;// ожидаемое сообщение

 static TUsartGSM*		Instance					;

				TUsartGSM(void)						;

 void					InitGSM(void)				;
 void					InitHW(void)				;
 void					Init(void)					;
 
 void					SetFifoRx(char* buf,int size,callbackIfCR fnIfCR=0,const char* strEndS=0)
						{ FifoRx.Set(buf,size,fnIfCR,strEndS)	;}
 void					SetFifoTx(char* buf,int size,callbackIfCR fnIfCR=0,const char* strEndS=0)
						{ FifoTx.Set(buf,size,fnIfCR,strEndS)	;}
 void					ResetFiFo(void){ FifoRx.Reset()	; FifoTx.Reset()	;}

 void					WriteString(char* Str)					;
 void					WriteStringLN(const char* Str)			;
 void					WriteStringLN_P(const char* Str,const char* Prm)	;
 int					SendChar(int ch)						;
 char*					GetS(char* buf,int lenBuf)				;// взять одну строку, если есть
 __inline void			EnableRxIRQ(void)	;
 __inline void			DisableRxIRQ(void)	;
 __inline void			EnableTxIRQ(void)	;
 __inline void			DisableTxIRQ(void)	;
 
 EVENT_TYPE				OnEvent(TEvent* Event)				;
		void			FOnTimer(void)						;
 static void			OnTimer(void)						;
			
//		void			ReceiveBuf (class TFiFo* fifo,int cnt)	;
// static void			FReceiveBuf(class TFiFo* fifo,int cnt)	;
// void					ReqCntSMS(void)						;
// TGetUInt32Value		FnGetPswGSM							;
// TSetUInt32Value		FnSetPswGSM							;
 TGetString				FnGetInfSMS							;
private:
 uint16_t				OnEventGSM(void)					;
 uint16_t				Parse(char* str,int cnt)			;
 uint16_t				ParseCPMS(char* str)				;
 uint16_t				ParseCMGR(char* str)				;
 uint16_t				ParseCMTI(char* str)				;
 uint16_t				ParseCLIP(char* str)				;
 char*					DecodeHEX_SMS (char* str,char* buf)	;
 uint16_t				ParseSMS (char* str)				;
 uint16_t				ParseTextSMS (char* str)			;
 
 int					Operate(int Msg=0)					;// выполнять последовательность действий
 int					Operate_PwrON (int Msg=0)			;// выполнять последовательность действий
 int					Operate_INIT (int Msg=0)			;// выполнять последовательность действий
 int					Operate_REQ_CNT_SMS(int Msg=0)		;// выполнять последовательность действий
 int					Operate_RD_SMS(int Msg=0)			;// выполнять последовательность действий
 int					Operate_DEL_SMS(int Msg)			;
 int					Operate_DEL_SMS1(int Msg=0)			;// выполнять последовательность действий
 int					Operate_DEL_ALL_SMS(int Msg=0)		;// выполнять последовательность действий
 int					Operate_InfSMS(int Msg=0)			;// выполнять последовательность действий
 int					Operate_ATH(int Msg=0)				;// выполнять последовательность действий
 int					Operate_IDLE(int Msg=0)				;// выполнять последовательность действий

 char*					GetMasterNmbr(void)					;
 void					SetMasterNmbr(char* src)			;
};
//*******************************************************************
//*******************************************************************
//*******************************************************************
#endif
