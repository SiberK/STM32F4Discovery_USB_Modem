//------------------------------------------------------------------------
#ifndef		USBHCORE_H
#define		USBHCORE_H

#include 	"usbh_core.h"
#include 	"usbh_usr.h"
#include 	"usbh_msc_core.h"
#include	"EventGUI.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class		TUsbhCore{
 
 static TUsbhCore*		Instance		;
 public:
			TUsbhCore(void){}
 
 
 EVENT_TYPE				OnEvent(TEvent* Event)				;
		void			FOnTimer(void)						;
 static void			OnTimer(void)						;
 
 void					Init(void)							;
};
//------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------
