/*
*********************************************************************************************************
*                                     MICRIUM BOARD SUPPORT SUPPORT
*
*                          (c) Copyright 2003-2012; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                    MICRIUM BOARD SUPPORT PACKAGE
*                                       SERIAL (UART) INTERFACE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                           STM3220G-EVAL
*                                         Evaluation Board
*
* Filename      : BSP_RS485GKJ.c
* Version       : V1.00
* Programmer(s) : SL
*********************************************************************************************************
*/

/* Define ------------------------------------------------------------------*/
#define  WT_UART_24GHZ_GLOBALS


/* Includes ------------------------------------------------------------------*/
#include "k_bsp.h"
#include "main.h"
#include "verision.h"
#pragma diag_suppress 870 



static void WT_UART_24GHz_Rx_IT(uint8_t dat);
static uint8_t WT_UART_24GHz_Cnfig(void);
static void Uart24G_RX_Process(void);
static void Uart24GHz_UploadDevStat(char *buff);
static void Uart24GHz_UploadTestRes(void);
static void Uart24GHz_UploadTestStat(char *buff);
static void Uart24G_UploadDevInfo(char *buff);
static void Uart24G_UploadSysconfig(char *buff);
static void Uart24G_UploadNetconfig(char *buff);
static void Uart24G_UploadRTC(char *buff);
static void Uart24G_UploadTestFolder(char *buff);
static void Uart24G_UpdateSysconfig(char *buff);
static void Uart24G_UpdateRTC(char *buff);
static void Uart24G_UploadTestFile(char *RxBuff);
static uint8_t Uart24G_Download_Start(char *buff);
static void Uart24G_Download_Process(char *buff);
static void Uart24G_Download_End(char *buff);
static uint16_t last_rec;
static uint8_t ok_cnt;
//static uint16_t frame_cnt = 0;

static uint8_t filename[50];
//static uint32_t str_size=0;

extern void set_systemtime(uint8_t sec,uint8_t min, uint8_t hour, uint8_t day, uint8_t month,uint16_t year);
extern char *itoa(int num, char *str, int radix);
extern void wt_SetText_Menu(const char * pTitle); 
extern void wt_SetText_Status(const char * pTitle); 

#define TIME_OUT  500

/**
  * @brief  wire self check task
  * @param  argument: pointer that is passed to the thread function as start argument.
  * @retval None
  */
void UART24GHZThread(void const * argument)
{
	osEvent event;
	uint16_t i;
	static uint16_t cnt=0;
	
	WT_UART_24GHz_Init();	//Init
	
	UART_24GHz_Buffer_RxClear();
	osDelay(6000);
	WT_UART_24GHz_Cnfig();	//config
	
	
  for( ;; )
  {
    //clear message
		while(1)
		{
			event = osMessageGet( WireTestEvent, 0);
			if(event.status != osEventMessage) break;
		}
	//	event = osMessageGet(Uart24GHzEvent, osWaitForever );
		event = osMessageGet(Uart24GHzEvent, 10000 );
		
//		for(i=0;i<3000;i++)
//		{
//			osDelay(10);
//			event = osMessageGet( WireTestEvent, 0);
//			if( event.status == osEventMessage ) break;
//			else i++;
//			if(i>=3000)
//			{
//				last_rec = UART_24GHz_Cnt_Buffer_Rx;
//				for(i=0;i<30;i++)
//				{
//					osDelay(3);
//					if(last_rec == UART_24GHz_Cnt_Buffer_Rx ) break;
//					else last_rec = UART_24GHz_Cnt_Buffer_Rx;
//				}	
//				
//				if(UART_24GHz_Cnt_Buffer_Rx == 0)
//				{
//					cnt++;
//					BSP_UartNRF_PowerOFF();
//					osDelay(1000);
//					WT_UART_24GHz_Init();
//					osDelay(1000);
//					UART_24GHz_Buffer_RxClear();
//					WT_UART_24GHz_Cnfig();	//config
//				}
//				UART_24GHz_Cnt_Buffer_Rx = 0;
//			}
//		}   
		
		
    if( event.status == osEventMessage )
    {
      switch(event.value.v)
      {
				case Uart24GHZ_RX_Event:
					Uart24G_RX_Process();
					break;		
				case Uart24GHZ_TX_Event:
					Uart24GHz_UploadTestRes();				
					break;
				case Uart24GHZ_CONF_Event:
					BSP_UartNRF_PowerOFF();
					osDelay(1000);
					WT_UART_24GHz_Init();
					osDelay(1000);
					UART_24GHz_Buffer_RxClear();
					WT_UART_24GHz_Cnfig();	//config
					break;
				default:
					break;
      }
    }
		else if( event.status == osEventTimeout )
		{
			last_rec = UART_24GHz_Cnt_Buffer_Rx;
			for(i=0;i<30;i++)
			{
				osDelay(3);
				if(last_rec == UART_24GHz_Cnt_Buffer_Rx ) break;
				else last_rec = UART_24GHz_Cnt_Buffer_Rx;
			}	
			
			if(UART_24GHz_Cnt_Buffer_Rx == 0)
			{
				cnt++;
				BSP_UartNRF_PowerOFF();
				osDelay(1000);
				WT_UART_24GHz_Init();
				osDelay(1000);
				UART_24GHz_Buffer_RxClear();
				WT_UART_24GHz_Cnfig();	//config
			}
			UART_24GHz_Cnt_Buffer_Rx = 0;
		}
		else	//error
		{
			osDelay(1000);
		}
  }
}


