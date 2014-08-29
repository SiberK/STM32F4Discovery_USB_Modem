/**
  ******************************************************************************
  * @file    usbh_msc_core.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   This file implements the MSC class driver functions
  *          ===================================================================      
  *                                MSC Class  Description
  *          =================================================================== 
  *           This module manages the MSC class V1.0 following the "Universal 
  *           Serial Bus Mass Storage Class (MSC) Bulk-Only Transport (BOT) Version 1.0
  *           Sep. 31, 1999".
  *           This driver implements the following aspects of the specification:
  *             - Bulk-Only Transport protocol
  *             - Subclass : SCSI transparent command set (ref. SCSI Primary Commands - 3 (SPC-3))
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include	<stdio.h>
#include	<string.h>
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"
#include "usbh_ioreq.h"
#include "usbh_core.h"
#include	"Log.h"
//------------------------------------------------------------------------
#define USBH_MSC_ERROR_RETRY_LIMIT 10
//------------------------------------------------------------------------
USB_Setup_TypeDef     	MSC_Setup				;
uint8_t 				MSCErrorCount = 0		;
USBH_BOTXfer_TypeDef	USBH_CDC_BOTXferParam	;
MSC_Machine_TypeDef    	MSC_Machine				;
//------------------------------------------------------------------------
static 	USBH_Status		USBH_MSC_InterfaceInit  (USB_OTG_CORE_HANDLE *pdev,void *phost);
static 	void 			USBH_MSC_InterfaceDeInit(USB_OTG_CORE_HANDLE *pdev,void *phost);
static 	USBH_Status	 	USBH_MSC_Handle			(USB_OTG_CORE_HANDLE *pdev,void *phost);
static 	USBH_Status 	USBH_MSC_ClassRequest	(USB_OTG_CORE_HANDLE *pdev,void *phost);
static 	USBH_Status	 	USBH_MSC_BOTReset		(USB_OTG_CORE_HANDLE *pdev,USBH_HOST *phost);
static 	USBH_Status	 	USBH_MSC_GETMaxLUN		(USB_OTG_CORE_HANDLE *pdev,USBH_HOST *phost);
		void 			USBH_MSC_ErrorHandle	(uint8_t status);
							   
		USBH_Status		MY_ModeSwitch			(USB_OTG_CORE_HANDLE *pdev,void *phost);
		USBH_Status		USBH_MY_InterfaceInit	(USB_OTG_CORE_HANDLE *pdev,void *phost,uint8_t InterfaceClass,uint8_t InterfaceProtocol,short Intf);
		USBH_Status		USBH_CDC_WriteBuff		(void* Data,int Len)		;
		int				(*cbUSBH_CDC_ListenData)(void* Data,int Len) = 0	;
		void			(*cbUSBH_CDC_MDM_Init)	(void)				 = 0	;
//------------------------------------------------------------------------
USBH_Class_cb_TypeDef  USBH_MSC_cb = 
{
  USBH_MSC_InterfaceInit,
  USBH_MSC_InterfaceDeInit,
  USBH_MSC_ClassRequest,
  USBH_MSC_Handle,
};
//------------------------------------------------------------------------
#define		CDC_CLASS1		0xFF
#define		CDC_CLASS2		0x00
#define		CDC_PROTOCOL1	0x61
#define		CDC_PROTOCOL2	0x63
#define		CDC_PROTOCOL3	0x62
//------------------------------------------------------------------------
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
char		strMsg_HUAWEI_E171[] = "55534243123456780000000000000011062000000100000000000000000000";// MAGIC!!!!
char		strMsg_ZTE_MF112[]   = "5553424392020000000000000000061B000000020000000000000000000000";
//char		strMsg_ZTE_MF112[]   = "5553424312345678000000000000061e000000000000000000000000000000";
uint8_t		buffMsg[100]			;

typedef struct{
 uint16_t		Vid,Pid	;
 char*			strModeSwitch	;
 uint8_t		Class,Proto		; 
 short			Intf			;// номер интерфейса
}TDeviceUsbMdm;

TDeviceUsbMdm		TableUsbMdm[]={
   {0x12D1,0x155B,strMsg_HUAWEI_E171,0xFF,0x62,-1}
  ,{0x19D2,0x2000,strMsg_ZTE_MF112  ,0xFF,0xFF, 1}
};

static	TDeviceUsbMdm*		DeviceUsbMdm = 0	;
#define	SIZE_TBL_USB_MDM	(sizeof(TableUsbMdm)/sizeof(*TableUsbMdm))
//------------------------------------------------
/**
  * @brief  USBH_MSC_InterfaceInit 
  *         Interface initialization for MSC class.
  * @param  pdev: Selected device
  * @param  hdev: Selected device property
  * @retval USBH_Status : Status of class request handled.
  */
