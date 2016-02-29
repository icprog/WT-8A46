/*
******************************************************************************
* @progect LZY Wire Cube Tester
* @file    wt_uart_COM1.c
* @author  LZY Zhang Manxin
* @version V1.0.0
* @date    2014-12-09
* @brief   ...
******************************************************************************
*/

/* Define ------------------------------------------------------------------*/
#define  WT_UART_COM1_GLOBALS


/* Includes ------------------------------------------------------------------*/
#include "k_bsp.h"
#include "main.h"


/* Private functions ---------------------------------------------------------*/
static void WT_UART_COM1_Rx_IT(uint8_t dat);
static void UART_COM1_Buffer_RxClear(void);
static void UartCOM1_RX_Process(void);

extern void get_system_time(char * date ,char * time);
/**
  * @brief  wire self check task
  * @param  argument: pointer that is passed to the thread function as start argument.
  * @retval None
  */
void UARTCOM1Thread(void const * argument)
{
	osEvent event;
	
	WT_UART_COM1_Init();	//Init
	UART_COM1_Buffer_RxClear();
	
  for( ;; )
  {
		//clear message
		while(1)
		{
			event = osMessageGet( UartCOM1Event, 0);
			if(event.status != osEventMessage) break;
		}
		//wait new message
    event = osMessageGet(UartCOM1Event, osWaitForever );
    
    if( event.status == osEventMessage )
    {
      switch(event.value.v)
      {
				case UartCOM1_RX_Event:
					UartCOM1_RX_Process();
					break;
				
				case UartCOM1_TX_Event:		
					UartCOM1_Send_StartPrint_Cmd();		
					break;
				
				default:
					break;
      }
    }
		else	//error
		{
			osDelay(500);
		}
  }
}


/**
  * @brief  WT_UART_COM1_Init
  * @param  None
  * @retval None
  */
void WT_UART_COM1_Init(void)
{
	uint32_t i;
	
	for(i=0;i<UART_COM1_RX_Bufer_Length;i++) UART_COM1_Buffer_Rx[i]=0;
	UART_COM1_Cnt_Buffer_Rx = 0;

  UartHandle_COM1.Instance        = UART_COM1;
  UartHandle_COM1.Init.BaudRate   = 375000;
  UartHandle_COM1.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle_COM1.Init.StopBits   = UART_STOPBITS_1;
  UartHandle_COM1.Init.Parity     = UART_PARITY_NONE;
  UartHandle_COM1.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle_COM1.Init.Mode       = UART_MODE_TX_RX;
  HAL_UART_Init(&UartHandle_COM1);	
}


/**
  * @brief  This function handles UART interrupt request.
  * @param  huart: UART handle
  * @retval None
  */
void WT_UART_COM1_IRQHandler(UART_HandleTypeDef *huart)
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
		WT_UART_COM1_Rx_IT((uint8_t)(huart->Instance->DR & (uint8_t)0x00FF));	//cndz, 20140723
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
  * @brief  UART_COM1_Buffer_RxClear
  * @param  None
  * @retval None
  */
static void UART_COM1_Buffer_RxClear(void)
{
	uint32_t i;
	
	for(i=0;i<UART_COM1_RX_Bufer_Length;i++) UART_COM1_Buffer_Rx[i]=0;
	UART_COM1_Cnt_Buffer_Rx = 0;
//	Is_UART_COM1_Rx_Come = 0;
}

/**
  * @brief  WT_UART_COM1_Rx
  * @param  uint8_t dat
  * @retval None
  */
uint8_t WT_UART_COM1_WrBuf(uint8_t* pData, uint8_t length)
{
	while(length--)
	{
		if(UART_WaitOnFlagUntilTimeout(&UartHandle_COM1, UART_FLAG_TXE, RESET, 100) != HAL_OK)
		{
			return 1;
		}
		UartHandle_COM1.Instance->DR = (*pData++ & (uint8_t)0xFF);
	}
	
	if(UART_WaitOnFlagUntilTimeout(&UartHandle_COM1, UART_FLAG_TXE, RESET, 100) != HAL_OK)
	{
		return 1;
	}
	
	return 0;
}

