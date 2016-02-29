/**
  ******************************************************************************
  * @file    wt_uart_24GHz.h
  * @author  zhang manxin
  * @version V1.0.0
  * @date    2014-7-23
  * @brief   This file contains all the functions prototypes for the uart_24GHz.
  ******************************************************************************
  */ 


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  __WT_UART_24GHZ_H
#define  __WT_UART_24GHZ_H


/*
*********************************************************************************************************
*                                              EXTERNS
*********************************************************************************************************
*/

#ifdef   WT_UART_24GHZ_GLOBALS
#define  WT_UART_24GHZ_EXT
#else
#define  WT_UART_24GHZ_EXT  extern
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


#define UART_24GHz_RX_Bufer_Length			1100
#define UART_24GHz_RX_Timeout						100

#define Uart24GHZ_RX_Event							1
#define Uart24GHZ_TX_Event							2
#define Uart24GHZ_CONF_Event						3
/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/
void UART24GHZThread(void const * argument);
void WT_UART_24GHz_Init(void);
WT_UART_24GHZ_EXT uint8_t UART_24GHz_Buffer_Rx[UART_24GHz_RX_Bufer_Length];
//WT_UART_24GHZ_EXT uint8_t UART_24GHz_Cnt_Buffer_Rx;
WT_UART_24GHZ_EXT uint16_t UART_24GHz_Cnt_Buffer_Rx;
WT_UART_24GHZ_EXT uint8_t Is_UART_24GHz_Rx_Come;

static uint8_t UART_24GHz_UpdateConfig(void);
static void UART_24GHz_Buffer_RxClear(void);
static void Uart24G_HeartBeat(char * buff);
static uint8_t Uart24G_GetDevStat(void);


/*
*********************************************************************************************************
*                                              DATA TYPES
*********************************************************************************************************
*/




/*
*********************************************************************************************************
*                                              MODULE END
*********************************************************************************************************
*/

#endif
