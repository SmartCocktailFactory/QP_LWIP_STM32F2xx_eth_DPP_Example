/****************************************************************************************
                         Copyright (C) Adrian Lutz
                              All Rights Reserved
 ****************************************************************************************

  DESCRIPTION:        Control RS-232 serial COM port on Olimex P207 eval board.

 ****************************************************************************************

 ****************************************************************************************
 Includes
*****************************************************************************************/
#include "rs232.h"
#include <STM32F2xx_StdPeriph_Driver/inc/stm32f2xx_gpio.h>
#include <STM32F2xx_StdPeriph_Driver/inc/stm32f2xx_rcc.h>


/****************************************************************************************
 Defines
*****************************************************************************************/
/**
 * RS-232 COM port 2 on Olimex P207 eval board (connected to USART3)
 */
#define OMX_EVAL_COM2                        USART3
#define OMX_EVAL_COM2_CLK                    RCC_APB1Periph_USART3
#define OMX_EVAL_COM2_TX_PIN                 GPIO_Pin_8
#define OMX_EVAL_COM2_TX_GPIO_PORT           GPIOD
#define OMX_EVAL_COM2_TX_GPIO_CLK            RCC_AHB1Periph_GPIOD
#define OMX_EVAL_COM2_TX_SOURCE              GPIO_PinSource8
#define OMX_EVAL_COM2_TX_AF                  GPIO_AF_USART3
#define OMX_EVAL_COM2_RX_PIN                 GPIO_Pin_9
#define OMX_EVAL_COM2_RX_GPIO_PORT           GPIOD
#define OMX_EVAL_COM2_RX_GPIO_CLK            RCC_AHB1Periph_GPIOD
#define OMX_EVAL_COM2_RX_SOURCE              GPIO_PinSource9
#define OMX_EVAL_COM2_RX_AF                  GPIO_AF_USART3
#define OMX_EVAL_COM2_IRQn                   USART3_IRQn


/****************************************************************************************
 Enums
*****************************************************************************************/


/****************************************************************************************
 Structs and typedefs
*****************************************************************************************/


/****************************************************************************************
 Macros
*****************************************************************************************/


/****************************************************************************************
 Forward declarations
*****************************************************************************************/


/****************************************************************************************
 Local data
*****************************************************************************************/


/****************************************************************************************
 Public function implementations
*****************************************************************************************/
void omxEval_rs232_init(void)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(OMX_EVAL_COM2_TX_GPIO_CLK | OMX_EVAL_COM2_RX_GPIO_CLK, ENABLE);

  /* Enable UART clock */
  RCC_APB1PeriphClockCmd(OMX_EVAL_COM2_CLK, ENABLE);

  /* Connect PXx to USARTx_TX*/
  GPIO_PinAFConfig(OMX_EVAL_COM2_TX_GPIO_PORT, OMX_EVAL_COM2_TX_SOURCE, OMX_EVAL_COM2_TX_AF);

  /* Connect PXx to USARTx_RX*/
  GPIO_PinAFConfig(OMX_EVAL_COM2_RX_GPIO_PORT, OMX_EVAL_COM2_RX_SOURCE, OMX_EVAL_COM2_RX_AF);

  /* Configure USART TX as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = OMX_EVAL_COM2_TX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(OMX_EVAL_COM2_TX_GPIO_PORT, &GPIO_InitStructure);

  /* Configure USART RX as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = OMX_EVAL_COM2_RX_PIN;
  GPIO_Init(OMX_EVAL_COM2_RX_GPIO_PORT, &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(OMX_EVAL_COM2, &USART_InitStructure);

  /* Enable USART */
  USART_Cmd(OMX_EVAL_COM2, ENABLE);
}

void omxEval_rs232_testTx(void)
{
  while(1) {
    if ((OMX_EVAL_COM2->SR & USART_FLAG_TXE) != 0) {              /* check if transmit data register is empty */
      uint16_t b = 0;
      char * pB = (char*)(&b);
      *pB = 'a';      /* this character actually gets written to the serial port */
      *(pB+1) = 'b';  /* this character is NOT written to the serial port */

      OMX_EVAL_COM2->DR = (b & 0xFF);      /* put data to be sent into the DR register */
    }
  }
}

void omxEval_rs232_testRx(void)
{
  while(1) {
    if ((OMX_EVAL_COM2->SR & USART_FLAG_RXNE) != 0) {              /* check if receive data register is not empty */
      uint16_t b = 0;
      char * pB0 = (char*)(&b);
      char b0 = 0;
      char b1 = 0;

      b = OMX_EVAL_COM2->DR;      /* read data from the DR register */

      b0 = *pB0;  /* character is received here */
      b1 = *(pB0 + 1);  /* no data */
    }
  }
}


/****************************************************************************************
 Private function implementations
*****************************************************************************************/
