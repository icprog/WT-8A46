/**
  ******************************************************************************
  * @progect LZY Wire Cube Tester
	* @file    wt_bsp_ad.c
  * @author  LZY Zhang Manxin
  * @version V1.0.0
  * @date    2014-07-18
  * @brief   This file provides the E2PROM functions
  ******************************************************************************
  */
	
	
/* Includes ------------------------------------------------------------------*/
#include "stm32f429i_wt_eeprom.h"
#include "cmsis_os.h"
#include "main.h"
#include "k_rtc.h"



/** @defgroup EEPROM_Private_Variables
  * @{
  */
__IO uint16_t  EEPROMAddress = 0;


/**
  * @brief  Initializes peripherals used by the I2C EEPROM driver.
  * @param  None
  * 
  * @note   Then try to connect FM24C64_I2C_ADDRESS
	*
  * @retval EEPROM_OK (0) if operation is correctly performed, else return value 
  *         different from EEPROM_OK (0)
  */
uint8_t BSP_EEPROM_Init(void)
{ 
  /* I2C Initialization */
  EEPROM_IO_Init();

  /*Select the EEPROM address and check if OK*/
  EEPROMAddress = FM24C64_I2C_ADDRESS;
  if (EEPROM_IO_IsDeviceReady(EEPROMAddress, EEPROM_MAX_TRIALS) != HAL_OK) 
  {
    return EEPROM_FAIL;
  }
  return EEPROM_OK;
}

/**
  * @brief  Reads a block of data from the EEPROM.
  * @param  pBuffer : pointer to the buffer that receives the data read from 
  *         the EEPROM.
  * @param  ReadAddr : EEPROM's internal address to start reading from.
  * @param  NumByteToRead : pointer to the variable holding number of bytes to 
  *         be read from the EEPROM.
  * 
  *        @note The variable pointed by NumByteToRead is reset to 0 when all the 
  *              data are read from the EEPROM. Application should monitor this 
  *              variable in order know when the transfer is complete.
  * 
  * @retval EEPROM_OK (0) if operation is correctly performed, else return value 
  *         different from EEPROM_OK (0) or the timeout user callback.
  */
uint8_t BSP_EEPROM_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{  
  uint32_t buffersize = NumByteToRead;
  
  if (EEPROM_IO_ReadData(EEPROMAddress, ReadAddr, pBuffer, buffersize) != HAL_OK)
  {
    return EEPROM_FAIL;
  }
  
  /* If all operations OK, return EEPROM_OK (0) */
  return EEPROM_OK;
}



/**
  * @brief  Writes more than one byte to the EEPROM with a single WRITE cycle.
  *
  * @note   The number of bytes (combined to write start address) must not 
  *         cross the EEPROM page boundary. This function can only write into
  *         the boundaries of an EEPROM page.
  *         This function doesn't check on boundaries condition (in this driver 
  *         the function BSP_EEPROM_WriteBuffer() which calls BSP_EEPROM_WritePage() is 
  *         responsible of checking on Page boundaries).
  * 
  * @param  pBuffer : pointer to the buffer containing the data to be written to 
  *         the EEPROM.
  * @param  WriteAddr : EEPROM's internal address to write to.
  * @param  NumByteToWrite : pointer to the variable holding number of bytes to 
  *         be written into the EEPROM. 
  * 
  *        @note The variable pointed by NumByteToWrite is reset to 0 when all the 
  *              data are written to the EEPROM. Application should monitor this 
  *              variable in order know when the transfer is complete.
  * 
  * @note This function just configure the communication and enable the DMA 
  *       channel to transfer data. Meanwhile, the user application may perform 
  *       other tasks in parallel.
  * 
  * @retval EEPROM_OK (0) if operation is correctly performed, else return value 
  *         different from EEPROM_OK (0) or the timeout user callback.
  */
uint8_t BSP_EEPROM_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{ 
  uint32_t buffersize = NumByteToWrite;
  uint32_t status = EEPROM_OK;

  if (EEPROM_IO_WriteData(EEPROMAddress, WriteAddr, pBuffer, buffersize) != HAL_OK)
  {
    status = EEPROM_FAIL;
  }
  
  /* If all operations OK, return EEPROM_OK (0) */
  return status;
}

/**
  * @brief  Wait for EEPROM Standby state.
  * 
  * @note  This function allows to wait and check that EEPROM has finished the 
  *        last operation. It is mostly used after Write operation: after receiving
  *        the buffer to be written, the EEPROM may need additional time to actually
  *        perform the write operation. During this time, it doesn't answer to
  *        I2C packets addressed to it. Once the write operation is complete
  *        the EEPROM responds to its address.
  * 
  * @param  None
  * @retval EEPROM_OK (0) if operation is correctly performed, else return value 
  *         different from EEPROM_OK (0) or the timeout user callback.
  */
