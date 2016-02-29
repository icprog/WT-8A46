/**
  ******************************************************************************
  * @file    main.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    26-February-2014
  * @brief   Header for main.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"

/* EVAL includes component */
#include "stm32f429i_WireTester.h"
#include "stm32f429i_wt_sdram.h"
//#include "stm32f429i_discovery_ts.h"
//#include "stm32f429i_discovery_io.h"
#include "stm32f429i_wt_lcd.h"

/* FatFs includes component */
#include "ff_gen_drv.h"
#include "usbh_diskio.h"
#include "sd_diskio.h"
#include "gui.h"
	 
/* Exported types ------------------------------------------------------------*/
typedef union
{
  uint32_t d32;
  struct
  {
    uint32_t enable_sprite     : 1;
    uint32_t enable_background : 1;
    uint32_t use_180Mhz        : 1;
    uint32_t disable_flex_skin : 1;  
  }b;
}
SystemSettingsTypeDef;


//配置文件
typedef struct{
	
	uint8_t 	BackLight;		//背光亮度，0-10
	uint8_t 	AudioVolume;	//语音音量，0-10
	uint8_t  	Print;			  //本地打印 0:no, 1:yes
	uint8_t 	InfoRTC[7];		//时间、日期、星期
	uint8_t 	Mode_TestW;		//导通阈值0:365R,1:1K 2:10K
	uint8_t 	Mode_TestZ;		//高阻测试模式, 0:12R, 1:27R, 2:100R, 3:125R; 4:365R, 5:1K, 6:10k
	uint8_t   Mode_TestK;		//开关测试模式, 0:12R, 1:27R, 2:100R, 3:125R; 4:365R, 5:1K, 6:10k,
	uint8_t 	NetWork[10];	//网络SSID
	uint8_t 	SecKey[10];	  //密钥
	uint8_t 	Server_IP[4];	//服务器地址
	uint16_t 	TCP_Port;	    //端口号
	uint8_t 	Dev_ID;	      //设备编号
	uint8_t 	TestMode;   	//0-自动模式  1-手动模式  2-定时模式
	uint8_t 	AudioOK;		  //声音文件，0-3
	uint8_t 	AudioNG;		  //声音文件，0-3
	uint8_t		StudyOptimizingMode;	//0:full test完整模式, 1:only test Z bettwn max & min W point,优化模式
	uint8_t   Lockstat;			//0:unlock  1:lock
	uint8_t   TestRule;    //测试规则
	uint8_t   Passwd[6];    //密码
	uint8_t   IP_Addr[4];   //IP地址
	uint8_t   MAC_Addr[6];   //MAC地址
	uint8_t 	Mode_TestCap;		//电容阈值, 0:0.1uF, 1:0.47uF, 2:1.0uF, 3:4.7uF; 
	uint8_t 	StudyMode;		//自学习规则, 0:导通测试, 1:导通+电阻
	uint8_t 	Print_ID;		 //测试文件索引
}
WT_ConfigTypedef;  


//导通仪状态
typedef struct{
	
	uint8_t 	mode;					//0：待机状态；1：正常运行测试程序；2：正在运行自学习程序；
													//3：正在运行找点程序；4：正在运行自检程序；5：正在运行调试程序；
	uint8_t 	error;				//0：no error, 1:error
	
}
WT_TesterStatusTypedef;  

/* Exported variables --------------------------------------------------------*/
extern WT_TesterStatusTypedef TesterStatus;


/* Exported constants --------------------------------------------------------*/
#define NUM_DISK_UNITS       2
#define USB_DISK_UNIT        0
#define MSD_DISK_UNIT        1

#define CALIBRATION_BKP0                    RTC_BKP_DR0
#define CALIBRATION_BKP1                    RTC_BKP_DR1
#define CALIBRATION_GENERAL_SETTINGS_BKP    RTC_BKP_DR2
#define CALIBRATION_IMAGE_SETTINGS_BKP      RTC_BKP_DR3
#define CALIBRATION_BENCH_SETTING_BKP       RTC_BKP_DR4
#define CALIBRATION_VIDEOPLAYER_SETTING_BKP RTC_BKP_DR5




#define DEF_WT_NAME			"线束检测仪"
#define DEF_WT_MODEL		"WT-8A46"

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

extern SystemSettingsTypeDef 	settings;
extern osMessageQId 					WireTestEvent;
extern osMessageQId 					WireStudyEvent;
extern osMessageQId 					WireSelfCheckEvent;
extern osMessageQId 					WireFindPointEvent;
extern osMessageQId 					UartCOM1Event;
extern osMessageQId 					UartCOM2Event;
extern osMessageQId 					UartAudioEvent;
extern osMessageQId 					Uart24GHzEvent;
extern osMessageQId 					UartInfrEvent;
extern WT_ConfigTypedef				WT_Config;

extern void num_converter(uint16_t num, char * str);
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