static USBH_Status USBH_MSC_InterfaceInit(USB_OTG_CORE_HANDLE *pdev,void *phost)
{	 
  USBH_HOST *pphost = phost;
  USBH_Status	Status = USBH_FAIL	;
  
  if(Status != USBH_OK && DeviceUsbMdm){
    Status = USBH_MY_InterfaceInit(pdev,phost,DeviceUsbMdm->Class,DeviceUsbMdm->Proto,DeviceUsbMdm->Intf)	;
	if(Status == USBH_OK) MSC_Machine.isCDC = 1							;}
	
  if(Status != USBH_OK){
    Status = USBH_MY_InterfaceInit(pdev,phost,MSC_CLASS,MSC_PROTOCOL,-1)	;
	if(Status == USBH_OK) MSC_Machine.isCDC = 0							;}
  
  if(Status != USBH_OK){ pphost->usr_cb->DeviceNotSupported()			;}
  return USBH_OK ;
}
//------------------------------------------------------------------------------------
USBH_Status	USBH_MY_InterfaceInit(USB_OTG_CORE_HANDLE *pdev,void *phost,uint8_t InterfaceClass,uint8_t InterfaceProtocol,short Intf)
{
  USBH_Status	Status = USBH_FAIL			;
  USBH_HOST 	*pphost = phost				;
  int			if_ix = 1,ep_ix=0,CntEp		;
  
  USBH_InterfaceDesc_TypeDef        *Itf_Desc	;
  USBH_EpDesc_TypeDef               *Ep_Desc,*ep_in=0,*ep_out=0		;
  
  for(if_ix=0;if_ix<pphost->device_prop.Cfg_Desc.bNumInterfaces;if_ix++){
    Itf_Desc = &pphost->device_prop.Itf_Desc[if_ix]				;
    if(Itf_Desc->bInterfaceClass    != InterfaceClass || 
	   Itf_Desc->bInterfaceProtocol != InterfaceProtocol ||
	   (Intf>=0 && Intf != if_ix)) continue	;
	
	ep_in = ep_out = 0				;
	CntEp = Itf_Desc->bNumEndpoints	;
	for(ep_ix=0;ep_ix<CntEp;ep_ix++){
	  Ep_Desc = &pphost->device_prop.Ep_Desc[if_ix][ep_ix]		;
	  if((Ep_Desc->bmAttributes & 3) != EP_TYPE_BULK) continue	;// need BULK
	  if(Ep_Desc->bEndpointAddress & 0x80) ep_in = Ep_Desc		; else ep_out = Ep_Desc	;	  
	  if(ep_in && ep_out) break	;
	}	
	if(ep_in && ep_out) break	;	  
  }
  
  if(ep_in && ep_out){
    MSC_Machine.isCDC = 1	;
    MSC_Machine.MSBulkInEp      = ep_in ->bEndpointAddress		;
    MSC_Machine.MSBulkInEpSize  = ep_in ->wMaxPacketSize		;
    MSC_Machine.MSBulkOutEp     = ep_out->bEndpointAddress		;
    MSC_Machine.MSBulkOutEpSize = ep_out->wMaxPacketSize		;

    MSC_Machine.hc_num_out = USBH_Alloc_Channel(pdev,MSC_Machine.MSBulkOutEp);
    MSC_Machine.hc_num_in  = USBH_Alloc_Channel(pdev,MSC_Machine.MSBulkInEp);  
    
    /* Open the new channels */
    USBH_Open_Channel  (pdev,
                        MSC_Machine.hc_num_out,
                        pphost->device_prop.address,
                        pphost->device_prop.speed,
                        EP_TYPE_BULK,
                        MSC_Machine.MSBulkOutEpSize)	;
    
    USBH_Open_Channel  (pdev,
                        MSC_Machine.hc_num_in,
                        pphost->device_prop.address,
                        pphost->device_prop.speed,
                        EP_TYPE_BULK,
                        MSC_Machine.MSBulkInEpSize)		;
    Status = USBH_OK	;
	
	Log.d("InterfaceInit\n   EpIn=0x%02X, EpOut=0x%02X\n",ep_in->bEndpointAddress,ep_out->bEndpointAddress);
  }  
  return Status ; 
}
//------------------------------------------------------------------------------------