uint8_t BSP_EEPROM_WaitEepromStandbyState(void)      
{
  /* Check if the maximum allowed number of trials has bee reached */
  if (EEPROM_IO_IsDeviceReady(EEPROMAddress, EEPROM_MAX_TRIALS) != HAL_OK)
  {
    /* If the maximum number of trials has been reached, exit the function */
      BSP_EEPROM_TIMEOUT_UserCallback();
      return EEPROM_TIMEOUT;
  }
  return EEPROM_OK;
}

/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
__weak void BSP_EEPROM_TIMEOUT_UserCallback(void)
{
}




/**
  * @brief  WT测试程序
  * @param  None
  * @retval None
  */
//void WT_Test(void)
//{
//	uint8_t buffer[100];
//	uint32_t i;

//	
//	FM25V_IO_Init();
//	
//	for(i=0;i<100;i++) buffer[i]=i+1;
//	FM25V_IO_Write(buffer, 0, 100);
//	osDelay(1);
//	
//	for(i=0;i<100;i++) buffer[i]=0;
//	FM25V_IO_Read(buffer,  0, 100);
//	osDelay(1);
//	
//	for(i=0;i<100;i++) buffer[i]=0;
//	FM25V_IO_ReadID(buffer, 9);
//	osDelay(1);
//	
//	for(i=0;i<100;i++) buffer[i]=0;
//	FM25V_IO_ReadSN(buffer, 9);
//	osDelay(1);

//	
//	// Init
//	BSP_EEPROM_Init();
//	// Write
//	for(i=0;i<100;i++) buffer[i]=i;
//	BSP_EEPROM_WriteBuffer(buffer, 16, 50);
//	osDelay(1);
//	// Read
//	for(i=0;i<100;i++) buffer[i]=0;
//	BSP_EEPROM_ReadBuffer(buffer,  16, 50);
//	osDelay(1);
//}

