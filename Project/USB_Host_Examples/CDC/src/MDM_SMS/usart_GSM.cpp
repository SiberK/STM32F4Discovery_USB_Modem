//***************************************************************
#include		<string.h>
#include		<stdio.h>
#include		<stdlib.h>
#include		<ctype.h>
#include		"usart_GSM.h"
//***************************************************************
//***************************************************************
//#define		USART_GSM			USART2
#define		LenBF				200

//// Enable Vin
//#define		VEN_PORT			GPIOC
//#define		VEN_PIN				GPIO_Pin_7
//// Power ON
//#define		PWR_ON_PORT			GPIOC
//#define		PWR_ON_PIN			GPIO_Pin_6
//// STATE flag
//#define		STT_PORT			GPIOB
//#define		STT_PIN				GPIO_Pin_10
//// NETLIGHT flag
//#define		NTL_PORT			GPIOB
//#define		NTL_PIN				GPIO_Pin_11

#define		TIM_PULSE_PWR_ON	1200
#define		TIM_WAIT_PWR_ON		800
#define		TIM_WAIT_500		200
#define		TIM_REPEAT_REQ		50000
#define		TIM_IDLE			600000
#define		TIM_REPEAT_SMS		10000
#define		TIM_GUARD_SMS		60000
#define		TIM_TX_PAUSE		100
//***************************************************************
#define		StrCmp(X,Y)	strncasecmp(X,Y,strlen(Y))
//***************************************************************
static	char	GsmBufRx[LenBF]	;
static	char	GsmBufTx[LenBF]	;
static	char	RcvBuf[LenBF]						;
static	char	SmsInBuf[LenBF]						;
static	char	SmsOutBuf[LenBF]					;
static	char	StrDbg[LenBF], *strDbg = 0			;
static	char	StrStt[LenBF], *strStt = 0			;
static	char	StrMasterNmbr[40]					;
const	char*	strMsg = 0							;
//static	char	GsmMsg[80]						;
//***************************************************************
const	char	strOK[] 			= "OK"			;
const	char	strAT[] 			= "AT"			;
const	char	strERROR[] 			= "ERROR"		;
const	char	strNO_CRR[]			= "NO CARRIER"	;
const	char	strPROMPT[]			= ">"			;
const	char	smbPROMPT			= '>'			;
const	char	smbCtrlZ			= 0x1A			;
const	char	strCCLK[]			= "AT+CCLK?"	;
const	char	strCLIP1[]			= "AT+CLIP=1"	;// Calling Line Identification Presentation
const	char	strCPAS[]			= "AT+CPAS"		;// Phone Activity Status
const	char	strCSQ[]			= "AT+CSQ"		;// Signal Quality Report
const	char	strSV[]				= "AT+GSV"		;// Display Product Identification
const	char	strCOPS[]			= "AT+COPS?"	;// Display Current Operator
const	char	strGSN[]			= "AT+GSN"		;// Request IMEI
const	char	strFMT_PDUp[]		= "AT+CMGF=0"	;
const	char	strFMT_PDUt[]	 	= "AT+CMGF=1"	;
//const	char	strGET_LST_SMS[]	= "AT+CMGL=4,0"	;
const	char	strSMS_STRG1[]  	= "AT+CPMS=?"	;
const	char	strREQ_CNT_SMS[]	= "AT+CSQ;+CPMS?"	;
const	char	strRD_SMS[]  		= "AT+CMGR=%d"	;
const	char	strRD_SMS1[]  		= "AT+CMGR=1"	;// читать СМС_1
const	char	strDEL_SMS[]		= "AT+CMGD=%d,0";// удалить одну СМС
const	char	strDEL_SMS1[]		= "AT+CMGD=1,0"	;// удалить СМС_1
const	char	strDEL_ALL_SMS[]	= "AT+CMGD=1,1"	;
const	char	strSEND_SMS[]		= "AT+CMGS="	;
const	char	strCLTS[]			= "AT+CLTS=1"	;// Get Local Timestamp
const	char	strINIT1[]			= "AT+CMGF=1;+COPS?;+GSV;+CSQ;+CLIP=1;+CLTS=1;+CCLK?"	;

//const	char	str3[]="";
//const	char	str4[]="";

const	char	strAnsCPMS[]		= "+CPMS: \"SM\""	;
const	char	strAnsCMGR[]		= "+CMGR: "			;
const	char	strAnsCMTI[]		= "+CMTI: "			;// Пришла СМС!
const	char	strAnsCLIP[]		= "+CLIP: "			;

