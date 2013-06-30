/**
  ******************************************************************************
  * @file    stm32f2x7_eth_bsp.c
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    06-June-2011 
  * @brief   STM32F2x7 Ethernet hardware configuration.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f2x7_eth.h"
#include "stm32f2x7_eth_bsp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DP83848_PHY_ADDRESS       0x01 /* Relative to STM322xG-EVAL Board;
                                          alu: the PHY KS8721BL on the Olimex STM32-P207 eval board uses the same default address
                                          --> no change needed */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/


#define RMII_MODE  /* alu: only RMII mode is supported on the Olimex STM32-P207 eval board */
/* Uncomment the define below to clock the PHY from external 25MHz crystal (only for MII mode) */
#ifdef 	MII_MODE
 #define PHY_CLOCK_MCO
#endif
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Configures the Ethernet Interface
  * @param  None
  * @retval None
  */
uint32_t ETH_MACDMA_Config(void) {
	/* Initialize some variables */
	uint32_t ETH_Status = ETH_ERROR;
	ETH_InitTypeDef ETH_InitStructure;

	/* Enable ETHERNET clock  */
	RCC_AHB1PeriphClockCmd(	RCC_AHB1Periph_ETH_MAC |
			RCC_AHB1Periph_ETH_MAC_Tx |
			RCC_AHB1Periph_ETH_MAC_Rx,
			ENABLE);

	/* Reset ETHERNET on AHB Bus */
	ETH_DeInit();

	/* Software reset */
	ETH_SoftwareReset();

	/* Wait for software reset */
	while (ETH_GetSoftwareResetStatus() == SET);

	/* ETHERNET Configuration --------------------------------------------------*/
	/* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
	ETH_StructInit(&ETH_InitStructure);

	/* Fill ETH_InitStructure parametrs */
	/*------------------------   MAC   -----------------------------------*/

	ETH_InitStructure.ETH_AutoNegotiation          		= ETH_AutoNegotiation_Enable;
	//  ETH_InitStructure.ETH_AutoNegotiation          	= ETH_AutoNegotiation_Disable;
	ETH_InitStructure.ETH_Speed                    		= ETH_Speed_100M;
	ETH_InitStructure.ETH_Mode                     		= ETH_Mode_FullDuplex;
	ETH_InitStructure.ETH_LoopbackMode             		= ETH_LoopbackMode_Disable;
	ETH_InitStructure.ETH_RetryTransmission        		= ETH_RetryTransmission_Disable;
	ETH_InitStructure.ETH_AutomaticPadCRCStrip     		= ETH_AutomaticPadCRCStrip_Disable;
	ETH_InitStructure.ETH_ReceiveAll               		= ETH_ReceiveAll_Disable;
	ETH_InitStructure.ETH_BroadcastFramesReception 		= ETH_BroadcastFramesReception_Enable;
	ETH_InitStructure.ETH_PromiscuousMode          		= ETH_PromiscuousMode_Disable;
	ETH_InitStructure.ETH_MulticastFramesFilter   		= ETH_MulticastFramesFilter_Perfect;
	ETH_InitStructure.ETH_UnicastFramesFilter      		= ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload          = ETH_ChecksumOffload_Enable;
#endif

	/*------------------------   DMA   -----------------------------------*/

	/* When we use the Checksum offload feature, we need to enable the Store
	 * and Forward mode: the store and forward guarantee that a whole frame
	 * is stored in the FIFO, so the MAC can insert/verify the checksum, if
	 * the checksum is OK the DMA can handle the frame otherwise the frame
	 * is dropped */
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame 	= ETH_DropTCPIPChecksumErrorFrame_Enable;
	ETH_InitStructure.ETH_ReceiveStoreForward 			= ETH_ReceiveStoreForward_Enable;
	ETH_InitStructure.ETH_TransmitStoreForward 			= ETH_TransmitStoreForward_Enable;

	ETH_InitStructure.ETH_ForwardErrorFrames 			= ETH_ForwardErrorFrames_Disable;
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames 	= ETH_ForwardUndersizedGoodFrames_Disable;
	ETH_InitStructure.ETH_SecondFrameOperate 			= ETH_SecondFrameOperate_Enable;
	ETH_InitStructure.ETH_AddressAlignedBeats 			= ETH_AddressAlignedBeats_Enable;
	ETH_InitStructure.ETH_FixedBurst 					= ETH_FixedBurst_Enable;
	ETH_InitStructure.ETH_RxDMABurstLength 				= ETH_RxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_TxDMABurstLength				= ETH_TxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_DMAArbitration 				= ETH_DMAArbitration_RoundRobin_RxTx_2_1;

	/* Configure Ethernet */
	ETH_Status = ETH_Init(&ETH_InitStructure, DP83848_PHY_ADDRESS);

	/* Enable the Ethernet Rx and Tx Interrupts */
	ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R | ETH_DMA_IT_T, ENABLE);

	return ETH_Status;
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void ETH_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOs clocks; alu: H and I are not required for Olimex STM32-P207 eval board */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
                         RCC_AHB1Periph_GPIOC |
                         RCC_AHB1Periph_GPIOG |
                         RCC_AHB1Periph_GPIOF, ENABLE);

  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