/**
  * @brief  Read WT Config to the struct
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_Read(void)
{
	uint8_t i, res;
	uint8_t buf8[72];
	uint8_t *p;
	
	res = BSP_EEPROM_ReadBuffer(buf8, E2PROM_ADDR_WT_CONFIG, 46+6+1+10+1+2);
	
	if(res == 0)
	{
		if((buf8[0] > 10) || (buf8[1] > 30) || (buf8[2] > 1) || (buf8[3] < 14) || (buf8[3] > 100)) //非法配置
		{
			WT_Config_Restore();
			return 1;
		}
		else
		{
			p = (uint8_t *) &WT_Config;	
			for(i=0;i<66;i++)
			{
				*p++ = buf8[i];
			}
			return 0;
		}
	}
	else
	{
		WT_Config_Restore();
		return 1;
	}
}

/**
  * @brief  Save WT Config from the struct
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_Save(void)
{
	uint8_t res;
	uint8_t * p;
	
	p = (uint8_t *) &WT_Config;
	res = BSP_EEPROM_WriteBuffer(p, E2PROM_ADDR_WT_CONFIG, 46+6+1+10+1+2);
	if(res == 0) 	return 0;
	else					return 1;
}

/**
  * @brief  Restore WT Config
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_Restore(void)
{
	uint8_t res;
//	RTC_DateTypeDef RTC_Date;
//	RTC_TimeTypeDef RTC_Time;
	
	// Init
	WT_Config.BackLight = 10;		//背光亮度，0-10
	WT_Config.AudioVolume = 8;	//语音音量，0-10
	WT_Config.AudioOK = 0;      //OK声音
	WT_Config.AudioNG = 4;      //NG声音
	WT_Config.Print = 0;				//本地打印 0:no, 1:yes
	WT_Config.InfoRTC[0] = 15;	//年
	WT_Config.InfoRTC[1] = 1; 	//月
	WT_Config.InfoRTC[2] = 1; 	//日
	WT_Config.InfoRTC[3] = 12; 	//时
	WT_Config.InfoRTC[4] = 0; 	//分
	WT_Config.InfoRTC[5] = 0; 	//秒
	WT_Config.InfoRTC[6] = 4; 	//星期
	WT_Config.Mode_TestW = 0;		//导通阈值, 0:365R, 1:1k, 2:10k
	WT_Config.Mode_TestZ = 0;		//高阻阈值, 0:365R, 1:1k, 2:10k
	WT_Config.Mode_TestK = 1;		//开关测试模式, 0:27R, 1:365R,
	strcpy((char *)WT_Config.NetWork,"WT-8A46");//网络名称
	strcpy((char *)WT_Config.SecKey,"lzy123456");//密钥
//	memset(WT_Config.NetWork,0,10);		//网络名称
//	memset(WT_Config.SecKey,0,10);		//密钥
	WT_Config.Server_IP[0] = 192;	 //服务器IP地址
	WT_Config.Server_IP[1] = 168;	 //服务器IP地址
	WT_Config.Server_IP[2] = 1; 	 //服务器IP地址
	WT_Config.Server_IP[3] = 100;	 //服务器IP地址
	WT_Config.TCP_Port = 1000;	   //端口号
	WT_Config.Dev_ID = 1;	         //设备编号,	0-255
	WT_Config.TestMode = 0;     //0-自动模式  1-手动模式  2-定时模式
	WT_Config.StudyOptimizingMode = 0;	//0:full test, 1:only test Z bettwn max & min W point,
	WT_Config.Lockstat = 0;	    //0:unlock  1:lock
	WT_Config.TestRule = 0;	    //0:循环测试  1:单次测试
	//WT_Config.StudyBoardNumber = 2;			//1-4 board
	WT_Config.Passwd[0] = '1';
	WT_Config.Passwd[1] = '2';
	WT_Config.Passwd[2] = '3';
	WT_Config.Passwd[3] = '4';
	WT_Config.Passwd[4] = '5';
	WT_Config.Passwd[5] = '6';
	
	WT_Config.IP_Addr[0] = 0;
	WT_Config.IP_Addr[1] = 0;
	WT_Config.IP_Addr[2] = 0;
	WT_Config.IP_Addr[3] = 0;
	
	WT_Config.MAC_Addr[0] = 0;
	WT_Config.MAC_Addr[1] = 0;
	WT_Config.MAC_Addr[2] = 0;
	WT_Config.MAC_Addr[3] = 0;
	WT_Config.MAC_Addr[4] = 0;
	WT_Config.MAC_Addr[5] = 0;
	WT_Config.Mode_TestCap = 0; //电容阈值, 0:0.1uF, 1:0.47uF, 2:1.0uF, 3:4.7uF; 
	
	WT_Config.StudyMode = 0; //自学习模式, 0:导通, 1:导通+电阻
	WT_Config.Print_Mode = 0; //打印方式，0：并口打印 1：串口打印
	
	//Update
	LCD_BackLight_SetPWM(WT_Config.BackLight);
	
//	if(WT_Config.InfoRTC[0]<14) WT_Config.InfoRTC[0]=14;		
//	RTC_Date.Year = WT_Config.InfoRTC[0] - 14;
//	RTC_Date.Month = WT_Config.InfoRTC[1];
//	RTC_Date.Date = WT_Config.InfoRTC[2];
//	RTC_Date.WeekDay = WT_Config.InfoRTC[6];
//	RTC_Time.Hours = WT_Config.InfoRTC[3];
//	RTC_Time.Minutes = WT_Config.InfoRTC[4];
//	RTC_Time.Seconds = WT_Config.InfoRTC[5];
//	k_SetDate(&RTC_Date);
//	k_SetTime(&RTC_Time);
	
	//Save	
	res = WT_Config_Save();	// save
	return res;
}

/**
  * @brief  WT_Config_Lockstat_Read
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_Lockstat_Read(void)
{
	uint8_t res;
	uint8_t buf8[2];
	
	res = BSP_EEPROM_ReadBuffer(buf8, E2PROM_ADDR_WT_CONFIG + 45, 1);
	
	if(res == 0)
	{
		WT_Config.Lockstat = buf8[0];			//0:unlock  FE:lock
		WT_Config_Lockstat_Save();
		return 0;
	}
	else
	{
		return 1;
	}
}

/**
  * @brief  Save Lockstat
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_Lockstat_Save(void)
{
	uint8_t res;
	uint8_t * p;
	
	p = (uint8_t *) &WT_Config.Lockstat;
	res = BSP_EEPROM_WriteBuffer(p, E2PROM_ADDR_WT_CONFIG + 45, 1);
	if(res == 0) 	return 0;
	else					return 1;
}

/**
  * @brief  WT_Config_Testrule_Read
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_Testrule_Read(void)
{
	uint8_t res;
	uint8_t buf8[2];
	
	res = BSP_EEPROM_ReadBuffer(buf8, E2PROM_ADDR_WT_CONFIG + 46, 1);
	
	if(res == 0)
	{
		WT_Config.TestRule = buf8[0];			//0:unlock  FE:lock
		WT_Config_Testrule_Save();
		return 0;
	}
	else
	{
		return 1;
	}
}

/**
  * @brief  Save Testrule
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_Testrule_Save(void)
{
	uint8_t res;
	uint8_t * p;
	
	p = (uint8_t *) &WT_Config.TestRule;
	res = BSP_EEPROM_WriteBuffer(p, E2PROM_ADDR_WT_CONFIG + 46, 1);
	if(res == 0) 	return 0;
	else					return 1;
}

/**
  * @brief  WT_Config_Passwd_Read
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_Passwd_Read(void)
{
	uint8_t res;
	uint8_t buf8[7];
	
	res = BSP_EEPROM_ReadBuffer(buf8, E2PROM_ADDR_WT_CONFIG + 47, 6);
	
	if(res == 0)
	{
		WT_Config.Passwd[0] = buf8[0];			//
		WT_Config.Passwd[1] = buf8[1];
		WT_Config.Passwd[2] = buf8[2];
		WT_Config.Passwd[3] = buf8[3];
		WT_Config.Passwd[4] = buf8[4];
		WT_Config.Passwd[5] = buf8[5];
		return 0;
	}
	else
	{
		return 1;
	}
}

/**
  * @brief  Save ip addr
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_Ipaddr_Save(void)
{
	uint8_t res;
	uint8_t * p;
	
	p = (uint8_t *) &WT_Config.IP_Addr;
	res = BSP_EEPROM_WriteBuffer(p, E2PROM_ADDR_WT_CONFIG + 53, 4);
	if(res == 0) 	return 0;
	else					return 1;
}

uint8_t WT_Config_MAC_Save(void)
{
	uint8_t res;
	uint8_t * p;
	
	p = (uint8_t *) &WT_Config.MAC_Addr;
	res = BSP_EEPROM_WriteBuffer(p, E2PROM_ADDR_WT_CONFIG + 57, 6);
	if(res == 0) 	return 0;
	else					return 1;
}

/**
  * @brief  WT_Config_TestCap_Read
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_TestCap_Read(void)
{
	uint8_t res;
	uint8_t buf8[2];
	
	res = BSP_EEPROM_ReadBuffer(buf8, E2PROM_ADDR_WT_CONFIG + 63, 1);
	
	if(res == 0)
	{
		WT_Config.Mode_TestCap = buf8[0];			//0:unlock  FE:lock
		WT_Config_TestCap_Save();
		return 0;
	}
	else
	{
		return 1;
	}
}

/**
  * @brief  Save TestCap
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_TestCap_Save(void)
{
	uint8_t res;
	uint8_t * p;
	
	p = (uint8_t *) &WT_Config.Mode_TestCap;
	res = BSP_EEPROM_WriteBuffer(p, E2PROM_ADDR_WT_CONFIG + 63, 1);
	if(res == 0) 	return 0;
	else					return 1;
}

/**
  * @brief  WT_Config_StudyMode_Read
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_StudyMode_Read(void)
{
	uint8_t res;
	uint8_t buf8[2];
	
	res = BSP_EEPROM_ReadBuffer(buf8, E2PROM_ADDR_WT_CONFIG + 64, 1);
	
	if(res == 0)
	{
		WT_Config.StudyMode = buf8[0];			//0:unlock  FE:lock
		WT_Config_StudyMode_Save();
		return 0;
	}
	else
	{
		return 1;
	}
}

/**
  * @brief  Save StudyMode
  * @param  None.
  * @retval 0-ok, 1-error
  */