const	char	strMsg_INIT_OK[]   	= "->INIT_OK"		;
const	char	strMsg_INIT_ERR[]  	= "->INIT_ERR"		;
const	char	strMsg_CPMS_OK[] 	= "->CPMS_OK"		;
const	char	strMsg_RD_SMS_OK[] 	= "->RD_SMS_OK"		;
const	char	strMsg_DEL_ALL_SMS_OK[] = 	"->DEL_ALL_SMS_OK";
const	char	strMsg_DEL_SMS_OK[] = "->DEL_SMS_OK"	;
const	char	strMsg_DEL_SMS_ERR[]= "->DEL_SMS_ERR"	;
const	char	strMsg_SMS_SEND_OK[]= "->SMS SEND OK"	;
const	char	strMsg_SMS_SEND_ERR[]="->SMS SEND ERR"	;
const	char	strNO_INFO[]		= "NO INFO\032\r\n"	;

const	char	cmdNewPass[]		= "np"				;
const	char	cmdStart[]			= "start"			;
const	char	cmdStop[]			= "stop"			;
const	char	cmdTermTrg[]		= "t"				;
const	char	cmdMaster[]			= "ms"				;
const	char	strValidNmbr[]		= "\"+79"			;
//***************************************************************
enum{
  msgEmpty 		= 0,
  msgOK 		= 1,
  msgERROR		= 2,
  msgNO_CRR		= 3,
  msgRING		= 4,
  msgCLIP		= 5,
  msgMemSMS		= 6,
  msgInSMS		= 7,
//  msgCMTI		= 8,
  msgPROMPT		= 9,
  msgTimeOut	= 10,
  msgSMS_PARSED = 11,
  msgDelSMS,
  msgSendSMS,
  msgGSM
};
//-----------------------------------------------------
typedef enum{
  sttNone 		= 0,
  sttOFF		= 1,
  sttReady   	= 2,
  sttPwrON	 	= 3,
  sttINIT		= 4,
  sttREQ_CNT_SMS= 5,
  sttRD_SMS		= 6,
  sttDEL_ALL_SMS= 7,
  sttInfSMS		= 8,
  sttDEL_SMS	= 9,
  sttDEL_SMS1	= 10,
  sttIDLE		= 11,
  sttATH		= 12
}TGSM_State		;

const char*	strStat[sttATH-sttNone+1]={
"None"			,
"OFF"			,
"Ready"			,
"PwrON"			,
"INIT"			,
"REQ_CNT_SMS"	,
"RD_SMS"		,
"DEL_ALL_SMS"	,
"InfSMS"		,
"DEL_SMS"		,
"DEL_SMS1"		,
"IDLE"			,
"ATH"
};
//***************************************************************
int	ParseParams(char* str,char* dlm,char* Prm,int* Val,const int CntPrm,const int LenPrm);
//***************************************************************
TUsartGSM*		TUsartGSM::Instance=0	;
//***************************************************************
#define		VEN_PIN_SET()		GPIO_SetBits(VEN_PORT,VEN_PIN)
#define		VEN_PIN_RST()		GPIO_ResetBits(VEN_PORT,VEN_PIN)
#define		PWR_ON_PIN_SET()	GPIO_SetBits(PWR_ON_PORT,PWR_ON_PIN)
#define		PWR_ON_PIN_RST()	GPIO_ResetBits(PWR_ON_PORT,PWR_ON_PIN)
//***************************************************************
		TUsartGSM::TUsartGSM(void){}
//***************************************************************
EVENT_TYPE	TUsartGSM::OnEvent(TEvent* Event)
{uint16_t	msgMsg = msgEmpty	;

 strRcv = GetS(RcvBuf,LenBF)								;// есть принятые строки?
 cntRcv = (strRcv && *strRcv)? strlen(strRcv) : 0			;// сколько их?
 if(strRcv && cntRcv){ msgMsg = Parse(strRcv,cntRcv)		;  strRcv = 0	;}

 if(!msgMsg) msgMsg = OnEventGSM()							;
 if(!timOut && !msgMsg){ msgMsg = msgTimeOut ; timOut = -1	;}
 if(msgMsg != msgEmpty){ timOut = Operate(msgMsg)			;}// !!!!!!!!!!!!!!!!!
 if(timOut <= 0) timOut = TIM_IDLE							;

 if(prStateTrg != StateTrg || prSttPhase != SttPhase){
   prStateTrg = StateTrg	; prSttPhase = SttPhase			;
   sprintf(StrStt,"%s, %d",strStat[StateTrg],SttPhase)		; strStt = StrStt	;}
 
 switch(Event->Type){
 case evGetEvent:
   if(strDbg)     { Event->Type = evDbgMsg1	; Event->strData[0] = strDbg		; strDbg = 0	;}
   else if(strMsg){ Event->Type = evDbgMsg2	; Event->strData[0] = (char*)strMsg	; strMsg = 0	;}
   else if(flEventNeed){ Event->Type = flEventNeed	; Event->Value = flValueNeed; flEventNeed=0	;}
   else if(strStt){ Event->Type = evStat1	; Event->strData[0] = strStt		; strStt = 0	;}
 break	;
 
// case evClearPswGSM	: PswGSM = 0	; if(FnSetPswGSM ) FnSetPswGSM(PswGSM)			; break	;
 
	// если произошло событие по которому необходимо отправить инфоСМС, то
 case evEventSMS :  strncpy(PhoneNmbrSMS,GetMasterNmbr(),16)					;// отправить инф. СМС на MasterNmbr
					if(!StrCmp(PhoneNmbrSMS,strValidNmbr)){		 				 // если он валидный
					  NeedSendSMS = 1 ; timGuardSMS = 0							;}
					else{ *PhoneNmbrSMS = 0	; SetMasterNmbr(PhoneNmbrSMS)		;}// иначе, стереть его
		break	;
 } 
 return Event->Type		;}