/**
  * @brief  USBH_MSC_InterfaceDeInit 
  *         De-Initialize interface by freeing host channels allocated to interface
  * @param  pdev: Selected device
  * @param  hdev: Selected device property
  * @retval None
  */
void USBH_MSC_InterfaceDeInit ( USB_OTG_CORE_HANDLE *pdev,
                                void *phost)
{	
  if ( MSC_Machine.hc_num_out)
  {
    USB_OTG_HC_Halt(pdev, MSC_Machine.hc_num_out);
    USBH_Free_Channel  (pdev, MSC_Machine.hc_num_out);
    MSC_Machine.hc_num_out = 0;     /* Reset the Channel as Free */
  }
   
  if ( MSC_Machine.hc_num_in)
  {
    USB_OTG_HC_Halt(pdev, MSC_Machine.hc_num_in);
    USBH_Free_Channel  (pdev, MSC_Machine.hc_num_in);
    MSC_Machine.hc_num_in = 0;     /* Reset the Channel as Free */
  } 
}

/**
  * @brief  USBH_MSC_ClassRequest 
  *         This function will only initialize the MSC state machine
  * @param  pdev: Selected device
  * @param  hdev: Selected device property
  * @retval USBH_Status : Status of class request handled.
  */

static USBH_Status USBH_MSC_ClassRequest(USB_OTG_CORE_HANDLE *pdev ,void *phost)
{   
  USBH_Status status = USBH_OK ;
  USBH_MSC_BOTXferParam.MSCState = USBH_MSC_BOT_INIT_STATE	;
  USBH_CDC_BOTXferParam.MSCState = USBH_CDC_INIT			;
  
  return status; 
}

//-------------------------------------------------------------------------------
static	char	InBuff[200]	;
//static	char	OutBuff[] = "ATi\r\n"	;
//-------------------------------------------------------------------------------
static USBH_Status 	USBH_CDC_Handle(USB_OTG_CORE_HANDLE *pdev ,void   *phost)
{
//  USBH_HOST *pphost = phost;
    
  USBH_Status 		status      = USBH_BUSY;
//  uint8_t 			cdcStatus   = USBH_CDC_BUSY;
//  uint8_t 			appliStatus = 0;
//  uint8_t 			xferDirection, index;
  static uint32_t 	datalen,remainingDataLength;
  static uint8_t 	*datapointer;// , *datapointer_prev;
  URB_STATE 		URB_State	;
    
  if(HCD_IsDeviceConnected(pdev))
  {   
    switch(USBH_CDC_BOTXferParam.MSCState){
	
	case	USBH_CDC_INIT:
		USBH_CDC_BOTXferParam.MSCStateBkp = USBH_CDC_BOTXferParam.MSCState	;
		USBH_CDC_BOTXferParam.MSCState    = USBH_CDC_GET_DATA				;// Слушаем IN endpoint
		datapointer = 0	; remainingDataLength = 0							;
		if(cbUSBH_CDC_MDM_Init) cbUSBH_CDC_MDM_Init()						;
	break	;

	case	USBH_CDC_SEND_DATA:
        status = USBH_OK;
		URB_State = HCD_GetURB_State(pdev , MSC_Machine.hc_num_out)		;
		
		if(URB_State == URB_DONE){
		  if(USBH_CDC_BOTXferParam.MSCStateBkp != USBH_CDC_BOTXferParam.MSCState){
		    USBH_CDC_BOTXferParam.MSCStateBkp = USBH_CDC_BOTXferParam.MSCState	;
            datapointer = (uint8_t*)USBH_CDC_BOTXferParam.pRxTxBuff				;
		    remainingDataLength =   USBH_CDC_BOTXferParam.DataLength			;}
			
		  datalen = MIN(remainingDataLength,MSC_Machine.MSBulkInEpSize)			;
		  status = USBH_BulkSendData(pdev,datapointer,datalen, MSC_Machine.hc_num_out);
		  if(status == USBH_OK){
		    remainingDataLength -= datalen	;
			if(remainingDataLength > 0) datapointer += datalen			;// если передали еще не весь буфер
			else{			  		    datapointer = 0					;// иначе
		      USBH_CDC_BOTXferParam.MSCState = USBH_CDC_GET_DATA		;// Слушаем IN endpoint
			}
		  }
		}
	break	;
	
	case	USBH_CDC_GET_DATA:
        datapointer = (uint8_t*)InBuff	;
		URB_State = HCD_GetURB_State(pdev , MSC_Machine.hc_num_in)		;
		
		if(URB_State == URB_DONE){
		  datalen = HCD_GetXferCnt(pdev,MSC_Machine.hc_num_in)			;
		  if(datalen > 0){
		    InBuff[datalen] = 0	;
		    if(cbUSBH_CDC_ListenData) cbUSBH_CDC_ListenData(InBuff,datalen)	;
//		    Log.d("RECEIVE %d bytes: %s \n",datalen,InBuff)				;
		  }
		}
		
		if(URB_State == URB_DONE || USBH_CDC_BOTXferParam.MSCStateBkp != USBH_CDC_BOTXferParam.MSCState){
		  USBH_CDC_BOTXferParam.MSCStateBkp = USBH_CDC_BOTXferParam.MSCState	;
		  status = USBH_BulkReceiveData (pdev,datapointer,MSC_Machine.MSBulkInEpSize, MSC_Machine.hc_num_in);
		}
		else if(URB_State == URB_IDLE){
		  USBH_CDC_BOTXferParam.MSCState = USBH_CDC_GET_DATA			;
		}
	break	;
	default : break	;
	}
  }
  return status	;
}
//-------------------------------------------------------------------------------
/**
  * @brief  USBH_MSC_Handle 
  *         MSC state machine handler 
  * @param  pdev: Selected device
  * @param  hdev: Selected device property
  * @retval USBH_Status
  */

