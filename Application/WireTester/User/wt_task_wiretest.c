/**
  ******************************************************************************
  * @progect LZY Wire Cube Tester
	* @file    wt_task_wiretest.c
  * @author  LZY Zhang Manxin
  * @version V1.0.0
  * @date    2014-8-5
  * @brief   This file provides the wire test functions
  ******************************************************************************
  */

#define WT_WINDOWS_WIRETEST_GLOBALS



/* Includes ------------------------------------------------------------------*/
#include "wt_task_wiretest.h"
#include "main.h"
#include "wt_ad_app.h"
#include "k_bsp.h" 
 

/* External variables --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* External functions --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
													 


/**
  * @brief  WIRETEST task
  * @param  argument: pointer that is passed to the thread function as start argument.
  * @retval None
  */
void WIRETESTThread(void const * argument)
{
	osEvent event;
  uint8_t res;
	uint8_t res_wait;
	uint8_t res_last;
	uint32_t i;
	uint8_t test_okcnt;
	
	//Init 
//	WT_TestConfig.TestMode_W = WT_WZK_MOD_100R;
//	WT_TestConfig.TestMode_Z = WT_WZK_MOD_100R;
//	WT_TestConfig.TestMode_K = WT_WZK_MOD_100R;
	
	WT_Config.TestMode = 0;	//0-自动模式，1-手动模式，2-定时模式
	TestFile.number_point = 0;	//端口数量
	TestFile.number_line = 0;		//通道数量
	TestFile.test_num = 0;//测试总数
	test_okcnt = 0;
	
  for( ;; )
  {
		TestFile.task_status = 0;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
		//TestFile.test_num=0;
		//clear message
		while(1)
		{
			event = osMessageGet( WireTestEvent, 0);
			if(event.status != osEventMessage) break;
		}
		//wait new message
		event = osMessageGet( WireTestEvent, osWaitForever );
    TestFile.command=1;
    if( event.status == osEventMessage )
    {
      switch(event.value.v)
      {
				case WIRETEST_START_EVENT:
					// check Test file
					if((TestFolder.status != 1) || (TestFile.file_status != 1))	//init error
					{
						if(WT_StoreFiles_Init() != 0) break;
					}

					switch(WT_Config.TestMode)	//0-自动模式，1-手动模式，2-定时模式
					{
						case 0:	//自动模式
							for(;;)
							{
								test_start:
								TestFile.task_status = 1;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
								res = WireTest_Process();	//0:ok, 1:file errror, 2:test error, 3-user cancel; 
								if(res == 0)
								{
									test_okcnt++;
									if(test_okcnt < 2 && WT_Config.TestRule == 1) goto test_start;  ////0-循环检测  1-单次检测 
									else test_okcnt=0;
										
									TestFile.task_status = 3;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
									//LED4_OFF();
									LED4_ON_G();
									//发送红外信号
									osMessagePut(UartInfrEvent, UartInfr_TX_Event, 0);
									
									//osMessagePut(Uart24GHzEvent, Uart24GHZ_TX_Event, 0);	//wifi发送测试结果							
									osMessagePut(UartAudioEvent, UartAudioOK_TX_Event, 0);
									if(WT_Config.Print == 1 ) osMessagePut(UartCOM1Event, UartCOM1_TX_Event, 0);//本地打印
									TestFile.test_num++;
									TestFile.task_status = 4;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
									res_wait = WireTest_WaitForNext();	//0:all remove, 1:file error, 3-user cancel; 
									if(res_wait != 0) 
									{
										TestFile.task_status = 2;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
										osMessagePut(UartAudioEvent, UartAudioSTOP_TX_Event, 0);
										if(res_wait == 3)//user cancel 清除标志位
										{
											for(i=0; i<TestFile.item_total; i++)
											TestFile.test_item[i].result = 0;
											TestFile.test_ErrFlag = 0;	//for max error ID，0-no error, 1-error detect and retest
										}
										break;
									}
									else//all remove 清除标志位
									{
										for(i=0; i<TestFile.item_total; i++)
										TestFile.test_item[i].result = 0;
										TestFile.test_ErrFlag = 0;	//for max error ID，0-no error, 1-error detect and retest
									}
									
								}
								else//res !=0  0:ok, 1:file errror, 2:test error, 3-user cancel; 					
								{				
									TestFile.task_status = 2;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
									//Infra_PWM_OFF();//关闭红外信号
									UART_Infr_Switch = 1;//关闭红外信号
									//osMessagePut(Uart24GHzEvent, Uart24GHZ_TX_Event, 0);	//wifi发送测试结果
									if(res != 3) osMessagePut(UartAudioEvent, UartAudioNG_TX_Event, 0);
									if(res == 3)//user cancel
									{
										for(i=0; i<TestFile.item_total; i++)
										TestFile.test_item[i].result = 0;
										TestFile.test_ErrFlag = 0;	//for max error ID，0-no error, 1-error detect and retest
										LED4_OFF();
										osMessagePut(UartAudioEvent, UartAudioSTOP_TX_Event, 0);
									}
									
									break;
								}
							}
							break;
						
						case 1:	//手动模式
							TestFile.task_status = 1;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
							res = WireTest_Process();	//0:ok, 1:file errror, 2:test error, 3-user cancel; 
							if(res == 0)
							{
								TestFile.task_status = 3;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
								LED4_OFF();
								//BSP_UartAudioOK_BeepOn(2);
								osMessagePut(UartAudioEvent, UartAudioOK_TX_Event, 0);
								TestFile.test_num++;
							}
							else					
							{
								TestFile.task_status = 2;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
								LED4_ON_RG();
							}
							break;
						
						default:
							break;						
					}
					break;
				
				default:
					break;
      }
    }
  }
}


//void WT_Test(void)
//{
//	static uint16_t i=0;
//	
//	i++;
//	osMessagePut (WireTestEvent, WIRETEST_START_EVENT, 0);
//}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