//***************************************************************


//***************************************************************
uint16_t TUsartGSM::OnEventGSM(void)
{uint16_t	msgMsg = msgEmpty	;

 if(StateTrg == sttIDLE){
   if(*PhoneNmbrCall && !timTxPause){ 					 // если есть вх вызов и истекла защитная пауза
	 if(NeedSendSMS)									 // если надо отправить СМС
	   strcpy(PhoneNmbrOut,PhoneNmbrCall)				;// то для этого возьмем номер
	 *PhoneNmbrCall = 0									;// сбросим флаг
	 msgMsg = msgCLIP		;
   }
   else if(FIxDelSMS && !timTxPause){					 // если есть СМС, которую надо удалить
	 msgMsg   = msgDelSMS	;
   }
   else if(NeedSendSMS && !timTxPause && !timGuardSMS){
	 if(*PhoneNmbrSMS && !*PhoneNmbrOut){ 
	   strcpy(PhoneNmbrOut,PhoneNmbrSMS)	; *PhoneNmbrSMS = 0	;}
	 if(*PhoneNmbrOut) msgMsg = msgSendSMS	;	 
   }
   else if(FIxInSMS && !timTxPause){					 // если есть вх СМС и истекла защитная пауза
	 FIxRdSMS = FIxInSMS	; FIxInSMS = 0				;// возьмем IxSMS и сбросим флаг
	 msgMsg   = msgInSMS	;
   }
   else if(FIxMemSMS){
     FIxRdSMS = FIxMemSMS	; FIxMemSMS++	;
	 if(FIxMemSMS > FTtlMemSMS) FIxMemSMS=FTtlMemSMS=FCntMemSMS=0	;
	 msgMsg = msgMemSMS		;
   }
 }

 return	msgMsg	;}
//***************************************************************

//***************************************************************
int		TUsartGSM::Operate(int Msg)
{int	result = TIM_WAIT_PWR_ON;

 switch(Msg){
   case	msgCLIP 		: StateTrg = sttATH		; SttPhase = 1	; break	;
   case msgInSMS		: StateTrg = sttRD_SMS	; SttPhase = 1	; break	;
   case msgMemSMS		: StateTrg = sttRD_SMS	; SttPhase = 1	; break	;
   case msgDelSMS		: StateTrg = sttDEL_SMS	; SttPhase = 1	; break	;
   case msgSendSMS		: StateTrg = sttInfSMS	; SttPhase = 1	; break	;
   case msgSMS_PARSED 	: break	;
   case msgTimeOut		: break	;
 }
 
 if(State == StateTrg){
   SttPhase = 1		;
   switch((int)StateTrg){
      case sttNone			: StateTrg = sttIDLE			; break	;
	  case sttPwrON			: StateTrg = sttINIT			; break	;
	  case sttINIT			: StateTrg = sttIDLE			; break	;
	  case sttRD_SMS		: StateTrg = sttDEL_SMS			; break	;// удалить 1 СМС после прочтения
	  case sttDEL_SMS		: StateTrg = sttIDLE			; break	;// перейти к опросу СМС
	  case sttDEL_ALL_SMS	: StateTrg = sttIDLE			; break	;
//	  case sttATH			: StateTrg = sttInfSMS			; break	;
	  case sttInfSMS		: StateTrg = sttIDLE			; break	;
	  case sttREQ_CNT_SMS 	: StateTrg = sttIDLE			; break	;//
	  case sttIDLE		 	: StateTrg = sttREQ_CNT_SMS		; break	;// повторить этот запрос
	  default      			: StateTrg = sttIDLE			;
   }
 }
 else{
   switch((int)StateTrg){
//     case sttPwrON 			: result = Operate_PwrON(Msg)		; break	;
     case sttINIT 			: result = Operate_INIT(Msg)		; break	;
     case sttREQ_CNT_SMS	: result = Operate_REQ_CNT_SMS(Msg)	; break	;
     case sttRD_SMS			: result = Operate_RD_SMS(Msg)		; break	;
     case sttDEL_SMS		: result = Operate_DEL_SMS(Msg)		; break	;
     case sttDEL_ALL_SMS	: result = Operate_DEL_ALL_SMS(Msg)	; break	;
	 case sttInfSMS			: result = Operate_InfSMS(Msg)		; break	;
	 case sttATH		 	: result = Operate_ATH(Msg)			; break	;
	 case sttIDLE		 	: result = Operate_IDLE(Msg)		; break	;
	 default	   			: result = -1						;
   }
   if(SttPhase > 0 && result > 0) SttPhase++	;
 }
 
 if(result <= 0) result = TIM_WAIT_500	;
 
 return result	;}