static USBH_Status USBH_MSC_Handle(USB_OTG_CORE_HANDLE *pdev ,void   *phost)
{
  USBH_HOST *pphost = phost;
    
  USBH_Status 		status      = USBH_BUSY;
  uint8_t 			mscStatus   = USBH_MSC_BUSY;
  uint8_t 			appliStatus = 0;
  
  static uint8_t maxLunExceed = FALSE;

//  uint8_t 			xferDirection, index;
//  static uint32_t 	datalen;//remainingDataLength,;
//  static uint8_t 	*datapointer;// , *datapointer_prev;
//  URB_STATE 		URB_State	;
    
  if(MSC_Machine.isCDC == 1)  status = USBH_CDC_Handle(pdev,phost)	;
  
  else if(HCD_IsDeviceConnected(pdev)){   
    switch(USBH_MSC_BOTXferParam.MSCState){
    case USBH_MSC_BOT_INIT_STATE:
      USBH_MSC_Init(pdev);
	  
	  MY_ModeSwitch(pdev,phost)		;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	  
      if(MSC_Machine.isCDC == 0)  USBH_MSC_BOTXferParam.MSCState = USBH_MSC_BOT_RESET	;  
	  else                        USBH_MSC_BOTXferParam.MSCState = USBH_CDC_INIT		;
      break;
	  
    case USBH_MSC_BOT_RESET:   
      /* Issue BOT RESET request */
      status = USBH_MSC_BOTReset(pdev, phost);
      if(status == USBH_OK )
      {
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_GET_MAX_LUN;
      }
      
      if(status == USBH_NOT_SUPPORTED )
      {
       /* If the Command has failed, then we need to move to Next State, after
        STALL condition is cleared by Control-Transfer */
        USBH_MSC_BOTXferParam.MSCStateBkp = USBH_MSC_GET_MAX_LUN; 

        /* a Clear Feature should be issued here */
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_CTRL_ERROR_STATE;
      }  
      break;
      
    case USBH_MSC_GET_MAX_LUN:
      /* Issue GetMaxLUN request */
      status = USBH_MSC_GETMaxLUN(pdev, phost);
      
      if(status == USBH_OK )
      {
        MSC_Machine.maxLun = *(MSC_Machine.buff) ;
        
        /* If device has more that one logical unit then it is not supported */
        if((MSC_Machine.maxLun > 0) && (maxLunExceed == FALSE))
        {
          maxLunExceed = TRUE;
          pphost->usr_cb->DeviceNotSupported();
          
          break;
        }
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_TEST_UNIT_READY;
      }
      
      if(status == USBH_NOT_SUPPORTED )
      {
               /* If the Command has failed, then we need to move to Next State, after
        STALL condition is cleared by Control-Transfer */
        USBH_MSC_BOTXferParam.MSCStateBkp = USBH_MSC_TEST_UNIT_READY; 
        
        /* a Clear Feature should be issued here */
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_CTRL_ERROR_STATE;
      }    
      break;
      
    case USBH_MSC_CTRL_ERROR_STATE:
      /* Issue Clearfeature request */
      status = USBH_ClrFeature(pdev,
                               phost,
                               0x00,
                               pphost->Control.hc_num_out);
      if(status == USBH_OK )
      {
        /* If GetMaxLun Request not support, assume Single LUN configuration */
        MSC_Machine.maxLun = 0;  
        
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_BOTXferParam.MSCStateBkp;     
      }
      break;  
      
    case USBH_MSC_TEST_UNIT_READY:
      /* Issue SCSI command TestUnitReady */ 
      mscStatus = USBH_MSC_TestUnitReady(pdev);
      
      if(mscStatus == USBH_MSC_OK )
      {
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_READ_CAPACITY10;
        MSCErrorCount = 0;
        status = USBH_OK;
      }
      else
      {
        USBH_MSC_ErrorHandle(mscStatus);
      } 
      break;
      
    case USBH_MSC_READ_CAPACITY10:
      /* Issue READ_CAPACITY10 SCSI command */
      mscStatus = USBH_MSC_ReadCapacity10(pdev);
      if(mscStatus == USBH_MSC_OK )
      {
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_MODE_SENSE6;
        MSCErrorCount = 0;
        status = USBH_OK;
      }
      else
      {
        USBH_MSC_ErrorHandle(mscStatus);
      }
      break;

    case USBH_MSC_MODE_SENSE6:
      /* Issue ModeSense6 SCSI command for detecting if device is write-protected */
      mscStatus = USBH_MSC_ModeSense6(pdev);
      if(mscStatus == USBH_MSC_OK )
      {
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_DEFAULT_APPLI_STATE;
        MSCErrorCount = 0;
        status = USBH_OK;
      }
      else
      {
        USBH_MSC_ErrorHandle(mscStatus);
      }
      break;
      
    case USBH_MSC_REQUEST_SENSE:
      /* Issue RequestSense SCSI command for retreiving error code */
      mscStatus = USBH_MSC_RequestSense(pdev);
      if(mscStatus == USBH_MSC_OK )
      {
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_BOTXferParam.MSCStateBkp;
        status = USBH_OK;
      }
      else
      {
        USBH_MSC_ErrorHandle(mscStatus);
      }  
      break;
      
    case USBH_MSC_BOT_USB_TRANSFERS:
      /* Process the BOT state machine */
      USBH_MSC_HandleBOTXfer(pdev , phost);
      break;
    
    case USBH_MSC_DEFAULT_APPLI_STATE:
      /* Process Application callback for MSC */
      appliStatus = pphost->usr_cb->UserApplication();
      if(appliStatus == 0)
      {
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_DEFAULT_APPLI_STATE;
      }
      else if (appliStatus == 1) 
      {
        /* De-init requested from application layer */
        status =  USBH_APPLY_DEINIT;
      }
      break;
      
    case USBH_MSC_UNRECOVERED_STATE:
      
      status = USBH_UNRECOVERED_ERROR;
      
      break;
      
    default:
      break; 
      
    }
  }
   return status;
}