/**
  * @brief  WT_UART_24GHz_Init
  * @param  None
  * @retval None
  */
void WT_UART_24GHz_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	uint32_t i;
	
	for(i=0;i<UART_24GHz_RX_Bufer_Length;i++) UART_24GHz_Buffer_Rx[i]=0;
	UART_24GHz_Cnt_Buffer_Rx = 0;
	Is_UART_24GHz_Rx_Come = 0;
	
  UartHandle_24GHz.Instance        = UART_24GHz;
  UartHandle_24GHz.Init.BaudRate   = 115200;
  UartHandle_24GHz.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle_24GHz.Init.StopBits   = UART_STOPBITS_1;
  UartHandle_24GHz.Init.Parity     = UART_PARITY_NONE;
  UartHandle_24GHz.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle_24GHz.Init.Mode       = UART_MODE_TX_RX;
  HAL_UART_Init(&UartHandle_24GHz);
	
	/* Enable the PowerSW_WireLess_PIN Clock */
  PowerSW_GPIO_CLK_ENABLE();

  /* Configure the PowerSW_WireLess_PIN  */
  GPIO_InitStruct.Pin = PowerSW_WireLess_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(PowerSW_PORT, &GPIO_InitStruct);
	BSP_UartNRF_PowerON();
}


/**
  * @brief  This function handles UART interrupt request.
  * @param  huart: UART handle
  * @retval None
  */
void WT_UART_24GHz_IRQHandler(UART_HandleTypeDef *huart)
{
  uint32_t tmp1 = 0, tmp2 = 0;

  tmp1 = __HAL_UART_GET_FLAG(huart, UART_FLAG_PE);
  tmp2 = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_PE);  
  /* UART parity error interrupt occurred ------------------------------------*/
  if((tmp1 != RESET) && (tmp2 != RESET))
  { 
    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_PE);
    
    huart->ErrorCode |= HAL_UART_ERROR_PE;
  }
  
  tmp1 = __HAL_UART_GET_FLAG(huart, UART_FLAG_FE);
  tmp2 = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_ERR);
  /* UART frame error interrupt occurred -------------------------------------*/
  if((tmp1 != RESET) && (tmp2 != RESET))
  { 
    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_FE);
    
    huart->ErrorCode |= HAL_UART_ERROR_FE;
  }
  
  tmp1 = __HAL_UART_GET_FLAG(huart, UART_FLAG_NE);
  tmp2 = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_ERR);
  /* UART noise error interrupt occurred -------------------------------------*/
  if((tmp1 != RESET) && (tmp2 != RESET))
  { 
    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_NE);
    
    huart->ErrorCode |= HAL_UART_ERROR_NE;
  }
  
  tmp1 = __HAL_UART_GET_FLAG(huart, UART_FLAG_ORE);
  tmp2 = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_ERR);
  /* UART Over-Run interrupt occurred ----------------------------------------*/
  if((tmp1 != RESET) && (tmp2 != RESET))
  { 
    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE);
    
    huart->ErrorCode |= HAL_UART_ERROR_ORE;
  }
  
  tmp1 = __HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE);
  tmp2 = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp1 != RESET) && (tmp2 != RESET))
  { 
    //UART_Receive_IT(huart);
		WT_UART_24GHz_Rx_IT((uint8_t)(huart->Instance->DR & (uint8_t)0x00FF));	//cndz, 20140723
    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_RXNE);
  }
  
  tmp1 = __HAL_UART_GET_FLAG(huart, UART_FLAG_TC);
  tmp2 = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TC);
  /* UART in mode Transmitter ------------------------------------------------*/
  if((tmp1 != RESET) && (tmp2 != RESET))
  {
    //UART_Transmit_IT(huart);
  }
  
  if(huart->ErrorCode != HAL_UART_ERROR_NONE)
  {
    /* Set the UART state ready to be able to start again the process */
    huart->State = HAL_UART_STATE_READY;
    
    HAL_UART_ErrorCallback(huart);
  }  
}


/**
  * @brief  WT_UART_24GHz_Rx
  * @param  uint8_t dat
  * @retval None
  */
static void WT_UART_24GHz_Rx_IT(uint8_t dat)
{
	/* receiver data */
	UART_24GHz_Buffer_Rx[UART_24GHz_Cnt_Buffer_Rx] = dat;
	if(UART_24GHz_Cnt_Buffer_Rx < (UART_24GHz_RX_Bufer_Length-2)) UART_24GHz_Cnt_Buffer_Rx++;

	if(UART_24GHz_Cnt_Buffer_Rx >=3 )
	{
		//Is_UART_COM2_Rx_Come = 1;
		osMessagePut(Uart24GHzEvent, Uart24GHZ_RX_Event, 0);	//收到数据帧
		//UartCOM2_RX_Process();
	}

}

/**
  * @brief  UART_24GHz_Buffer_RxClear
  * @param  None
  * @retval None
  */
static void UART_24GHz_Buffer_RxClear(void)
{
	uint32_t i;
	
	for(i=0;i<UART_24GHz_RX_Bufer_Length;i++) UART_24GHz_Buffer_Rx[i]=0;
	UART_24GHz_Cnt_Buffer_Rx = 0;
	Is_UART_24GHz_Rx_Come = 0;
}

/**
  * @brief  WT_UART_24GHz_Rx
  * @param  uint8_t dat
  * @retval None
  */