//***************************************************************
int		TUsartGSM::Operate_IDLE(int Msg)
{int	result = 500		;

 switch(SttPhase){
   case	1	: result = TIM_IDLE			; break	;
   case 2	: State = StateTrg			; break	;
   default	: SttPhase = 0	; result = -1		;
 }
 
 return result				;}
//***************************************************************
//int		TUsartGSM::Operate_PwrON(int Msg)
//{int	result = TIM_WAIT_PWR_ON	;

// switch(SttPhase){
//   case	1:	VEN_PIN_SET()					; break	;// Vbat ON
//   case 2:  PWR_ON_PIN_SET()				; break	;// PWRKEY set LOW
//   case 3:  PWR_ON_PIN_RST()				; break	;// PWRKEY set HIGH
//   case 4:	State = StateTrg				; break	;
//   default: SttPhase = 0	; result = -1	;
// }
// return result	;}
//***************************************************************
int		TUsartGSM::Operate_INIT(int Msg)
{int	result = 500;

 switch(SttPhase){
   case	1 : ResetFiFo()		; WriteStringLN(strAT)			; break	;
   case 2 : 				  WriteStringLN(strINIT1)		; break	;
   case	3 : if(Msg == msgOK){ State  = StateTrg				;
							  strMsg = strMsg_INIT_OK		;
							  flMdmPresent = 1				;}
			else			 {strMsg = strMsg_INIT_ERR		;
							  flMdmPresent = 0				;}
			break	;
   default: SttPhase = 0	; result = -1	;
 }
 return result	;}
//***************************************************************
// Послать запрос количества СМС в памяти
int		TUsartGSM::Operate_REQ_CNT_SMS(int Msg)
{int	result = 1000		;
 static	int	needReadSMS = 0	;

 switch(SttPhase){
   case	1 : needReadSMS = 0	; WriteStringLN(strREQ_CNT_SMS)	; break	;
   case	2 : if(Msg == msgMemSMS) needReadSMS = 1			; break	;// ждем ответа "CPMS:" или таймаут
   case	3 : if(needReadSMS && Msg == msgOK){ 						 // ждем "ОК" или таймаут
			  StateTrg = sttRD_SMS	; SttPhase = 1	; result = -1	;}
			else State = StateTrg	; 
			break	;
   default: SttPhase = 0	; result = -1			;
 }
 return result	;}
//***************************************************************
int		TUsartGSM::Operate_ATH(int Msg)
{int	result = 1000		;

 switch(SttPhase){
   case	1 : WriteStringLN("ATH"); result = 2000		; break	;// долго!!!
   case	2 : State = StateTrg	; result = 100		; break	;// ждем "ОК" или таймаут
   default: SttPhase = 0		; result = -1		;
 }
 return result	;}
//***************************************************************
int		TUsartGSM::Operate_InfSMS(int Msg)
{int	result = 500	;
 char	*str = 0		;

 switch(SttPhase){
   case	1 : if(*PhoneNmbrOut && NeedSendSMS && !timGuardSMS){ 
			  Msg = msgOK	; WriteStringLN_P(strSEND_SMS,PhoneNmbrOut)	;}
   break	;
   
   case	2 : if(Msg == msgPROMPT){
				str = FnGetInfSMS ? FnGetInfSMS(SmsOutBuf,LenBF):
									(char*)strNO_INFO			;
				WriteStringLN(str)	; NeedSendSMS = 0		; 
				*PhoneNmbrOut =0	; timGuardSMS = TIM_GUARD_SMS	;}
   break	;
   
   case 3 : if(Msg == msgOK)   { strMsg = strMsg_SMS_SEND_OK	;}// TODO TODO
	   else if(Msg == msgERROR){ strMsg = strMsg_SMS_SEND_ERR	;}
	   State = StateTrg	; NeedSendSMS = 0	; *PhoneNmbrOut =0	;
   break	;
   
   default: SttPhase = 0	; result = -1	;			
 }
 return result	;}
