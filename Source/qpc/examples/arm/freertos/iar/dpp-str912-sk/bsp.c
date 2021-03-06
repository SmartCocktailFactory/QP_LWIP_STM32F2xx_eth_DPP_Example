/*****************************************************************************
* Product: Board Support Package for for STR12-SK, with FreeRTOS.org
* Last Updated for Version: 4.0.01
* Date of the Last Update:  Jul 02, 2008
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2008 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information:
* Quantum Leaps Web site:  http://www.quantum-leaps.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

#include "91x_lib.h"                                        /* ST ST91x MCU */

Q_DEFINE_THIS_FILE

/* Local objects -----------------------------------------------------------*/
static u8 const l_gpioPin[] = {
    0,            /* unused */

    GPIO_Pin_0,   /* LED  1 */
    GPIO_Pin_1,   /* LED  2 */
    GPIO_Pin_2,   /* LED  3 */
    GPIO_Pin_3,   /* LED  4 */
    GPIO_Pin_4,   /* LED  5 */
    GPIO_Pin_5,   /* LED  6 */
    GPIO_Pin_6,   /* LED  7 */
    GPIO_Pin_7,   /* LED  8 */

    GPIO_Pin_0,   /* LED  9 */
    GPIO_Pin_1,   /* LED 10 */
    GPIO_Pin_2,   /* LED 11 */
    GPIO_Pin_3,   /* LED 12 */
    GPIO_Pin_4,   /* LED 13 */
    GPIO_Pin_5,   /* LED 14 */
    GPIO_Pin_6,   /* LED 15 */
    GPIO_Pin_7    /* LED 16 */
};

#define LED_ON(num_)       GPIO_WriteBit(GPIO6, l_gpioPin[num_], Bit_RESET)
#define LED_OFF(num_)      GPIO_WriteBit(GPIO6, l_gpioPin[num_], Bit_SET)
#define LED_ON_ALL()       GPIO_WriteBit(GPIO6, GPIO_Pin_All,   Bit_RESET)
#define LED_OFF_ALL()      GPIO_WriteBit(GPIO6, GPIO_Pin_All,   Bit_SET)

typedef void (*IntVector)(void);           /* IntVector pointer-to-function */

/* set prescaler and priority of Timer3, which is system clock tick source */
#define BSP_CPU_CLOCK_HZ   96000000
#define BSP_CPU_PERIPH_HZ  (BSP_CPU_CLOCK_HZ/2)
#define BSP_TIM3_PRESCALER 48
#define BSP_TICK_PRIO      15

#define BSP_UART_TX_FIFO   16
#define BSP_QS_BUF_SIZE    (2*1024)
#define BSP_QS_BAUD_RATE   115200


#define BSP_TIM3_PERIOD    \
    (BSP_CPU_PERIPH_HZ / BSP_TIM3_PRESCALER / BSP_TICKS_PER_SEC)


#ifdef Q_SPY
    static uint32_t l_currTime32;
    static uint16_t l_prevTime16;
    enum AppRecords {                 /* application-specific trace records */
        PHILO_STAT = QS_USER
    };
#endif

