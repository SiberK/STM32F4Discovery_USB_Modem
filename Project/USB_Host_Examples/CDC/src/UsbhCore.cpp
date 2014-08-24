//------------------------------------------------------------------------
#include	"UsbhCore.h"
//------------------------------------------------------------------------
USB_OTG_CORE_HANDLE    USB_OTG_Core	;
USBH_HOST              USB_Host		;
//------------------------------------------------------------------------
TUsbhCore*		TUsbhCore::Instance = 0		;
//------------------------------------------------------------------------
void	TUsbhCore::Init(void)
{Instance = this	;
 USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_cb)	; 
}
//------------------------------------------------------------------------
EVENT_TYPE		TUsbhCore::OnEvent(TEvent* Event)
{
 switch(Event->Type){
   default : USBH_Process(&USB_OTG_Core, &USB_Host)	;
 }
 return	Event->Type	;}
//------------------------------------------------------------------------
void			TUsbhCore::FOnTimer(void)
{
}
//------------------------------------------------------------------------
void			TUsbhCore::OnTimer(void)
{if(Instance) Instance->FOnTimer()	;}
//------------------------------------------------------------------------

//------------------------------------------------------------------------