//***************************************************************
int		TUsartGSM::Operate_RD_SMS(int Msg)
{int	result = 500;
 char	str[20]		;

 switch(SttPhase){
   case	1 :	if(!FIxRdSMS) FIxRdSMS = 1					;
			sprintf(str,strRD_SMS,FIxRdSMS)				;
			FIxDelSMS = FIxRdSMS	; FIxRdSMS = 0		;
			WriteStringLN(str)		; result = 2000		; break	;
   case	2 : if(Msg == msgOK) strMsg = strMsg_RD_SMS_OK	; 
								      result = 100		; break	;
   case 3 :	State = StateTrg							; break	;
   default: SttPhase = 0			; result = -1		;
 }
 return result	;}
//***************************************************************
int		TUsartGSM::Operate_DEL_SMS(int Msg)
{int	result = 500;
 char	str[20]		;

 switch(SttPhase){
   case	1 : sprintf(str,strDEL_SMS,FIxDelSMS)			; 
			FIxDelSMS = 0	; WriteStringLN(str)		; break	;
   case	2 : if(Msg == msgOK)    strMsg = strMsg_DEL_SMS_OK;
	   else if(Msg == msgERROR) strMsg = strMsg_DEL_SMS_ERR;
	   State = StateTrg									; break	;   
   default: SttPhase = 0	; result = -1				;
 }
 return result	;}
//***************************************************************
int		TUsartGSM::Operate_DEL_SMS1(int Msg)
{int	result = 500;

 switch(SttPhase){
   case	1 : WriteStringLN(strDEL_SMS1)					; break	;
   case	2 : if(Msg == msgOK) strMsg = strMsg_DEL_SMS_OK	;
	   else if(Msg == msgERROR){}
	   State = StateTrg									; break	;   
   default: SttPhase = 0	; result = -1				;
 }
 return result	;}
//***************************************************************
int		TUsartGSM::Operate_DEL_ALL_SMS(int Msg)
{int	result = 500;

 switch(SttPhase){
   case	1 : WriteStringLN(strDEL_ALL_SMS)				; break	;
   case	2 : if(Msg == msgOK) strMsg = strMsg_DEL_ALL_SMS_OK	;
	   else if(Msg == msgERROR){}
	   State = StateTrg									; break	;   
   default: SttPhase = 0	; result = -1				;
 }
 return result	;}
//***************************************************************


//***************************************************************
void	TUsartGSM::SetMasterNmbr(char* src)
{strcpy(StrMasterNmbr,src)						;
 /*strncpy(StoreRec.MasterNmbr,src,15)			;
 StoreFlash.StoreRec(&StoreRec)					;*/}
//***************************************************************
char*	TUsartGSM::GetMasterNmbr(void)					
{return StrMasterNmbr	;/*StoreRec.MasterNmbr			;*/}
//***************************************************************
uint16_t	TUsartGSM::ParseTextSMS(char* str)
{uint16_t		msgMsg = msgEmpty		;
 const int		cntPrm = 6, lenPrm = 16	;
 char			Prm[cntPrm][lenPrm]		;
 char*			pPrm = &Prm[0][0]		;
 int			Val[cntPrm]				;
 char			Dlm[] = ",= #*"			;

 flEventNeed = NeedSendSMS = 0			;
 strncpy(SmsInBuf,str,LenBF-1)			;
 strMsg = SmsInBuf						;
 
 ParseParams(str,Dlm,pPrm,Val,cntPrm,lenPrm)						;
 if(Val[0] == PswGSM){
//   if(!StrCmp(Prm[1],cmdNewPass)){ PswGSM = Val[2]					; 
//	 if(FnSetPswGSM ) FnSetPswGSM(PswGSM)							;
//	 strMsg = StrDbg	; sprintf(StrDbg,"new Psw = %d", PswGSM)	;
//								   flEventNeed = evGetEvent			;}
//   if(!StrCmp(Prm[1],cmdStart  ))  flEventNeed = ebBoilStartP		;
//   if(!StrCmp(Prm[1],cmdStop   ))  flEventNeed = ebBoilStop			;
//   if(!StrCmp(Prm[1],cmdTermTrg)){ flEventNeed = evSetTermo			; 
//								   flValueNeed = Val[2]				;}
   if(!StrCmp(Prm[1],cmdMaster)){   SetMasterNmbr(PhoneNmbrSMS)		;
								   flEventNeed = evGetEvent			;}
   
   if(flEventNeed && *PhoneNmbrSMS){ NeedSendSMS = 1 ; timGuardSMS = 0	;}// отправить ответную СМС
   sprintf(StrDbg," true Psw %d",Val[0])		; strMsg = StrDbg	;
 }
 else{sprintf(StrDbg," wrong Psw %d",Val[0])	; strMsg = StrDbg	;
   NeedSendSMS = 0 ; *PhoneNmbrSMS = 0			;
 }

 msgMsg = msgSMS_PARSED		;
 timTxPause = TIM_TX_PAUSE	;// защитная пауза TX 
 return	msgMsg	;}