/**************************************************/
/*** Aplication ***********************************/
/**************************************************/

/**
  * @brief  WT_UART_COM1_Rx_IT
  * @param  uint8_t dat
  * @retval None
  */
static void WT_UART_COM1_Rx_IT(uint8_t dat)
{
//	uint8_t buf8=0;
	
	/* receiver data */
	UART_COM1_Buffer_Rx[UART_COM1_Cnt_Buffer_Rx] = dat;
	if(UART_COM1_Cnt_Buffer_Rx < (UART_COM1_RX_Bufer_Length-2)) UART_COM1_Cnt_Buffer_Rx++;
	
	//检测桢头标志
	if(UART_COM1_Cnt_Buffer_Rx == 1)
	{
		if(UART_COM1_Buffer_Rx[0] != 0x7e ) 
		{
			UART_COM1_Buffer_RxClear();
			return;
		}
	}
	else if(UART_COM1_Cnt_Buffer_Rx <= 4) //通信地址、帧格式
	{
		//Byte[1] -	帧长度
		//Byte[2]	-	功能
		//Byte[3]	-	参数
		//Byte[4]	-	累加和
		if(UART_COM1_Cnt_Buffer_Rx == 2)
		{
			if((UART_COM1_Buffer_Rx[1] < 4) |													//长度太短
				 (UART_COM1_Buffer_Rx[1] > UART_COM1_RX_Bufer_Length))	//长度太长
			{
				UART_COM1_Buffer_RxClear();
				return;
			}
		}
	}
	else //wait frame end
	{
		if(UART_COM1_Cnt_Buffer_Rx >= UART_COM1_Buffer_Rx[1])
		{
			//Is_UART_COM1_Rx_Come = 1;
			osMessagePut(UartCOM1Event, UartCOM1_RX_Event, 0);	//收到数据帧
			//UartCOM1_RX_Process();
		}
	}
}


/**
  * @brief  UartCOM1_HeatBeat
  * @param  None
  * @retval None
  */
static void UartCOM1_QueryInfo_Status(void)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[4];
	
	//send data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 0x7e;
	TxBuffer[j++] = 0x04;	//帧长度
	TxBuffer[j++] = 0x04;	//功能，查询打印机信息

	/****** 累加和校验 ****************************************************/
	i = Data_CheckSum(TxBuffer, TxBuffer[1]-1); //计算出CheckSum
	TxBuffer[j++] = i&0xFF;

	/****** Send ***********************************************************/
	WT_UART_COM1_WrBuf(TxBuffer, TxBuffer[1]);
}

/**
  * @brief  Update_Printtime
  * @param  None
  * @retval None
  */
static void Update_Print_Time()
{
	uint16_t i;
	char date_replace[20];
	char time_replace[20];
	
	memset(date_replace,0,20);
	memset(time_replace,0,20);
	get_system_time(date_replace,time_replace);
	for(i=0;i<8;i++)
	{
		PrintFile.PrintFilestr[76+i] = time_replace[i];
	}
  
	
	
}

/**
  * @brief  UartCOM1_Send_StartPrint_Cmd
  * @param  None
  * @retval None
  */
static void UartCOM1_Send_StartPrint_Cmd(void)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[7];
	uint16_t sendbytes=0;
	
	sendbytes = (PrintFile.sum_str/100) + 1;
	
	//send data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 0x7e;
	TxBuffer[j++] = 0x07;	//帧长度
	TxBuffer[j++] = 0x01;	//功能，开始发送打印内容命令
	TxBuffer[j++] = (sendbytes>>8)&0xFF;//打印内容行数
	TxBuffer[j++] = sendbytes & 0xFF;
	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[1]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_COM1_WrBuf(TxBuffer, TxBuffer[1]);
	Update_Print_Time();
}

/**
  * @brief  UartCOM1_Send_Print_Cmd
  * @param  None
  * @retval None
  */