uint8_t WT_UART_24GHz_WrBuf(uint8_t* pData, uint8_t length)
{
	while(length--)
	{
		if(UART_WaitOnFlagUntilTimeout(&UartHandle_24GHz, UART_FLAG_TXE, RESET, 100) != HAL_OK)
		{
			return 1;
		}
		UartHandle_24GHz.Instance->DR = (*pData++ & (uint8_t)0xFF);
	}
	
	//if(UART_WaitOnFlagUntilTimeout(&UartHandle_24GHz, UART_FLAG_TXE, RESET, 100) != HAL_OK)
	if(UART_WaitOnFlagUntilTimeout(&UartHandle_24GHz, UART_FLAG_TXE, RESET, 100) != HAL_OK)
	{
		return 1;
	}
	
	return 0;
}


static void WT_GetIP(char *buff)
{
	char * ptr;	
	uint8_t index = 0;
	strtok(buff,",");	
		index=0;
		while(1)
		{
			ptr=strtok(NULL,".,");
			if(ptr == 0) break;
			switch(index)
			{
				case 0:
					index++;
					WT_Config.IP_Addr[0]= atoi(ptr);
					break;
				case 1:
					index++;
					WT_Config.IP_Addr[1]= atoi(ptr);
					break;
				case 2:
					index++;
					WT_Config.IP_Addr[2]= atoi(ptr);
					break;
				case 3:
					index++;
					WT_Config.IP_Addr[3]= atoi(ptr);
					break;
				default:
					break;
			}
		}
}

static void WT_GetMAC(char *buff)
{
	uint8_t i=0;
	char ptr[2];
	for(i=0;i<6;i++)
	{
		strncpy(ptr,&buff[4+i*2],2);
		WT_Config.MAC_Addr[i]=strtol(ptr,NULL,16);
	}
	
}

static uint16_t WT_GetSockID(char *buff)
{
	uint16_t sockid=0;
	char ptr[8];
	strcpy(ptr,&buff[4]);
	sockid = atoi(ptr);
	return sockid;
}