//***************************************************************


static	char	bufSmpl[160]		; 
//***************************************************************
uint16_t	TUsartGSM::Parse(char* str,int cnt)
{uint16_t	msgMsg = msgEmpty				;
 
 if(cnt>0 && str[0] != '\r' && str[0] != '\n'){
   if(flWaitSMS){
     strDbg = DecodeHEX_SMS(str,bufSmpl)	;   
     msgMsg = ParseTextSMS(str)				; flWaitSMS = 0		;}
   if(!strDbg) strDbg = str										;// строка не должна быть пустой
//   xputs(strDbg)	; xputs("\r")			;
 }
	  if(*str == smbPROMPT)			msgMsg = msgPROMPT			;
 else if(!StrCmp(str,strOK))		msgMsg = msgOK				;
 else if(!StrCmp(str,strERROR)) 	msgMsg = msgERROR			;
 else if(!StrCmp(str,strNO_CRR))	msgMsg = msgNO_CRR			;
 
 else if(!StrCmp(str,strAnsCPMS)) 	msgMsg = ParseCPMS(str)		;
 else if(!StrCmp(str,strAnsCMGR)) 	msgMsg = ParseCMGR(str)		;
 else if(!StrCmp(str,strAnsCLIP)) 	msgMsg = ParseCLIP(str)		;
 else if(!StrCmp(str,strAnsCMTI)) 	msgMsg = ParseCMTI(str)		;// Пришла СМС!
 
 return	msgMsg	;}
//***************************************************************
// Если пришел вызов, запомнить номер, сбросить, поставить флаг NeedSendSMS
//	RING
//	+CLIP: "+79231234567",145,"",,"",0
uint16_t	TUsartGSM::ParseCLIP(char* str)
{uint16_t		msgMsg = msgEmpty		;
 const int		cntPrm = 6, lenPrm = 16	;
 char			Prm[cntPrm][lenPrm]		;
 char*			pPrm = &Prm[0][0]		;
 int			Val[cntPrm]				;
 char			Dlm[] = ", "			;
 
 int result = ParseParams(str,Dlm,pPrm,Val,cntPrm,lenPrm)	;
 
 strncpy(PhoneNmbrCall,Prm[1],20)		;
 if(!StrCmp(PhoneNmbrCall,strValidNmbr)){ NeedSendSMS = 1	;}

 timTxPause = TIM_TX_PAUSE				;// защитная пауза TX 
 return	msgMsg	;}
//***************************************************************
// разберем ответ на запрос "Кол-во СМС"
//	+CPMS: "SM",7,15,"SM",7,15,"SM",7,15
//	OK
uint16_t	TUsartGSM::ParseCPMS(char* str)
{uint16_t		msgMsg = msgEmpty		;
 const int		cntPrm = 4				;
 int			Val[cntPrm]				;
 char			Dlm[] = ", "			;
 
 int result = ParseParams(str,Dlm,0,Val,cntPrm,0)	;
 
 FCntMemSMS = Val[2]					;
 FTtlMemSMS = Val[3]					;
 if(FCntMemSMS > 0 && FTtlMemSMS > 0) FIxMemSMS = 1	;// В памяти есть СМС
 else FIxMemSMS = FCntMemSMS = FTtlMemSMS = 0		;
 
 timTxPause = TIM_TX_PAUSE				;// защитная пауза TX 
 return	msgMsg		;}
//***************************************************************
//	+CMTI: "SM",1
uint16_t	TUsartGSM::ParseCMTI(char* str)		// Входящая СМС!
{uint16_t		msgMsg = msgEmpty		;
 const int		cntPrm = 3				;
 int			Val[cntPrm]				;
 char			Dlm[] = ",:"			;
 
 int result = ParseParams(str,Dlm,0,Val,cntPrm,0)	;
 if(Val[2]>0){ FIxInSMS = Val[2]		;}// Будем Читать СМС 
 
 timTxPause = TIM_TX_PAUSE				;// защитная пауза TX 
 return	msgMsg	;}