uint8_t WT_Config_StudyMode_Save(void)
{
	uint8_t res;
	uint8_t * p;
	
	p = (uint8_t *) &WT_Config.StudyMode;
	res = BSP_EEPROM_WriteBuffer(p, E2PROM_ADDR_WT_CONFIG + 64, 1);
	if(res == 0) 	return 0;
	else					return 1;
}

/**
  * @brief  WT_Config_PrintID_Read
  * @param  None.
  * @retval 0-ok, 1-error
  */
//uint8_t WT_Config_PrintID_Read(void)
//{
//	uint8_t res;
//	uint8_t buf8[2];
//	
//	res = BSP_EEPROM_ReadBuffer(buf8, E2PROM_ADDR_WT_CONFIG + 65, 1);
//	
//	if(res == 0)
//	{
//		WT_Config.Print_Mode = buf8[0];			//0：并口打印 1：串口打印
//		WT_Config_PrintID_Save();
//		return 0;
//	}
//	else
//	{
//		return 1;
//	}
//}

///**
//  * @brief  Save PrintID
//  * @param  None.
//  * @retval 0-ok, 1-error
//  */
//uint8_t WT_Config_PrintID_Save(void)
//{
//	uint8_t res;
//	uint8_t * p;
//	
//	p = (uint8_t *) &WT_Config.Print_Mode;
//	res = BSP_EEPROM_WriteBuffer(p, E2PROM_ADDR_WT_CONFIG + 65, 1);
//	if(res == 0) 	return 0;
//	else					return 1;
//}

//void WT_Test(void)
//{
//	uint8_t res;
//	static uint8_t i=0;
//	
//	res = WT_Config_Read();
//	if(res != 0) i++;
//	
//	res = WT_Config_Restore();
//	if(res != 0) i++;
//	
//	res = WT_Config_Read();
//	if(res != 0) i++;
//}


/**
  * @}
  */  

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
