/**
  ******************************************************************************
  * @file    usbh_usr.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   This file includes the usb host library user callbacks
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
#include <string.h>
#include <stdio.h>
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_debug.h"
#include "usbh_usr.h"
#include "ff.h"       /* FATFS */
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"
#include	"Log.h"

/** @addtogroup USBH_USER
* @{
*/

/** @addtogroup USBH_MSC_DEMO_USER_CALLBACKS
* @{
*/

/** @defgroup USBH_USR 
* @brief    This file includes the usb host stack user callbacks
* @{
*/ 

/** @defgroup USBH_USR_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Defines
* @{
*/ 
#define IMAGE_BUFFER_SIZE    512
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Macros
* @{
*/ 
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Variables
* @{
*/ 
uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;
uint8_t filenameString[15]  = {0};

FATFS fatfs;
FIL file;
uint8_t Image_Buf[IMAGE_BUFFER_SIZE];
uint8_t line_idx = 0;   
extern	Led_TypeDef	LEDind			;


/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

USBH_Usr_cb_TypeDef USR_cb =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  USBH_USR_MSC_Application,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
    
};

/**
* @}
*/

/** @defgroup USBH_USR_Private_Constants
* @{
*/ 
/*--------------- LCD Messages ---------------*/
const uint8_t MSG_HOST_INIT[]        = "> Host Library Initialized \n\r";
const uint8_t MSG_DEV_ATTACHED[]     = "> Device Attached \n\r";
const uint8_t MSG_DEV_DISCONNECTED[] = "> Device Disconnected \n\r";
const uint8_t MSG_DEV_ENUMERATED[]   = "> Enumeration completed \n\r";
const uint8_t MSG_DEV_HIGHSPEED[]    = "> High speed device detected \n\r";
const uint8_t MSG_DEV_FULLSPEED[]    = "> Full speed device detected \n\r";
const uint8_t MSG_DEV_LOWSPEED[]     = "> Low speed device detected \n\r";
const uint8_t MSG_DEV_ERROR[]        = "> Device fault \n\r";

const uint8_t MSG_MSC_CLASS[]        = "> Mass storage device connected \n\r";
const uint8_t MSG_HID_CLASS[]        = "> HID device connected \n\r";
const uint8_t MSG_DISK_SIZE[]        = "> Size of the disk in MBytes: \n\r";
const uint8_t MSG_LUN[]              = "> LUN Available in the device: \n\r";
const uint8_t MSG_ROOT_CONT[]        = "> Exploring disk flash ... \n\r";
const uint8_t MSG_WR_PROTECT[]       = "> The disk is write protected \n\r";
const uint8_t MSG_UNREC_ERROR[]      = "> UNRECOVERED ERROR STATE \n\r";

/**
* @}
*/


/** @defgroup USBH_USR_Private_FunctionPrototypes
* @{
*/
static uint8_t Explore_Disk (char* path , uint8_t recu_level);
static void Toggle_Leds(void);
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Functions
* @{
*/ 


/**
* @brief  USBH_USR_Init 
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBH_USR_Init(void)
{
  static uint8_t startup = 0;  
  
  if(startup == 0 )
  {
    startup = 1;
    /* Configure the LEDs */
    STM_EVAL_LEDInit(LED3); 
    STM_EVAL_LEDInit(LED4); 
    STM_EVAL_LEDInit(LED5); 
    STM_EVAL_LEDInit(LED6); 

		/* init the Debug COM */
//    STM32f4_Discovery_Debug_Init();
      
    Log.d("> USB OTG FS MSC Host \n\r");
    Log.d("> USB Host library started. \n\r"); 
    Log.d("> USB Host Library v2.1.0 \n\r" );
  }
}

/**
* @brief  USBH_USR_DeviceAttached 
*         Displays the message on LCD on device attached
* @param  None
* @retval None
*/
void USBH_USR_DeviceAttached(void)
{
  Log.d((void *)MSG_DEV_ATTACHED);
}