//***************************************************************
// разберем ответ на запрос "Дай СМС"
// sample: 
//	+CMGR: "REC UNREAD","+79231234567","","14/03/21,19:16:04+28"
//	45,stop
//	OK
uint16_t	TUsartGSM::ParseCMGR(char* str)
{uint16_t		msgMsg = msgEmpty		;
 const int		cntPrm = 6, lenPrm = 16	;
 char			Prm[cntPrm][lenPrm]		;
 char*			pPrm = &Prm[0][0]		;
 int			Val[cntPrm]				;
 char			Dlm[] = ","				;
 int			isUnread = 0			;
 
 int result = ParseParams(str,Dlm,pPrm,Val,cntPrm,lenPrm)	;
 char*	str2 = strchr(Prm[0],'\"')							;
 if(str2 && !StrCmp(str2+1,"REC UNR")) isUnread = 1			;
 
 strncpy(PhoneNmbrSMS,Prm[1],16)							;// Номер отправителя СМС! запомним
 if(isUnread && !StrCmp(PhoneNmbrSMS,strValidNmbr))			 // Если номер начинается на "+79" то будем распознавать SMS!
   flWaitSMS = 1											;// СМС начнется со следующей принятой строки
 else{ *PhoneNmbrSMS = 0				;}
 
 timTxPause = TIM_TX_PAUSE*5			;// большаая защитная пауза TX 
 return	msgMsg	;}
//***************************************************************

//***************************************************************
// >> 0020 041F 0440 0438 043C 0435 0440 0447 0438 043A
// ++        CF   F0   E8   EC   E5   F0   F7   E8   EA
// >> 00340035002C043F04400438043C0435044004470438043A0020043D043E043C043504400020043404320430003D00340038
//static	char	Smpl[]="00340035002C043F04400438043C0435044004470438043A0020043D043E043C043504400020043404320430003D00340038";
char*	TUsartGSM::DecodeHEX_SMS(char* str,char* buf)
{u16			hVal						;
 u8*			uVal = (u8*)&hVal			;
 char			*end , start[8] = "0000"	;
 int			len = (str && *str) ? strlen(str) : 0	;
 
 if(!len || (len % 4)) str = buf = 0		;// длина строки должна быть кратна 4
 
 start[4] = 0			;
 for(int ix=0;str && str[ix*4] && buf;ix++){
   strncpy(start,str+ix*4,4)				;
   hVal = strtoul(start,&end,16)			;
   if(end-start==4) buf[ix] = uVal[0] + ((uVal[1] != 4) ? 0:0xB0)	;
   else buf = 0	;   
 }

 return buf	;}
//***************************************************************