/**
  * @brief  USBH_MSC_BOTReset
  *         This request is used to reset the mass storage device and its 
  *         associated interface. This class-specific request shall ready the 
  *         device for the next CBW from the host.
  * @param  pdev: Selected device
  * @retval USBH_Status : Status of class request handled.
  */
static USBH_Status USBH_MSC_BOTReset(USB_OTG_CORE_HANDLE *pdev,
                              USBH_HOST *phost)
{
  
  phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_CLASS | \
                              USB_REQ_RECIPIENT_INTERFACE;
  
  phost->Control.setup.b.bRequest = USB_REQ_BOT_RESET;
  phost->Control.setup.b.wValue.w = 0;
  phost->Control.setup.b.wIndex.w = 0;
  phost->Control.setup.b.wLength.w = 0;           
  
  return USBH_CtlReq(pdev, phost, 0 , 0 ); 
}


/**
  * @brief  USBH_MSC_GETMaxLUN
  *         This request is used to reset the mass storage device and its 
  *         associated interface. This class-specific request shall ready the 
  *         device for the next CBW from the host.
  * @param  pdev: Selected device
  * @retval USBH_Status : USB ctl xfer status
  */
static USBH_Status USBH_MSC_GETMaxLUN(USB_OTG_CORE_HANDLE *pdev , USBH_HOST *phost)
{
  phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_TYPE_CLASS | \
                              USB_REQ_RECIPIENT_INTERFACE;
  
  phost->Control.setup.b.bRequest = USB_REQ_GET_MAX_LUN;
  phost->Control.setup.b.wValue.w = 0;
  phost->Control.setup.b.wIndex.w = 0;
  phost->Control.setup.b.wLength.w = 1;           
  
  return USBH_CtlReq(pdev, phost, MSC_Machine.buff , 1 ); 
}