#if 0  /* alu: MCO is not used on Olimex STM32-P207 eval board */
  /* Configure MCO (PA8) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
  
  /* MII/RMII Media interface selection --------------------------------------*/
#ifdef MII_MODE /* Mode MII with STM322xG-EVAL  */
 #ifdef PHY_CLOCK_MCO

  /* Output HSE clock (25MHz) on MCO pin (PA8) to clock the PHY */
  RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1);
 #endif /* PHY_CLOCK_MCO */

  SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_MII);
#elif defined RMII_MODE  /* Mode RMII with STM322xG-EVAL */

#if 0  /* alu: MCO is not used on Olimex STM32-P207 eval board */
  /* Output PLL clock divided by 2 (50MHz) on MCO pin (PA8) to clock the PHY */
  RCC_MCO1Config(RCC_MCO1Source_PLLCLK, RCC_MCO1Div_2);
#endif

  SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);
#endif
  
/* Ethernet pins configuration ************************************************/
   /*
        ETH_MDIO -------------------------> PA2    (same on Olimex STM32-P207 eval board)
        ETH_MDC --------------------------> PC1    (same on Olimex STM32-P207 eval board)
        ETH_PPS_OUT ----------------------> PB5    (not used on Olimex STM32-P207 eval board)
        ETH_MII_CRS ----------------------> PH2    (not used on Olimex STM32-P207 eval board)
        ETH_MII_COL ----------------------> PH3    (not used on Olimex STM32-P207 eval board)
        ETH_MII_RX_ER --------------------> PI10   (PF10 on Olimex STM32-P207 eval board)
        ETH_MII_RXD2 ---------------------> PH6    (not used on Olimex STM32-P207 eval board)
        ETH_MII_RXD3 ---------------------> PH7    (not used on Olimex STM32-P207 eval board)
        ETH_MII_TX_CLK -------------------> PC3    (not used on Olimex STM32-P207 eval board)
        ETH_MII_TXD2 ---------------------> PC2    (not used on Olimex STM32-P207 eval board)
        ETH_MII_TXD3 ---------------------> PB8    (not used on Olimex STM32-P207 eval board)
        ETH_MII_RX_CLK/ETH_RMII_REF_CLK---> PA1    (same on Olimex STM32-P207 eval board)
        ETH_MII_RX_DV/ETH_RMII_CRS_DV ----> PA7    (same on Olimex STM32-P207 eval board)
        ETH_MII_RXD0/ETH_RMII_RXD0 -------> PC4    (same on Olimex STM32-P207 eval board)
        ETH_MII_RXD1/ETH_RMII_RXD1 -------> PC5    (same on Olimex STM32-P207 eval board)
        ETH_MII_TX_EN/ETH_RMII_TX_EN -----> PG11   (PB11 on Olimex STM32-P207 eval board)
        ETH_MII_TXD0/ETH_RMII_TXD0 -------> PG13   (same on Olimex STM32-P207 eval board)
        ETH_MII_TXD1/ETH_RMII_TXD1 -------> PG14   (same on Olimex STM32-P207 eval board)
                                                  */

  /* Configure PA1, PA2 and PA7 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_7;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

#if 0  /* alu: not used on Olimex STM32-P207 eval board */
  /* Configure PB5 and PB8 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_8;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_ETH);	
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_ETH);
#endif

  /* alu: PB11 is used on Olimex STM32-P207 eval board for TX_EN (instead of PG11 on STM322xG-EVAL) */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_ETH);

  /* Configure PC1, PC2, PC3, PC4 and PC5; alu: PC2 and PC3 are not used on Olimex STM32-P207 eval board */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
                                
  /* Configure PG11, PG14 and PG13; alu: PG11 is not used on Olimex STM32-P207 eval board (use PB11 instead, see above) */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13 | GPIO_Pin_14;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);

#if 0  /* alu: not used on Olimex STM32-P207 eval board */
  /* Configure PH2, PH3, PH6, PH7 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_Init(GPIOH, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource2, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource3, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource6, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource7, GPIO_AF_ETH);
#endif

  /* Configure PI10; alu: use PF10 on Olimex STM32-P207 eval board */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOF, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource10, GPIO_AF_ETH);
}



/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