static uint8_t WT_UART_24GHz_Cnfig(void)
{
	uint16_t i;
	uint8_t mode=0;
	uint16_t sock_Id=0;
	char buff[30];
	char cmd[50];
	char res[20];
	char serv_ip[20];
	
	
	memset(cmd,0,50);
	memset(res,0,20);
	memset(serv_ip,0,20);
	
	sprintf(cmd,"AT+SSID=\"%s\"\n",(char *)WT_Config.NetWork);
	sprintf(serv_ip,"%d.%d.%d.%d",WT_Config.Server_IP[0],WT_Config.Server_IP[1],WT_Config.Server_IP[2],WT_Config.Server_IP[3]);
	WT_UART_24GHz_WrBuf((uint8_t *)cmd, strlen(cmd));
	
	while(1)
	{
		for(i=0;i<3000;i++) //wait for receive first data
		{
			osDelay(3);
			if(UART_24GHz_Cnt_Buffer_Rx != 0) break;
		}
		
		if(UART_24GHz_Cnt_Buffer_Rx == 0) //3S内无回复，重发命令
		{
//			WT_UART_24GHz_WrBuf((uint8_t *)cmd, strlen(cmd));
			return 0;
		}
		else //已收到数据，等待接收完成
		{
			last_rec = UART_24GHz_Cnt_Buffer_Rx;
			for(i=0;i<300;i++)
			{
				osDelay(3);
				if(last_rec == UART_24GHz_Cnt_Buffer_Rx) break;
				else last_rec = UART_24GHz_Cnt_Buffer_Rx;
			}
			
			if(i<10)  //接收完好
			{				
				last_rec =0 ;
				memset(buff,0,30);
				strncpy(buff,(char *)UART_24GHz_Buffer_Rx,30);
				
				//接收完成，成功、失败
				if(strncmp(buff,"+OK",3)==0 && ok_cnt==0) 
				{
					mode=1;
					ok_cnt++;
				}
				else if(strncmp(buff,"+OK",3)==0 && ok_cnt==1) 
				{
					mode=2;
					ok_cnt++;
				}
				else if(strncmp(buff,"+OK",3)==0 && ok_cnt==2) 
				{
					mode=3;
					ok_cnt++;
				}
				else if(strncmp(buff,"+OK",3)==0 && ok_cnt==3) 
				{
					mode=4;
					ok_cnt++;
				}
				else if(strncmp(buff,"+OK",3)==0 && ok_cnt==4) 
				{
					mode=5;
					ok_cnt++;
				}
				else if(strncmp(buff,"+OK",3)==0 && ok_cnt==5) 
				{
					WT_GetIP(buff);
					mode=6;
					WT_Config_Ipaddr_Save();
					ok_cnt++;
				}
				else if(strncmp(buff,"+OK",3)==0 && ok_cnt==6) 
				{
					WT_GetMAC(buff);
					mode=7;
					WT_Config_MAC_Save();
					ok_cnt++;
				}
				else if(strncmp(buff,"+OK",3)==0 && ok_cnt==7) 
				{
					sock_Id = WT_GetSockID(buff);
					mode=8;
					ok_cnt++;
				}
				else if(strncmp(buff,"+OK",3)==0 && ok_cnt==8) 
				{
					mode=9;
					ok_cnt++;
				}
				else if(strncmp(buff,"+OK",3)==0 && ok_cnt==9) 
				{
					ok_cnt=0;
					return 0;
				}
				else mode=10;
				
				//clear rx buffer
				UART_24GHz_Buffer_RxClear();
				
				//send new command
				switch(mode)
				{					
					case 1:	//配置KEY
						memset(cmd,0,50);
						sprintf(cmd,"AT+KEY=1,0,\"%s\"\n",(char *)WT_Config.SecKey);
						WT_UART_24GHz_WrBuf((uint8_t *)cmd, strlen(cmd));
						break;
					case 2:	//KEY OK
						WT_UART_24GHz_WrBuf((uint8_t *)"AT+PMTF\n", 8);
						for(i=0;i<300;i++) //wait for receive first data
						{
							osDelay(3);
							if(UART_24GHz_Cnt_Buffer_Rx != 0) break;
						}
						break;
					case 3:	//write flash OK
						WT_UART_24GHz_WrBuf((uint8_t *)"AT+Z\n", 5);
						for(i=0;i<500;i++) //reboot
						{
							osDelay(3);
							//if(UART_24GHz_Cnt_Buffer_Rx != 0) break;
						}
						break;
					case 4:	//reboot OK
						WT_UART_24GHz_WrBuf((uint8_t *)"AT+WJOIN\n", 9);
						for(i=0;i<300;i++) //wait for receive first data
						{
							osDelay(3);
							if(UART_24GHz_Cnt_Buffer_Rx != 0) break;
						}
						break;
					case 5:	//查询IP
						osDelay(TIME_OUT);
						WT_UART_24GHz_WrBuf((uint8_t *)"AT+LKSTT=?\n", 11);
//						for(i=0;i<300;i++) //wait for receive first data
//						{
//							osDelay(3);
//							if(UART_24GHz_Cnt_Buffer_Rx != 0) break;
//						}
						break;
					case 6:	//查询MAC
						WT_UART_24GHz_WrBuf((uint8_t *)"AT+QMAC=?\n", 10);
						for(i=0;i<20;i++) //wait for receive first data
						{
							osDelay(3);
							if(UART_24GHz_Cnt_Buffer_Rx != 0) break;
						}
						break;
					case 7:	//join OK
						memset(cmd,0,50);
						sprintf(cmd,"AT+SKCT=0,0,%s,%d,%d\n",serv_ip,WT_Config.TCP_Port,WT_Config.TCP_Port+WT_Config.Dev_ID);
						WT_UART_24GHz_WrBuf((uint8_t *)cmd, strlen(cmd));
						//osDelay(1000);
						for(i=0;i<300;i++) //wait for receive first data
						{
							osDelay(3);
							if(UART_24GHz_Cnt_Buffer_Rx != 0) break;
						}
						break;
					case 8:	//设置系统默认发送的socket
						memset(cmd,0,50);			
						sprintf(cmd,"AT+SKSDF=%d\n",sock_Id);
						WT_UART_24GHz_WrBuf((uint8_t *)cmd, strlen(cmd));
						for(i=0;i<100;i++) //wait for receive first data
						{
							osDelay(3);
							if(UART_24GHz_Cnt_Buffer_Rx != 0) break;
						}
						break;
					case 9:	//设置透传模式
						WT_UART_24GHz_WrBuf((uint8_t *)"AT+ENTM\r\n", 9);
						//WT_UART_24GHz_WrBuf((uint8_t *)"AT+SKSTT=1\r\n", 12);
						break;
					case 10:	//重新连接
						memset(cmd,0,50);
						sprintf(cmd,"AT+SSID=\"%s\"\n",(char *)WT_Config.NetWork);
						WT_UART_24GHz_WrBuf((uint8_t *)cmd, strlen(cmd));
						ok_cnt = 0;
						break;
					default:	
						ok_cnt = 0;				
						break;
				}

			}
			else//一直接收到数据，硬件问题、一直发送数据
			{
				UART_24GHz_Buffer_RxClear();
			}
		}
		osDelay(20);
	}
}


/**
  * @brief  Uart24G_RX_Process
  * @param  None
  * @retval None
  */