/**
  * @brief  USBH_MSC_ErrorHandle 
  *         The function is for handling errors occuring during the MSC
  *         state machine   
  * @param  status
  * @retval None
  */

void USBH_MSC_ErrorHandle(uint8_t status)
{  
    if(status == USBH_MSC_FAIL)
    { 
      MSCErrorCount++;
      if(MSCErrorCount < USBH_MSC_ERROR_RETRY_LIMIT)
      { /* Try MSC level error recovery, Issue the request Sense to get 
        Drive error reason  */
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_REQUEST_SENSE;
        USBH_MSC_BOTXferParam.CmdStateMachine = CMD_SEND_STATE;
      }
      else
      {
        /* Error trials exceeded the limit, go to unrecovered state */
        USBH_MSC_BOTXferParam.MSCState = USBH_MSC_UNRECOVERED_STATE;
      }
    } 
    else if(status == USBH_MSC_PHASE_ERROR)
    {
      /* Phase error, Go to Unrecoovered state */
      USBH_MSC_BOTXferParam.MSCState = USBH_MSC_UNRECOVERED_STATE;
    }
    else if(status == USBH_MSC_BUSY)
    {
      /*No change in state*/
    }
}

/**
  * @}
  */ 

//------------------------------------------------
uint8_t		hex2nbl(char chHex)
{uint8_t	rslt = 	chHex <  '0' ? 0 : chHex <= '9'	? chHex-'0'		: 
					chHex <  'A' ? 0 : chHex <= 'F'	? chHex-'A'+10	: 
					chHex <  'a' ? 0 : chHex <= 'a'	? chHex-'a'+10	: 0	;
 return		rslt	;}
//------------------------------------------------
int	hexstr2bin(uint8_t* buf,char* strMsg)
{int	ix = 0	;

 for(ix=0;buf && strMsg;ix+=2){
   if(!strMsg[ix] || !strMsg[ix+1]) break	;
   buf[ix>>1] = (hex2nbl(strMsg[ix])<<4) | hex2nbl(strMsg[ix+1])			;
 }
 
 if(ix) ix = (ix>>1)	;
 return	ix				;}
//------------------------------------------------


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
USBH_Status	MY_ModeSwitch(USB_OTG_CORE_HANDLE *pdev,void *phost)
{
 int 			Len = 0,Cnt=0,ix		;
 USBH_Status 	Status = USBH_OK		;
 URB_STATE		UrbState				;
 USBH_HOST 		*pphost = phost			;
 
 USBH_DevDesc_TypeDef *hs = &pphost->device_prop.Dev_Desc;
 DeviceUsbMdm = 0	;
 
 for(ix=0;ix<SIZE_TBL_USB_MDM;ix++){
   if(hs->idVendor == TableUsbMdm[ix].Vid &&
      hs->idProduct== TableUsbMdm[ix].Pid) 
	  DeviceUsbMdm = TableUsbMdm + ix	;
 }

 if(DeviceUsbMdm){
   Len = hexstr2bin(buffMsg,DeviceUsbMdm->strModeSwitch)	;// MAGIC

   Status = USBH_BulkSendData(pdev,buffMsg,Len,MSC_Machine.hc_num_out);
 
   if(Status == USBH_OK){
     Log.d("==>")	;
     for(Cnt=0;UrbState != URB_DONE;Cnt++){
       UrbState = HCD_GetURB_State(pdev ,MSC_Machine.hc_num_out);}
	 
     Log.d("ModeSwitch USBH_BulkSendData done %d\n",Cnt);
   } 
   else Log.d("ModeSwitch USBH_BulkSendData failed %d\n",Cnt);
 }
 return Status						;}
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//------------------------------------------------
USBH_Status		USBH_CDC_WriteBuff(void* Data,int Len)
{
 USBH_CDC_BOTXferParam.MSCState = USBH_CDC_SEND_DATA	;
 USBH_CDC_BOTXferParam.DataLength = Len					;
 USBH_CDC_BOTXferParam.pRxTxBuff  = Data				;

 return	USBH_OK	;}
//------------------------------------------------
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