/**
* @brief  USBH_USR_UnrecoveredError
* @param  None
* @retval None
*/
void USBH_USR_UnrecoveredError (void)
{
  
  /* Set default screen color*/ 
  Log.d((void *)MSG_UNREC_ERROR); 
}


/**
* @brief  USBH_DisconnectEvent
*         Device disconnect event
* @param  None
* @retval Staus
*/
void USBH_USR_DeviceDisconnected (void)
{
  /* Set default screen color*/
  Log.d((void *)MSG_DEV_DISCONNECTED);
  STM_EVAL_LEDOff(LED5)	; STM_EVAL_LEDOff(LED6)	; LEDind = LED3	;
}
/**
* @brief  USBH_USR_ResetUSBDevice 
* @param  None
* @retval None
*/
void USBH_USR_ResetDevice(void)
{
  /* callback for USB-Reset */
}


/**
* @brief  USBH_USR_DeviceSpeedDetected 
*         Displays the message on LCD for device speed
* @param  Device speed
* @retval None
*/
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
  if(DeviceSpeed == HPRT0_PRTSPD_HIGH_SPEED)
  {
    Log.d((void *)MSG_DEV_HIGHSPEED);
  }  
  else if(DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED)
  {
    Log.d((void *)MSG_DEV_FULLSPEED);
  }
  else if(DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED)
  {
    Log.d((void *)MSG_DEV_LOWSPEED);
  }
  else
  {
    Log.d((void *)MSG_DEV_ERROR);
  }
}

/**
* @brief  USBH_USR_Device_DescAvailable 
*         Displays the message on LCD for device descriptor
* @param  device descriptor
* @retval None
*/
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{ 
  USBH_DevDesc_TypeDef *hs;
  hs = DeviceDesc;  
  
  
  Log.d("VID\\PID : %04Xh\\%04Xh, "
			 "USB v%X.%02X\n" ,
			 hs->idVendor,hs->idProduct,
			 hs->bcdUSB>>8,hs->bcdUSB&0xFF);
  if(hs->idProduct == 0x155B){ STM_EVAL_LEDOff(LED3)	; STM_EVAL_LEDOff(LED6)	; LEDind = LED5	;}
  if(hs->idProduct == 0x1506){ STM_EVAL_LEDOff(LED3)	; STM_EVAL_LEDOff(LED6)	; LEDind = LED6	;}
}

/**
* @brief  USBH_USR_DeviceAddressAssigned 
*         USB device is successfully assigned the Address 
* @param  None
* @retval None
*/
void USBH_USR_DeviceAddressAssigned(void)
{
  
}


const char*	EpAttr7[] = {"OUT"," IN"}	;
const char*	EpAttr01[] = {"CTRL","ISOCHRONOUS","BULK","INTERRUPT"}	;

/**
* @brief  USBH_USR_Conf_Desc 
*         Displays the message on LCD for configuration descriptor
* @param  Configuration descriptor
* @retval None
*/
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc)
{
  USBH_InterfaceDesc_TypeDef *id;
  int		ix,ep,ixep;
  
  id = itfDesc;  
  
  Log.d("NumberOfInterfaces:%d\n",cfgDesc->bNumInterfaces);
  
  for(ix=0;ix<cfgDesc->bNumInterfaces && ix<USBH_MAX_NUM_INTERFACES;ix++){
    Log.d("  Interface: %d, "
			"NumEndpoints: %d, "
			"Class: 0x%02X, "
			"Subclass: 0x%02X, "
			"Protocol: 0x%02X\n"
			,itfDesc[ix].bInterfaceNumber
			,itfDesc[ix].bNumEndpoints
			,itfDesc[ix].bInterfaceClass
			,itfDesc[ix].bInterfaceSubClass
			,itfDesc[ix].bInterfaceProtocol);			
  
    for(ep=0;ep<itfDesc[ix].bNumEndpoints && ep<USBH_MAX_NUM_ENDPOINTS;ep++){
	  ixep = ix * USBH_MAX_NUM_ENDPOINTS + ep	;
      Log.d("    Endpoint: 0x%X, "
			"%s, %s, %d bytes\n"
			,epDesc[ixep].bEndpointAddress & 7
			,EpAttr7[(epDesc[ixep].bEndpointAddress>>7)&1]
			,EpAttr01[epDesc[ixep].bmAttributes&3]
			,epDesc[ixep].wMaxPacketSize);
	}  
  }
  
  if((*id).bInterfaceClass  == 0x08)  	  Log.d((void *)MSG_MSC_CLASS)	;
  else if((*id).bInterfaceClass  == 0x03) Log.d((void *)MSG_HID_CLASS)	;      
}