static void UartCOM1_Send_Print_Cmd(uint16_t cmd_line)
{
	uint16_t i;
	uint16_t j=0;
	uint8_t TxBuffer[108];
	uint16_t line=0;
	uint16_t frame_num = 0;
	
	memset(TxBuffer,0,100);
	frame_num = cmd_line + 1;

	//send data to Printer
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 0x7e;
	TxBuffer[j++] = 108;	//帧长度
	TxBuffer[j++] = 0x02;	//功能，发送打印内容命令
	TxBuffer[j++] = (frame_num>>8)&0xFF; //帧号
	TxBuffer[j++] = (frame_num)&0xFF;    //帧号
	TxBuffer[j++] = 100;//打印行字符串长度

	for(i=0;i<100;i++)
	{
		line = 100*cmd_line +i;
		if(line < PrintFile.sum_str)
		{
			TxBuffer[j++] = PrintFile.PrintFilestr[line];
		}
		else TxBuffer[j++] = 0;
		
	}
	
	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[1]-2); //计算出CRC-16
	TxBuffer[106] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[107] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_COM1_WrBuf(TxBuffer, TxBuffer[1]);
}

/**
  * @brief  UartCOM1_Send_Print_Cmd
  * @param  None
  * @retval None
  */
static void UartCOM1_Send_StopPrint_Cmd()
{
	uint16_t i;
	uint16_t j=0;
	uint8_t  TxBuffer[6];
	
	//send data to PC
	/****** 帧头 ****************************************************/
	TxBuffer[j++] = 0x7e;
	TxBuffer[j++] = 0x06;	//帧长度
	TxBuffer[j++] = 0x03;	//功能，结束发送打印内容命令
	TxBuffer[j++] = 0x5A; //发送文件结束

	/****** CRC-16 校验 ****************************************************/
	i = Data_CRC16_MOSBUS(TxBuffer, TxBuffer[1]-2); //计算出CRC-16
	TxBuffer[j++] = (i>>8)&0xFF; //CRC-16H
	TxBuffer[j++] = i&0xFF; //CRC-16L

	/****** Send ***********************************************************/
	WT_UART_COM1_WrBuf(TxBuffer, TxBuffer[1]);
}

/**
  * @brief  UartCOM1_RX_Process
  * @param  None
  * @retval None
  */
static void UartCOM1_RX_Process(void)
{
	static uint16_t cmdline;
	//Process
	switch(UART_COM1_Buffer_Rx[2])
	{
		case 0x01:	//开始发送打印命令返回值
			PrintFile.print_status = 3;//0-stop 1-wait, 2-print ok, 3-print run, 4-print error
			if(UART_COM1_Buffer_Rx[3]==0)//0-成功  开始发送打印命令
			{
				UartCOM1_Send_Print_Cmd(0);//发送第一行命令
				cmdline = 1;
			}
			break;
		
		case 0x02:	//发送打印内容返回值
			PrintFile.print_status = 3;//0-stop 1-wait, 2-print ok, 3-print run, 4-print error
			if(UART_COM1_Buffer_Rx[3]==0)//0-成功  开始发送下一条命令
			{
				if(cmdline < PrintFile.number_Cmdline)
				{
					UartCOM1_Send_Print_Cmd(cmdline);
					cmdline ++;
				}
				else if(cmdline == PrintFile.number_Cmdline)
				{
					UartCOM1_Send_StopPrint_Cmd();
				}
				else break;
			}	
			else //命令失败
			{
				PrintFile.print_status = 4;
			}
			break;
		
		case 0x03:	//结束发送打印内容命令返回值
			//UartCOM1_QueryInfo_Status();
			PrintFile.print_status = 2;//0-stop 1-wait, 2-print ok, 3-print run, 4-print error
			break;
				
		case 0x04:	//查询打印模块信息返回值
			UartCOM1_QueryInfo_Status();
			break;
		
		default:
			break;
	}
	
	// clear rx buffer
	UART_COM1_Buffer_RxClear();
}