/* ISRs --------------------------------------------------------------------*/
__arm void TIM3_IRQHandler(void) {
    portISR_ENTRY();        /* always inform FreeRTOS about entering an ISR */

    TIM3->OC1R += BSP_TIM3_PERIOD - 1;   /* set the output compare register */
    TIM3->SR &= ~TIM_IT_OC1;             /* clear interrupt source (Timer3) */

#ifdef Q_SPY
    {
        uint16_t currTime16 = (uint16_t)TIM3->CNTR;
        l_currTime32 += (currTime16 - l_prevTime16) & 0xFFFF;
        l_prevTime16 = currTime16;
    }
#endif

    vTaskIncrementTick();      /* handle the FreeRTOS.org system clock tick */
    QF_tick();                           /* handle the QF system clock tick */

    portISR_EXIT();          /* always inform FreeRTOS about exiting an ISR */
}
/*..........................................................................*/
static __arm void ISR_spur(void) {
    portISR_ENTRY();        /* always inform FreeRTOS about entering an ISR */
    portISR_EXIT();          /* always inform FreeRTOS about exiting an ISR */
}
/*..........................................................................*/
void vStartInterrupts(void) {              /* called with interrupts locked */
    TIM_InitTypeDef TIM_InitStruct;

    SCU_APBPeriphClockConfig(__TIM23, ENABLE);
    TIM_DeInit(TIM3);

    TIM_StructInit(&TIM_InitStruct);
    TIM_InitStruct.TIM_Mode           = TIM_OCM_CHANNEL_1;
    TIM_InitStruct.TIM_OC1_Modes      = TIM_TIMING;
    TIM_InitStruct.TIM_Clock_Source   = TIM_CLK_APB;
    TIM_InitStruct.TIM_Clock_Edge     = TIM_CLK_EDGE_RISING;
    TIM_InitStruct.TIM_Prescaler      = BSP_TIM3_PRESCALER - 1;
    TIM_InitStruct.TIM_Pulse_Level_1  = TIM_HIGH;
    TIM_InitStruct.TIM_Pulse_Length_1 = BSP_TIM3_PERIOD - 1;
    TIM_Init(TIM3, &TIM_InitStruct);

                                /* configure the VIC for the TIM3 interrupt */
    VIC_Config(TIM3_ITLine, VIC_IRQ, BSP_TICK_PRIO);
    VIC_ITCmd(TIM3_ITLine, ENABLE);

    TIM_ITConfig(TIM3, TIM_IT_OC1, ENABLE);
    TIM_CounterCmd(TIM3, TIM_CLEAR);
    TIM_CounterCmd(TIM3, TIM_START);
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
void vApplicationIdleHook(void) {

    portDISABLE_INTERRUPTS();
    LED_ON(16);
    LED_OFF(16);
    portENABLE_INTERRUPTS();

#ifdef Q_SPY                     /* use the idle cycles for QS transmission */

    if ((UART0->FR & 0x80) != 0) {                        /* TX FIFO empty? */
        uint16_t nBytes = BSP_UART_TX_FIFO;      /* capacity of the TX FIFO */
        uint8_t const *block;
        QF_INT_LOCK(dummy);
        block = QS_getBlock(&nBytes);     /* get the data block to transfer */
        QF_INT_UNLOCK(dummy);
        while (nBytes-- != 0) {
            UART0->DR = *block++;          /* stick the byte to the TX FIFO */
        }
    }

#elif defined NDEBUG       /* only if not debugging (Release configuration) */
    /* shut down unused peripherals to save power ...*/
//    SCU_EnterIdleMode();         /* Power-Management: disable the CPU clock */
// NOTE: idle or sleep mode hangs the J-TAG, it's difficult to
// get control of the MCU again!!!
#endif
}
/*..........................................................................*/
void BSP_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    uint8_t n;

    WDG_Cmd(DISABLE);

    SCU_PCLKDivisorConfig(SCU_PCLK_Div2); /* peripheral bus clokdivisor = 2 */

                            /* GPIO 6 clock source enable, used by the LEDs */
    SCU_APBPeriphClockConfig(__GPIO6, ENABLE);
                                                        /* enable VIC clock */
    SCU_AHBPeriphClockConfig(__VIC, ENABLE);
    SCU_AHBPeriphReset(__VIC, DISABLE);

    VIC_DeInit();
    VIC0->DVAR = (uint32_t)&ISR_spur;  /* install spurious handler for VIC0 */
    VIC1->DVAR = (uint32_t)&ISR_spur;  /* install spurious handler for VIC1 */
    for (n = 0; n < 32; ++n) {
        VIC_ITCmd(n, DISABLE);
    }
                                   /* configure GPIO6 to drive the LED's... */
    GPIO_DeInit(GPIO6);
    GPIO_InitStruct.GPIO_Direction   = GPIO_PinOutput;
    GPIO_InitStruct.GPIO_Pin         = GPIO_Pin_All;
    GPIO_InitStruct.GPIO_Type        = GPIO_Type_PushPull;
    GPIO_InitStruct.GPIO_IPConnected = GPIO_IPConnected_Disable;
    GPIO_InitStruct.GPIO_Alternate   = GPIO_OutputAlt1;
    GPIO_Init(GPIO6, &GPIO_InitStruct);

    if (QS_INIT((void *)0) == 0) {    /* initialize the QS software tracing */
        Q_ERROR();
    }
}
/*..........................................................................*/
void BSP_led_on (uint8_t n) {
    LED_ON(n);
}
/*..........................................................................*/
void BSP_led_off(uint8_t n) {
    LED_OFF(n);
}
/*..........................................................................*/
void BSP_led_toggle(uint8_t n) {
    GPIO_WriteBit(GPIO6, l_gpioPin[n],
        GPIO_ReadBit(GPIO6, l_gpioPin[n]) == Bit_RESET ? Bit_SET : Bit_RESET);
}

/*..........................................................................*/
void BSP_displyPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == (uint8_t)'e') {           /* is this Philosopher eating? */
        LED_ON(9 + n);
    }
    else {                                /* this Philosopher is not eating */
        LED_OFF(9 + n);
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n])  /* application-specific record begin */
        QS_U8(1, n);                                  /* Philosopher number */
        QS_STR(stat);                                 /* Philosopher status */
    QS_END()
}
/*..........................................................................*/
__arm void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    __disable_interrupt();    /* make sure that all interrupts are disabled */
    LED_ON_ALL();                                      /* light up all LEDs */
    for (;;) {                            /* hang here in the for-ever loop */
    }
}

