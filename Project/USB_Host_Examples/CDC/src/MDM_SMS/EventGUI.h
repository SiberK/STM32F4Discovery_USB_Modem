//--------------------------------------------------------------
//		miniGUI.h
//--------------------------------------------------------------
#ifndef		EVENT_GUI_H
#define		EVENT_GUI_H
//--------------------------------------------------------------
#include <stm32f4xx.h>
//--------------------------------------------------------------
#pragma pack(push,1)
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef		uint16_t	EVENT_TYPE	;
typedef		uint16_t	MSG			;
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef	EVENT_TYPE	(*ON_EVENT)(EVENT_TYPE EvType)	;
//--------------------------------------------------------------
enum	{KeyDemoLED,
		 evGetEvent,evClearPswGSM,evEventSMS,evUseFake,evDistantion,
		 evDbgMsg1,evDbgMsg2,evStat1,evStat2,evStat3,evStat4,
		 KeyNA};
//--------------------------------------------------------------
struct	TEvent{
  EVENT_TYPE	Type		;
  union{
	MSG			Msg			;
	uint16_t	Len			;
	short		Value		;
	bool		Checked		;
  };
  union{
	int			lParam[2]	;
	uint32_t	u32Value[2]	;
	short		shData[4]	;
	void*		Data		;
	char*		strData[2]	;
  };

  			TEvent(EVENT_TYPE _type=0,MSG _msg=0):Type(_type),Msg(_msg){}
			TEvent(EVENT_TYPE _type,uint16_t _len,void* _data):Type(_type),Len(_len),Data(_data){}
};
//--------------------------------------------------------------

//--------------------------------------------------------------
#define	SIZE_ARRAY(AR)	(sizeof(AR)/sizeof(*AR))
//--------------------------------------------------------------
extern	volatile int	flBackupStored	;
//--------------------------------------------------------------
//#ifdef __cplusplus
// }
//#endif
#pragma pack(pop)
//=========================================================
#endif