/**
* @brief  USBH_USR_Manufacturer_String 
*         Displays the message on LCD for Manufacturer String 
* @param  Manufacturer String 
* @retval None
*/
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
  Log.d("> Manufacturer : %s\n\r", (char *)ManufacturerString);
}

/**
* @brief  USBH_USR_Product_String 
*         Displays the message on LCD for Product String
* @param  Product String
* @retval None
*/
void USBH_USR_Product_String(void *ProductString)
{
  Log.d("> Product : %s\n\r", (char *)ProductString);  
}

/**
* @brief  USBH_USR_SerialNum_String 
*         Displays the message on LCD for SerialNum_String 
* @param  SerialNum_String 
* @retval None
*/
void USBH_USR_SerialNum_String(void *SerialNumString)
{
  Log.d( "> Serial Number : %s\n\r", (char *)SerialNumString);    
} 



/**
* @brief  EnumerationDone 
*         User response request is displayed to ask application jump to class
* @param  None
* @retval None
*/
void USBH_USR_EnumerationDone(void)
{
  
  /* Enumeration complete */
  Log.d((void *)MSG_DEV_ENUMERATED);
  
  Log.d("> To see the root content of the disk : \n\r" );
  Log.d("> Press Key... \n\r");
  
} 


/**
* @brief  USBH_USR_DeviceNotSupported
*         Device is not supported
* @param  None
* @retval None
*/
void USBH_USR_DeviceNotSupported(void)
{
  Log.d("> Device not supported. \n\r"); 
}  


/**
* @brief  USBH_USR_UserInput
*         User Action for application state entry
* @param  None
* @retval USBH_USR_Status : User response for key button
*/
USBH_USR_Status USBH_USR_UserInput(void)
{
  USBH_USR_Status usbh_usr_status = USBH_USR_NO_RESP	;
  
//  /*USER1 B3 is in polling mode to detect user action */
  /*Key B3 is in polling mode to detect user action */
  if(1){//STM_EVAL_PBGetState(BUTTON_USER) == SET){
    usbh_usr_status = USBH_USR_RESP_OK			;} 
  return usbh_usr_status;
}

/**
* @brief  USBH_USR_OverCurrentDetected
*         Over Current Detected on VBUS
* @param  None
* @retval Staus
*/
void USBH_USR_OverCurrentDetected (void)
{
  Log.d ("> Overcurrent detected.\n\r");
}