/*--------------------------------------------------------------------------*/
#ifdef Q_SPY
uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[BSP_QS_BUF_SIZE];        /* buffer for Quantum Spy */
    GPIO_InitTypeDef GPIO_InitStruct;
    UART_InitTypeDef UART_InitStruct;

    QS_initBuf(qsBuf, sizeof(qsBuf));

                                 /* configure the UART0 for QSPY output ... */
    SCU_APBPeriphClockConfig(__UART0, ENABLE);    /* enable clock for UART0 */
    SCU_APBPeriphClockConfig(__GPIO3, ENABLE);    /* enable clock for GPIO3 */

    SCU_APBPeriphReset(__UART0, DISABLE);        /* remove UART0 from reset */
    SCU_APBPeriphReset(__GPIO3, DISABLE);        /* remove GPIO3 from reset */

                                      /* configure UART0_TX pin GPIO3.4 ... */
    GPIO_DeInit(GPIO3);
    GPIO_InitStruct.GPIO_Pin         = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Direction   = GPIO_PinOutput;
    GPIO_InitStruct.GPIO_Type        = GPIO_Type_PushPull;
    GPIO_InitStruct.GPIO_IPConnected = GPIO_IPConnected_Disable;
    GPIO_InitStruct.GPIO_Alternate   = GPIO_OutputAlt3;
    GPIO_Init(GPIO3, &GPIO_InitStruct);

                                                      /* configure UART0... */
    UART_DeInit(UART0);            /* force UART0 registers to reset values */
    UART_InitStruct.UART_WordLength  = UART_WordLength_8D;
    UART_InitStruct.UART_StopBits    = UART_StopBits_1;
    UART_InitStruct.UART_Parity      = UART_Parity_No;
    UART_InitStruct.UART_BaudRate    = BSP_QS_BAUD_RATE;
    UART_InitStruct.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
    UART_InitStruct.UART_Mode        = UART_Mode_Tx;
    UART_InitStruct.UART_FIFO        = UART_FIFO_Enable;
    UART_InitStruct.UART_TxFIFOLevel = UART_FIFOLevel_1_8;
    UART_InitStruct.UART_RxFIFOLevel = UART_FIFOLevel_1_8;
    UART_Init(UART0, &UART_InitStruct);                 /* initialize UART0 */
    UART_Cmd(UART0, ENABLE);                                /* enable UART0 */

                                                 /* setup the QS filters... */
    QS_FILTER_ON(QS_ALL_RECORDS);

//    QS_FILTER_OFF(QS_QEP_STATE_EMPTY);
//    QS_FILTER_OFF(QS_QEP_STATE_ENTRY);
//    QS_FILTER_OFF(QS_QEP_STATE_EXIT);
//    QS_FILTER_OFF(QS_QEP_STATE_INIT);
//    QS_FILTER_OFF(QS_QEP_INIT_TRAN);
//    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
//    QS_FILTER_OFF(QS_QEP_TRAN);
//    QS_FILTER_OFF(QS_QEP_dummyD);

    QS_FILTER_OFF(QS_QF_ACTIVE_ADD);
    QS_FILTER_OFF(QS_QF_ACTIVE_REMOVE);
    QS_FILTER_OFF(QS_QF_ACTIVE_SUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_UNSUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET_LAST);
    QS_FILTER_OFF(QS_QF_EQUEUE_INIT);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET_LAST);
    QS_FILTER_OFF(QS_QF_MPOOL_INIT);
    QS_FILTER_OFF(QS_QF_MPOOL_GET);
    QS_FILTER_OFF(QS_QF_MPOOL_PUT);
    QS_FILTER_OFF(QS_QF_PUBLISH);
    QS_FILTER_OFF(QS_QF_NEW);
    QS_FILTER_OFF(QS_QF_GC_ATTEMPT);
    QS_FILTER_OFF(QS_QF_GC);
//    QS_FILTER_OFF(QS_QF_TICK);
    QS_FILTER_OFF(QS_QF_TIMEEVT_ARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_AUTO_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM_ATTEMPT);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_REARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_POST);
    QS_FILTER_OFF(QS_QF_INT_LOCK);
    QS_FILTER_OFF(QS_QF_INT_UNLOCK);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);

    return (uint8_t)1;            /* indicate successfull QS initialization */
}
/*..........................................................................*/
void QS_onCleanup(void) {
}
/*..........................................................................*/
void QS_onFlush(void) {
    uint16_t nBytes = BSP_UART_TX_FIFO; /* the capacity of the UART TX FIFO */
    uint8_t const *block;
    while ((block = QS_getBlock(&nBytes)) != (uint8_t *)0) {
        while ((UART0->FR & 0x80) == 0) {             /* TX FIFO not empty? */
        }                                                /* keep waiting... */
        while (nBytes-- != 0) {
            UART0->DR = *block++;          /* stick the byte to the TX FIFO */
        }
        nBytes = BSP_UART_TX_FIFO;              /* for the next time around */
    }
}
/*..........................................................................*/
/* NOTE: getTime is invoked within a critical section (inetrrupts disabled) */
uint32_t QS_onGetTime(void) {
    uint16_t currTime16 = (uint16_t)TIM3->CNTR;
    l_currTime32 += (currTime16 - l_prevTime16) & 0xFFFF;
    l_prevTime16 = currTime16;
    return l_currTime32;
}
#endif                                                             /* Q_SPY */
/*--------------------------------------------------------------------------*/