//***************************************************************
void	TUsartGSM::FnMdmInit(void)		// callback для CDC Mdm
{

}
//***************************************************************
void	TUsartGSM::Init(void)
{const	char	EndS[] = "\n>"			;// символы "конец строки"
 Instance = this	;
 SetFifoRx(GsmBufRx,LenBF,0,EndS)		;
 SetFifoTx(GsmBufTx,LenBF)				; 
// FnGetInfSMS = 0	; FnGetPswGSM = 0	; FnSetPswGSM = 0	;
// TUsart::InitHW(USART_GSM,9600)			;
 flEventNeed = 0	;
 
 FnWriteBuff = USBH_CDC_WriteBuff		;
 cbUSBH_CDC_ListenData = FnListenData	;
 cbUSBH_CDC_MDM_Init   = FnMdmInit		;

 
 InitHW()			;
}
//***************************************************************
void	TUsartGSM::InitGSM(void)
{flMdmPresent = 0			; strMsg = 0	;
 FCntMemSMS = FTtlMemSMS = FLenSMS = FReadAll = 0			;
 State = StateTrg = sttNone	; SttPhase = 0	;
 *PhoneNmbrSMS = *PhoneNmbrCall = *PhoneNmbrOut = 0			;
 timGuardSMS = 0			;
		 
// StoreFlash.Init(BANK_STORE_GSM,PAGE_CNT_GSM,0)	;
// StoreFlash.RestoreRec(&StoreRec)				;

// PswGSM = FnGetPswGSM ? FnGetPswGSM():0		;
// sprintf(StrDbg,"PswGSM = %d,%s",PswGSM,GetMasterNmbr())	; strMsg = StrDbg	;
 FIxRdSMS=FIxDelSMS=FIxMemSMS=FIxInSMS=0	;
 flWaitSMS = 0								; 
// strDbg = DecodeHEX_SMS(Smpl,bufSmpl)		;
}
//***************************************************************
void	TUsartGSM::InitHW(void)
{//GPIO_InitTypeDef		GPIO_InitStructure				;

// GPIO_Enable(VEN_PORT)	;
// GPIO_Enable(STT_PORT)	;
// 
// VEN_PIN_RST()			; PWR_ON_PIN_RST()				;
// 
// GPIO_InitStructure.GPIO_Pin   = VEN_PIN | PWR_ON_PIN	;
// GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP		;
// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz		;
// GPIO_Init(VEN_PORT, &GPIO_InitStructure)				;
// 
// GPIO_InitStructure.GPIO_Pin   = STT_PIN | NTL_PIN		;
// GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING	;
// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz		;
// GPIO_Init(STT_PORT, &GPIO_InitStructure)				; 
}
//***************************************************************
char*	TUsartGSM::GetS(char* buf,int maxLen)
{
 if(buf && maxLen>0 && FifoRx.CntStr>0){ 
   DisableRxIRQ()					;
   buf = FifoRx.GetS(buf,maxLen)	;
   EnableRxIRQ()					;
 }
 else buf = 0						;
 
 return buf							;}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void	TUsartGSM::WriteString(char* Str)
{
 FifoTx.Reset()		;

 for(int ix=0;Str && Str[ix];ix++) FifoTx.In(Str[ix])	;

 if(FnWriteBuff) FnWriteBuff(FifoTx.GetBuf(),FifoTx.GetLen())	;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void	TUsartGSM::WriteStringLN(const char* Str)
{
 FifoTx.Reset()		;

 for(int ix=0;Str && Str[ix];ix++) FifoTx.In(Str[ix])	;
 FifoTx.In('\r')	; FifoTx.In('\n')	;

 if(FnWriteBuff) FnWriteBuff(FifoTx.GetBuf(),FifoTx.GetLen())	;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void	TUsartGSM::WriteStringLN_P(const char* Str,const char* Prm)
{
 FifoTx.Reset()		;
 
 for(int ix=0;Str && Str[ix];ix++) FifoTx.In(Str[ix])	;
 for(int ix=0;Prm && Prm[ix];ix++) FifoTx.In(Prm[ix])	;
 FifoTx.In('\r')	; FifoTx.In('\n');
 
 if(FnWriteBuff) FnWriteBuff(FifoTx.GetBuf(),FifoTx.GetLen())	;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//int TUsartGSM::SendChar(int ch)
//{
// DisableTxIRQ()						;
// if(!FifoTx.Full()) FifoTx.In(ch)	;
// EnableTxIRQ()						;
// return ch							;
//}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int		TUsartGSM::FnListenData(void* Buf,int Len)
{

 return 0	;}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void	TUsartGSM::EnableRxIRQ (void){}
void	TUsartGSM::DisableRxIRQ(void){}
void	TUsartGSM::EnableTxIRQ (void){}
void	TUsartGSM::DisableTxIRQ(void){}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Эта подпрограмма выполняется в контексте прерывания !!!!!
void	TUsartGSM::FOnTimer(void)
{
 if(timOperate >0) timOperate--		;
 if(timOut     >0) timOut--			;
 if(timRepeat  >0) timRepeat--		;
 if(timGuardSMS>0) timGuardSMS--	;
 if(timTxPause >0) timTxPause--		;// защитная пауза, чтоб UART GSM модема успел переключится на прием 
}
//***************************************************************
void	TUsartGSM::OnTimer(void){if(Instance) Instance->FOnTimer()	;}
//***************************************************************
//***************************************************************
int	ParseParams(char* str,char* dlm,char* Prm,int* Val,const int CntPrm,const int LenPrm)
{int		result = 0						;
 int		ix=0,ixPrm=0,ixP=0				;
 int		isDlm=0,endStr=0				;
 const int	lenBufPrm = 20					;
 char		bufPrm[lenBufPrm]				;
 char*		cPrm     = bufPrm				;
 int		lenPrm   = lenBufPrm			;

 if(str && *str && dlm && *dlm && CntPrm){
   cPrm   = (Prm  && LenPrm) ? Prm + ixPrm*LenPrm : bufPrm	;
   lenPrm = (Prm  && LenPrm) ? LenPrm     : lenBufPrm		;
   ixP = 0	;
 
   for(ix=0;ixPrm < CntPrm && !endStr && cPrm;ix++){
     isDlm = 0	;
	 for(int ixD=0;dlm[ixD];ixD++){							 // текущий символ - разделитель?
       if(str[ix] == dlm[ixD]){ isDlm = 1	; break	;}}
     if(str[ix] == '\r' || str[ix] == '\n' || !str[ix]) endStr = 1	;// конец строки ?
	 
	 if(!isDlm && !endStr){											// если не разделитель и не конец строки
	   if(ixP < lenPrm-1) cPrm[ixP++] = str[ix]		;}
	 else{
	   cPrm[ixP] = 0		; ixP = 0	;							// иначе
       if(Val) Val[ixPrm] = atoi(cPrm)	;
	   result = (++ixPrm)				;
	   if(ixPrm >= CntPrm) endStr = 1	;
	   if(!endStr){
         cPrm   = (Prm && LenPrm) ? Prm + ixPrm*LenPrm : bufPrm		;
         lenPrm = (Prm && LenPrm) ? LenPrm     : lenBufPrm			;
	   }
	 }
   }
 }
 return result		;}
//***************************************************************
#pragma diag_suppress 177