/**
* @brief  USBH_USR_MSC_Application 
*         Demo application for mass storage
* @param  None
* @retval Staus
*/
int USBH_USR_MSC_Application(void)
{
  FRESULT res;
  uint8_t writeTextBuff[] = "STM32 Connectivity line Host Demo application using FAT_FS   ";
  uint16_t bytesWritten, bytesToWrite;
  
  switch(USBH_USR_ApplicationState)
  {
  case USH_USR_FS_INIT: 
    
    /* Initialises the File System*/
    if ( f_mount( 0, &fatfs ) != FR_OK ) 
    {
      /* efs initialisation fails*/
      Log.d("> Cannot initialize File System.\n\r");
      return(-1);
    }
    Log.d("> File System initialized.\n\r");
    Log.d("> Disk capacity : %u Bytes\n\r", USBH_MSC_Param.MSCapacity * \
      USBH_MSC_Param.MSPageLength); 
    
    if(USBH_MSC_Param.MSWriteProtect == DISK_WRITE_PROTECTED)
    {
      Log.d((void *)MSG_WR_PROTECT);
    }
    
    USBH_USR_ApplicationState = USH_USR_FS_READLIST;
    break;
    
  case USH_USR_FS_READLIST:
    
    Log.d((void *)MSG_ROOT_CONT);
    Explore_Disk("0:/", 1);
    line_idx = 0;   
    USBH_USR_ApplicationState = USH_USR_FS_WRITEFILE;
    
    break;
    
  case USH_USR_FS_WRITEFILE:
    Log.d("Press Key to write file\n\r");
    USB_OTG_BSP_mDelay(100);

    /*USER1 B3 in polling*/
    while((HCD_IsDeviceConnected(&USB_OTG_Core)) && \
      (STM_EVAL_PBGetState (BUTTON_USER) == RESET))          
//      (Get_Peek_Key() != EOF))          
    {
      Toggle_Leds();
    }

    /* Writes a text file, STM32.TXT in the disk*/
    Log.d("> Writing File to disk flash ...\n");
    if(USBH_MSC_Param.MSWriteProtect == DISK_WRITE_PROTECTED)
    {
      
      Log.d ( "> Disk flash is write protected \n");
      USBH_USR_ApplicationState = USH_USR_FS_DRAW;
      break;
    }
    
    /* Register work area for logical drives */
    f_mount(0, &fatfs);
    
    if(f_open(&file, "0:STM32.TXT",FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
    { 
      /* Write buffer to file */
      bytesToWrite = sizeof(writeTextBuff); 
      res= f_write (&file, writeTextBuff, bytesToWrite, (void *)&bytesWritten);   
      
      if((bytesWritten == 0) || (res != FR_OK)) /*EOF or Error*/
      {
        Log.d("> STM32.TXT CANNOT be writen.\n");
      }
      else
      {
        Log.d("> 'STM32.TXT' file created\n");
      }
      
      /*close file and filesystem*/
      f_close(&file);
      f_mount(0, NULL); 
    }
    
    else
    {
      Log.d ("> STM32.TXT created in the disk\n");
    }

    USBH_USR_ApplicationState = USH_USR_FS_INIT;   
    break;
  default: break;
  }
  return(0);
}

/**
* @brief  Explore_Disk 
*         Displays disk content
* @param  path: pointer to root path
* @retval None
*/
static uint8_t Explore_Disk (char* path , uint8_t recu_level)
{
  FRESULT res;
  FILINFO fno;
  DIR dir;
  char *fn;
  char tmp[14];
  
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    while(HCD_IsDeviceConnected(&USB_OTG_Core)) 
    {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0) 
      {
        break;
      }
      if (fno.fname[0] == '.')
      {
        continue;
      }

      fn = fno.fname;
      strcpy(tmp, fn); 

      line_idx++;
      if(line_idx > 9)
      {
        line_idx = 0;
        Log.d("Press Key to continue...\r\n");

        /*USER1 B3 in polling*/
        while((HCD_IsDeviceConnected(&USB_OTG_Core)) && \
      (STM_EVAL_PBGetState (BUTTON_USER) == RESET))          
//              (Get_Peek_Key() != EOF))
        {
          Toggle_Leds();
          
        }
      } 
      
      if(recu_level == 1)
      {
        Log.d("   |__");
      }
      else if(recu_level == 2)
      {
        Log.d("   |   |__");
      }
      if((fno.fattrib & AM_MASK) == AM_DIR)
      {
        strcat(tmp, "\n"); 
        Log.d((void *)tmp);
      }
      else
      {
        strcat(tmp, "\n"); 
        Log.d((void *)tmp);
      }

      if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recu_level == 1))
      {
        Explore_Disk(fn, 2);
      }
    }
  }
  return res;
}

/**
* @brief  Toggle_Leds
*         Toggle leds to shows user input state
* @param  None
* @retval None
*/
static void Toggle_Leds(void)
{
  static uint32_t i;
  if (i++ == 0x10000)
  {
    STM_EVAL_LEDToggle(LEDind);
    i = 0;
  }  
}

/**
* @brief  USBH_USR_DeInit
*         Deint User state and associated variables
* @param  None
* @retval None
*/
void USBH_USR_DeInit(void)
{
  USBH_USR_ApplicationState = USH_USR_FS_INIT;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