static void Uart24G_RX_Process(void)
{
	uint16_t i;
	uint8_t mode=0;
	//char buff[320];
	char cmd[50];
	
	//memset(buff,0,320);
	uint8_t *WIFI_RECBUFF = (uint8_t *)malloc(1024);
	memset(WIFI_RECBUFF,0,1024);
	memset(cmd,0,50);

	last_rec = UART_24GHz_Cnt_Buffer_Rx;
	
	for(i=0;i<50;i++)
	{
		osDelay(30);
		if(last_rec == UART_24GHz_Cnt_Buffer_Rx) break;
		else last_rec = UART_24GHz_Cnt_Buffer_Rx;
	}
	if(i < 50) 
	{
		last_rec =0 ;
		//strncpy(buff,(char *)UART_24GHz_Buffer_Rx,strlen((char *)UART_24GHz_Buffer_Rx));
		//strcpy(buff,(char *)UART_24GHz_Buffer_Rx);
		for(i=0;i<UART_24GHz_Cnt_Buffer_Rx;i++)
		{
			WIFI_RECBUFF[i] = UART_24GHz_Buffer_Rx[i];
		}
		
		if(	(UART_24GHz_Buffer_Rx[0] == 0x4C) && (UART_24GHz_Buffer_Rx[1] == 0x5A) && (UART_24GHz_Buffer_Rx[2] == 0x59) 
			   && (UART_24GHz_Buffer_Rx[3] == 0x30) )
		{
			if( WIFI_RECBUFF[7] == 0x01) mode=1;
			else if(WIFI_RECBUFF[7] == 0x02) mode=2;
			else if(WIFI_RECBUFF[7] == 0x11) mode=3;
			else if(WIFI_RECBUFF[7] == 0x12) mode=4;
			else if(WIFI_RECBUFF[7] == 0x20) mode=5;
			else if(WIFI_RECBUFF[7] == 0x21) mode=6;
			else if(WIFI_RECBUFF[7] == 0x22) mode=7;
			else if(WIFI_RECBUFF[7] == 0x23) mode=8;
			else if(WIFI_RECBUFF[7] == 0x31) mode=9;
			else if(WIFI_RECBUFF[7] == 0x32) mode=10;
			else if(WIFI_RECBUFF[7] == 0x33) mode=11;
			else if(WIFI_RECBUFF[7] == 0x40) mode=12;
			else if(WIFI_RECBUFF[7] == 0x41) mode=13;
			else if(WIFI_RECBUFF[7] == 0x48) mode=14;
			else if(WIFI_RECBUFF[8] == 0x49) mode=15;
			else if(WIFI_RECBUFF[7] == 0x4A) mode=16;
			else if(WIFI_RECBUFF[7] == 0x4B) mode=17;
			else if(WIFI_RECBUFF[7] == 0x4C) mode=18;
			else if(WIFI_RECBUFF[7] == 0x4D) mode=19;
			else mode=20;
		}
		else mode=100;
		
		UART_24GHz_Buffer_RxClear();
		switch(mode)
		{
			
			case 1:	//心跳同步
				Uart24G_HeartBeat((char *)WIFI_RECBUFF);
				break;
			case 2:	//主动上报测试结果				
				osDelay(200);
				break;
			case 3:	//查询设备状态
				Uart24GHz_UploadDevStat((char *)WIFI_RECBUFF);
				break;
			case 4:	//查询当前测试状态
				Uart24GHz_UploadTestStat((char *)WIFI_RECBUFF);
				break;
			case 5:	//查询设备信息				
				Uart24G_UploadDevInfo((char *)WIFI_RECBUFF);
				break;
			case 6:	//查询系统设置
				Uart24G_UploadSysconfig((char *)WIFI_RECBUFF);
				break;
			case 7:	//查询网络设置
				Uart24G_UploadNetconfig((char *)WIFI_RECBUFF);
				break;
			case 8:	//查询RTC信息
				Uart24G_UploadRTC((char *)WIFI_RECBUFF);
				break;
			case 9:	//更新系统设置
				Uart24G_UpdateSysconfig((char *)WIFI_RECBUFF);
				break;
			case 10://更新网络设置
				break;
		  case 11://更新RTC信息
				Uart24G_UpdateRTC((char *)WIFI_RECBUFF);
				break;
			case 12://查询测试文件夹信息
				Uart24G_UploadTestFolder((char *)WIFI_RECBUFF);
				break;
			case 13://查询测试文件信息
				Uart24G_UploadTestFile((char *)WIFI_RECBUFF);
				break;
			case 14://下载文件（开始）
				Uart24G_Download_Start((char *)WIFI_RECBUFF);
				break;
			case 15://下载文件（传输）
				Uart24G_Download_Process((char *)WIFI_RECBUFF);
				break;
			case 16://下载文件（完成）
				Uart24G_Download_End((char *)WIFI_RECBUFF);
				break;
			case 17://上传文件（开始）
				break;
			case 18://上传文件（传输）
				break;
			case 19://上传文件（完成）
				break;
			default:	
				ok_cnt = 0;				
				break;
		}

	}
	else UART_24GHz_Buffer_RxClear();
	free(WIFI_RECBUFF);
}

/**
  * @brief  Uart24GHz_HeatBeat
  * @param  None
  * @retval None
  */
static void Uart24G_HeartBeat(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[20];
	
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 12;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x01;	//7,心跳命令
	//for(i=0;i<2;i++) TxBuffer[j++] = 0x00;	//8_9,保留字节	
	TxBuffer[j++] = Uart24G_GetDevStat();	//测试仪工作状态	
	TxBuffer[j++] = 0x00; //测试仪故障  0-正常 其它-故障

	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
}

/**
  * @brief  Uart24GHz_GetDevStat
  * @param  None
  * @retval 0-待机 1-线束测试 2-线束自学习 3-设备自检 4-标签打印 5-其它
  */
static uint8_t Uart24G_GetDevStat()
{
	uint8_t res=0;
	if(TestFile.task_status == 0 && StudyFile.task_status == 0 && SelfCheckItem.state == 0)//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
	{
		res = 0;
	}
	else if(TestFile.task_status != 0)//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
	{
		res = 1;
	}
	else if(StudyFile.task_status == 1)	//0:waiting study command, 1:studying, 2:study ok, 3:study error, 4:optimize study file
	{
		res = 2;
	}
	else if(SelfCheckItem.state == 1)	//0:waiting, 1:testing
	{
		res = 3;
	}
	else if(PrintFile.print_status == 3)	//0-stop 1-wait, 2-print ok, 3-print run, 4-print error
	{
		res =4;
	}
	else res = 5;
	return res;
	
}

/**
  * @brief  Uart24GHz_UploadDevStat
  * @param  None
  * @retval None
  */
static void Uart24GHz_UploadDevStat(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[20];
	
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 16;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x11;	//7,查询设备状态命令
	TxBuffer[j++] = Uart24G_GetDevStat();	//测试仪工作状态	
	TxBuffer[j++] = 0x00; //测试仪故障  0-正常 其它-故障
	for(i=0;i<4;i++) TxBuffer[j++] = 0x00;	//10_13,保留字节	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}

/**
  * @brief  Uart24GHz_UploadTestRes
  * @param  None
  * @retval None
  */
static void Uart24GHz_UploadTestRes(void)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[20];
	
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = 0;	//4_5,帧序号
	TxBuffer[j++] = 0;	
	TxBuffer[j++] = 15;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x02;	//7,发送测试结果命令
	if(TestFile.task_status == 3)//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
	{
		TxBuffer[j++] = 0x00;	//8,测试结果：0x00-测试成功，0x01-测试失败；
	}
	else TxBuffer[j++] = 0x01;
	TxBuffer[j++] = TestFile.test_num >> 24;	//9_12,测试计数器，4字节，高字节在前	
	TxBuffer[j++] = (TestFile.test_num >> 16) & 0xFF;	
	TxBuffer[j++] = (TestFile.test_num >> 8) & 0xFF;	
	TxBuffer[j++] = (TestFile.test_num >> 0) & 0xFF;	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}

/**
  * @brief  Uart24GHz_UploadTestStat
  * @param  None
  * @retval None
  */
static void Uart24GHz_UploadTestStat(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[220];
	memset(TxBuffer,0,220);
	
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 214;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x12;	//7,查询当前测试命令
	for(i=0;i<55;i++) TxBuffer[j++] = TestFile.FileName[i];	//8_207, 200字节，分隔符’|’，包括：文件名称；
	TxBuffer[j++] = '|';
	for(i=0;i<20;i++) TxBuffer[j++] = TestFile.FileHeader_DrawingNumber[i];  //图纸编号
	TxBuffer[j++] = '|';
	for(i=0;i<20;i++) TxBuffer[j++] = TestFile.FileHeader_CreatDate[i];  //制作日期
	TxBuffer[j++] = '|';
	for(i=0;i<20;i++) TxBuffer[j++] = TestFile.FileHeader_ProductName[i];  //产品名称
	TxBuffer[j++] = '|';
	for(i=0;i<20;i++) TxBuffer[j++] = TestFile.FileHeader_ProductModel[i];  //产品型号
	TxBuffer[j++] = '|';
	for(i=0;i<20;i++) TxBuffer[j++] = TestFile.FileHeader_ProductCompany[i];  //产品客户
	TxBuffer[j++] = '|';
	for(i=0;i<39;i++) TxBuffer[j++] = 0x00;  //未使用字节为0
	
	TxBuffer[j++] = TestFile.test_num >> 24;	//208_211,测试计数器，4字节，高字节在前	
	TxBuffer[j++] = (TestFile.test_num >> 16) & 0xFF;	
	TxBuffer[j++] = (TestFile.test_num >> 8) & 0xFF;	
	TxBuffer[j++] = (TestFile.test_num >> 0) & 0xFF;	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}



/**
  * @brief  Uart24GHz_UploadDevInfo
  * @param  None
  * @retval None
  */
static void Uart24G_UploadDevInfo(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[45];
	uint8_t  Devid[20];
	
	memset(Devid,0,20);
	itoa(WT_Config.Dev_ID,(char *)Devid,10);
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 41;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x20;	//7,查询设备信息命令
	for(i=0;i<20;i++) TxBuffer[j++] = Devid[i];	//8_26,产品序列号	
	#ifdef HD_VERSION1
	TxBuffer[j++] = 0x01;	//28_29,硬件版本号
	#else
	TxBuffer[j++] = 0x02;	//28_29,硬件版本号
	#endif
	TxBuffer[j++] = 0x00;	
	TxBuffer[j++] = DEF_WT_VERSION_SUB2;	//30_31,软件版本号
	TxBuffer[j++] = DEF_WT_VERSION_RC;	
	TxBuffer[j++] = WT_Config.MAC_Addr[0];//32_37,网卡MAC地址
	TxBuffer[j++] = WT_Config.MAC_Addr[1];
	TxBuffer[j++] = WT_Config.MAC_Addr[2];
	TxBuffer[j++] = WT_Config.MAC_Addr[3];
	TxBuffer[j++] = WT_Config.MAC_Addr[4];
	TxBuffer[j++] = WT_Config.MAC_Addr[5];
	TxBuffer[j++] = SelfCheckItem.port_board_number;//38,板卡状态
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}


/**
  * @brief  Uart24GHz_UploadSysconfig
  * @param  None
  * @retval None
  */
static void Uart24G_UploadSysconfig(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[20];
	
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 18;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x21;	//7,查询系统设置命令
	TxBuffer[j++] = WT_Config.BackLight;	//8,查询背光亮度
	TxBuffer[j++] = WT_Config.AudioVolume;	//9,查询喇叭音量
	TxBuffer[j++] = WT_Config.AudioOK;	//10,OK语音编号
	TxBuffer[j++] = WT_Config.AudioNG;	//11,NG语音编号
	TxBuffer[j++] = WT_Config.Print;	//12,语言设置
	TxBuffer[j++] = WT_Config.Dev_ID;	//13,工位号
	for(i=0;i<2;i++) TxBuffer[j++] = 0x00;	//14_15,保留字节	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}

/**
  * @brief  Uart24GHz_UploadSysconfig
  * @param  None
  * @retval None
  */
static void Uart24G_UploadNetconfig(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[64];
	
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 60;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x22;	//7,查询网络设置命令
	for(i=0;i<10;i++) TxBuffer[j++] = WT_Config.NetWork[i];	//8_27,网络SSID	
	for(i=0;i<10;i++) TxBuffer[j++] = 0x00;
	for(i=0;i<10;i++) TxBuffer[j++] = WT_Config.SecKey[i];	//28_47,网络秘钥	
	for(i=0;i<10;i++) TxBuffer[j++] = 0x00;
	for(i=0;i<4;i++) TxBuffer[j++] = WT_Config.Server_IP[i];	//48_51,服务器IP地址	
	TxBuffer[j++] = (WT_Config.TCP_Port >> 8)&0xFF;//服务器端口号52_53
	TxBuffer[j++] = WT_Config.TCP_Port&0xFF;//服务器端口号
	for(i=0;i<4;i++) TxBuffer[j++] = 0x00;	//54_57,保留字节	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}


/**
  * @brief  Uart24GHz_UploadRTC
  * @param  None
  * @retval None
  */
static void Uart24G_UploadRTC(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[20];
	
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 20;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x23;	//7,查询RTC命令
	TxBuffer[j++] = WT_Config.InfoRTC[0];	//8,年（2000年-2255年）
	TxBuffer[j++] = WT_Config.InfoRTC[1];	//9,月
	TxBuffer[j++] = WT_Config.InfoRTC[2];	//10,日
	TxBuffer[j++] = WT_Config.InfoRTC[3];	//11,时
	TxBuffer[j++] = WT_Config.InfoRTC[4];	//12,分
	TxBuffer[j++] = WT_Config.InfoRTC[5];	//13,秒
	TxBuffer[j++] = WT_Config.InfoRTC[6];	//14,星期
	for(i=0;i<3;i++) TxBuffer[j++] = 0x00;	//15_17,保留字节	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}


/**
  * @brief  Uart24GHz_UploadTestFolder
  * @param  None
  * @retval None
  */
static void Uart24G_UploadTestFolder(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[20];
	
	WT_TestFolder_Init();
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 18;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x40;	//7,查询测试文件夹信息
	TxBuffer[j++] = 0x00;	//8_11,查询测试文件数量
	TxBuffer[j++] = 0x00;
	TxBuffer[j++] = 0x00;
	TxBuffer[j++] = TestFolder.number_TotalFile;
	for(i=0;i<4;i++) TxBuffer[j++] = 0x00;	//12_15,保留字节	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}

/**
  * @brief  Uart24GHz_UploadTestFile
  * @param  None
  * @retval None
  */
static void Uart24G_UploadTestFile(char *RxBuff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[210];
	uint32_t index=0;
	
	index = (RxBuff[8]>>24) + (RxBuff[9]>>16) + (RxBuff[10]>>8) + RxBuff[11];
	
	WT_TestItem_Init((char *)TestFolder.FilesName[index]);
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = RxBuff[4];	//4_5,帧序号
	TxBuffer[j++] = RxBuff[5];	
	TxBuffer[j++] = 210;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x41;	//7,查询测试文件信息
	for(i=0;i<55;i++) TxBuffer[j++] = TestFile.FileHeader_FileName[i];	//8_207, 200字节，分隔符’|’，包括：文件名称；
	TxBuffer[j++] = '|';
	for(i=0;i<20;i++) TxBuffer[j++] = TestFile.FileHeader_DrawingNumber[i];  //图纸编号
	TxBuffer[j++] = '|';
	for(i=0;i<20;i++) TxBuffer[j++] = TestFile.FileHeader_CreatDate[i];  //制作日期
	TxBuffer[j++] = '|';
	for(i=0;i<20;i++) TxBuffer[j++] = TestFile.FileHeader_ProductName[i];  //产品名称
	TxBuffer[j++] = '|';
	for(i=0;i<20;i++) TxBuffer[j++] = TestFile.FileHeader_ProductModel[i];  //产品型号
	TxBuffer[j++] = '|';
	for(i=0;i<20;i++) TxBuffer[j++] = TestFile.FileHeader_ProductCompany[i];  //产品客户
	TxBuffer[j++] = '|';
	for(i=0;i<39;i++) TxBuffer[j++] = 0x00;  //未使用字节为0
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}


/**
  * @brief  Uart24GHz_UpdateSysconfig
  * @param  None
  * @retval None
  */
static void Uart24G_UpdateSysconfig(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[20];
	
	WT_Config.BackLight = buff[8];
	WT_Config.AudioVolume = buff[9];
	WT_Config.AudioOK = buff[10];
	WT_Config.AudioNG = buff[11];
	WT_Config.Print = buff[12];
	
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = 0x00;	//4_5,帧序号
	TxBuffer[j++] = 0x00;	
	TxBuffer[j++] = 12;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x31;	//7,查询系统设置命令
	TxBuffer[j++] = 0x00;	//8,主机接收成功
	TxBuffer[j++] = 0x00;	//9,保留字节	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}

/**
  * @brief  Uart24GHz_UpdateRTC
  * @param  None
  * @retval None
  */
static void Uart24G_UpdateRTC(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[20];
	
	WT_Config.InfoRTC[0] = buff[8];	//8,年（2000年-2255年）
	WT_Config.InfoRTC[1] = buff[9];	//9,月
	WT_Config.InfoRTC[2] = buff[10];	//10,日
	WT_Config.InfoRTC[3] = buff[11];	//11,时
	WT_Config.InfoRTC[4] = buff[12];	//12,分
	WT_Config.InfoRTC[5] = buff[13];	//13,秒
	WT_Config.InfoRTC[6] = buff[14];	//14,星期
	
	WT_Config_Save();
	
	set_systemtime(buff[13],buff[12],buff[11],buff[10],buff[9],(buff[8]+2000-2014) );
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = 0x00;	//4_5,帧序号
	TxBuffer[j++] = 0x00;	
	TxBuffer[j++] = 20;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x33;	//7,更新RTC信息命令
	TxBuffer[j++] = 0x00;	//8,主机接收成功
	TxBuffer[j++] = 0x00;	//9,保留字节	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
}


/**
  * @brief  Uart24GHz_Download_start
  * @param  None
  * @retval 0--oK  1--error
  */
static uint8_t Uart24G_Download_Start(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t res;
	uint8_t  TxBuffer[20];
	uint8_t  str_name[30];
	
	
//	frame_cnt = (buff[38]<<8) + buff[39];
	memset(filename,0,50);
	memset(str_name,0,30);
	
	if(k_StorageGetStatus(USB_DISK_UNIT) == 1 && k_StorageGetStatus(MSD_DISK_UNIT) == 0) store_dev = 0;
	else if(k_StorageGetStatus(MSD_DISK_UNIT) == 1 && k_StorageGetStatus(USB_DISK_UNIT) == 0) store_dev = 1;
	else store_dev = 1;
	
	if(store_dev == 0)//usb
	{
		if(sizeof(path_studyfile) < 50) strcpy ((char*)filename,(char *)path_studyfile);
		else														strcpy ((char*)filename,(char *)path_Default);
	}
	if(store_dev == 1)//sd
	{
		if(sizeof(path_studyfile) < 50) strcpy ((char*)filename,(char *)path_studyfile_sd);
		else														strcpy ((char*)filename,(char *)path_Default_sd);
	}
	strcat ((char*)filename,"/");
	
	for(i=0;i<30;i++)
	{
		str_name[i]=buff[8+i];
	}
	strcat ((char*)filename,(char*)str_name);
	
	res = WT_TestFiles_Create(filename);
	if(res != 0) return 1;
	
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 12;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x48;	//7,下载文件开始命令
	TxBuffer[j++] = 0x00;	//8,主机接收成功
	TxBuffer[j++] = 0x00;	//9,保留字节	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	
	return 0;
}

/**
  * @brief  Uart24GHz_Download_process
  * @param  None
  * @retval None
  */
static void Uart24G_Download_Process(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	//uint16_t frame_idx=0;
	//uint16_t buf16;
	uint16_t len;
	uint8_t res=0;
	uint8_t  TxBuffer[20];
	//uint8_t  Rxbuff[320];
	
	//memset(Rxbuff,0,320);
	//memset(WIFI_TMPBUFF,0,1024);
	uint8_t *WIFI_RECDATA = (uint8_t *)malloc(1024*800);
	memset(WIFI_RECDATA,0,1024);
	
	len = (buff[11] << 8) + buff[12];
//	/* 比较CRC-16校验值 */
//	buf16 = (UART_24GHz_Buffer_Rx[len+11]<<8)+UART_24GHz_Buffer_Rx[len+12]; //获取CRC-16值
//	if(Data_CRC16_CCITT(UART_24GHz_Buffer_Rx, len + 10) != buf16) //校验数据的完整性,0x00-主机接收成功，0x01-数据格式错误；
//	{
//		res = 0x01;
//	}
//	else res = 0x00;
	
	//get data
	
//	for(i=0;i<len;i++)
//	{
//		WIFI_RECDATA[str_size] = buff[13+i];
//		str_size++;
//	}
	for(i=0;i<len;i++)
	{
		WIFI_RECDATA[i] = buff[13+i];
	}
	if(strlen((char *)filename) != 0)
	{
		WT_TestFiles_Write(filename,WIFI_RECDATA);//添加数据到文件
	}
	//frame_idx = (buff[8]<<8) + buff[9];
	
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 12;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x49;	//7,下载文件传输命令
	TxBuffer[j++] = res;	//8,0x00-主机接收成功，0x01-数据格式错误；
	TxBuffer[j++] = 0x00;	//9,保留字节	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	wt_SetText_Status("文件下载中！");
	free(WIFI_RECDATA);
}

/**
  * @brief  Uart24GHz_Download_process
  * @param  None
  * @retval None
  */
static void Uart24G_Download_End(char *buff)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[20];
	
//	if(strlen((char *)filename) != 0)
//	{
//		WT_TestFiles_Write(filename,WIFI_RECDATA);//添加数据到文件
//	}
//	str_size = 0;
	//return data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 'L';	//Logo
	TxBuffer[j++] = 'Z';
	TxBuffer[j++] = 'Y';
	TxBuffer[j++] = '1';
	TxBuffer[j++] = buff[4];	//4_5,帧序号
	TxBuffer[j++] = buff[5];	
	TxBuffer[j++] = 12;	//6,帧长度
	//---------------------------------------
	TxBuffer[j++] = 0x4A;	//7,下载文件结束命令
	TxBuffer[j++] = 0x00;	//8,主机接收成功
	TxBuffer[j++] = 0x00;	//9,保留字节	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[6]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_24GHz_WrBuf(TxBuffer, TxBuffer[6]);
	wt_SetText_Status("文件下载完成！");
	
}
