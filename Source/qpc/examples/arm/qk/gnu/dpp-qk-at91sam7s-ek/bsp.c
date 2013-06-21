/*****************************************************************************
* Product: QDK/C_ARM-GNU_AT91SAM7S-EK, QK port
* Last Updated for Version: 4.3.00
* Date of the Last Update:  Nov 08, 2011
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2011 Quantum Leaps, LLC. All rights reserved.
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

Q_DEFINE_THIS_FILE

/* Local objects -----------------------------------------------------------*/
uint32_t const l_led[] = {
    (1 << 0),                                     /* LED D1 on AT91SAM7S-EK */
    (1 << 1),                                     /* LED D2 on AT91SAM7S-EK */
    (1 << 2),                                     /* LED D3 on AT91SAM7S-EK */
    (1 << 3),                                     /* LED D4 on AT91SAM7S-EK */
    0                                          /* no LED 5  on AT91SAM7S-EK */
};

#define LED_ON(num_)       (AT91C_BASE_PIOA->PIO_CODR = l_led[num_])
#define LED_OFF(num_)      (AT91C_BASE_PIOA->PIO_SODR = l_led[num_])

typedef void (*IntVector)(void);           /* IntVector pointer-to-function */

#ifdef Q_SPY
    uint8_t const QS_tickIRQ = 0;
    enum AppRecords {                 /* application-specific trace records */
        PHILO_STAT = QS_USER
    };

#endif

/*..........................................................................*/
void QK_init(void) {
}
/*..........................................................................*/
__attribute__ ((section (".text.fastcode")))
void QK_onIdle(void) {
#ifdef Q_SPY                     /* use the idle cycles for QS transmission */

    if ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXBUFE) != 0) {  /* not busy? */
        uint16_t nBytes = 0xFFFF;
        uint8_t const *block;

        QF_INT_DISABLE();
        if ((block = QS_getBlock(&nBytes)) != (uint8_t *)0) { /* new block? */
            AT91C_BASE_DBGU->DBGU_TPR = (uint32_t)block;
            AT91C_BASE_DBGU->DBGU_TCR = (uint32_t)nBytes;
            nBytes = 0xFFFF;
            if ((block = QS_getBlock(&nBytes)) != (uint8_t *)0) {/*another? */
                AT91C_BASE_DBGU->DBGU_TNPR = (uint32_t)block;
                AT91C_BASE_DBGU->DBGU_TNCR = (uint32_t)nBytes;
            }
        }
        QF_INT_ENABLE();
    }

#elif defined NDEBUG /* only if not debugging (idle mode hinders debugging) */
    AT91C_BASE_PMC->PMC_SCDR = 1;/* Power-Management: disable the CPU clock */
    /* NOTE: an interrupt starts the CPU clock again */
#endif
}
/*..........................................................................*/
void BSP_init(void) {
    uint32_t i;

    for (i = 0; i < Q_DIM(l_led); ++i) {          /* initialize the LEDs... */
        AT91C_BASE_PIOA->PIO_PER = l_led[i];                  /* enable pin */
        AT91C_BASE_PIOA->PIO_OER = l_led[i];     /* configure as output pin */
        LED_OFF(i);                                   /* extinguish the LED */
    }
                /* configure Advanced Interrupt Controller (AIC) of AT91... */
    AT91C_BASE_AIC->AIC_IDCR = ~0;                /* disable all interrupts */
    AT91C_BASE_AIC->AIC_ICCR = ~0;                  /* clear all interrupts */
    for (i = 0; i < 8; ++i) {
        AT91C_BASE_AIC->AIC_EOICR = 0;           /* write AIC_EOICR 8 times */
    }

                             /* set the desired ticking rate for the PIT... */
    i = (MCK / 16 / BSP_TICKS_PER_SEC) - 1;
    AT91C_BASE_PITC->PITC_PIMR = (AT91C_PITC_PITEN | AT91C_PITC_PITIEN | i);

    if (QS_INIT((void *)0) == 0) {    /* initialize the QS software tracing */
        Q_ERROR();
    }

    QS_OBJ_DICTIONARY(&QS_tickIRQ);
}
/*..........................................................................*/
void BSP_busyDelay(void) {
}
/*..........................................................................*/
void BSP_displyPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == (uint8_t)'e') {           /* is this Philosopher eating? */
        LED_ON(n);
    }
    else {                                /* this Philosopher is not eating */
        LED_OFF(n);
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n])  /* application-specific record begin */
        QS_U8(1, n);                                  /* Philosopher number */
        QS_STR(stat);                                 /* Philosopher status */
    QS_END()
}
/*..........................................................................*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    QF_INT_DISABLE();
    for (;;) {                            /* hang here in the for-ever loop */
    }
}

/*--------------------------------------------------------------------------*/
#ifdef Q_SPY
uint32_t l_timeOverflow;

#define QS_BUF_SIZE        (2*1024)
#define BAUD_RATE          115200

uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[QS_BUF_SIZE];            /* buffer for Quantum Spy */
    AT91PS_DBGU pDBGU = AT91C_BASE_DBGU;
    AT91PS_TC   pTC0  = AT91C_BASE_TC0;/* TC0 used for timestamp generation */
    uint32_t tmp;

    QS_initBuf(qsBuf, sizeof(qsBuf));

    /* configure the Debug UART for QSPY output ... */
    AT91C_BASE_PIOA->PIO_PDR = AT91C_PA10_DTXD;    /* configure pin as DTXD */

    pDBGU->DBGU_CR   = AT91C_US_TXEN;            /* enable only transmitter */
    pDBGU->DBGU_IDR  = ~0;                   /* disable all DBGU interrupts */
    pDBGU->DBGU_MR   = AT91C_US_PAR_NONE;                  /* no parity bit */
    pDBGU->DBGU_BRGR = ((MCK/BAUD_RATE + 8) >> 4);   /* baud rate generator */
    pDBGU->DBGU_PTCR = AT91C_PDC_TXTEN;    /* enable PDC transfer from DBGU */

    /* configure Timer/Counter 0 for time measurements ... */
    AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0);  /* enable clock to TC0 */

    pTC0->TC_CCR = AT91C_TC_CLKDIS;        /* TC_CCR: disable Clock Counter */
    pTC0->TC_IDR = ~0;              /* TC_IDR: disable all timer interrupts */
    tmp = pTC0->TC_SR;                                       /* TC_SR: read */
                                                    /* CPCTRG, MCK/32 clock */
    pTC0->TC_CMR = (AT91C_TC_CPCTRG | AT91C_TC_CLKS_TIMER_DIV3_CLOCK);
    pTC0->TC_CCR = AT91C_TC_CLKEN;          /* TC_CCR: enable Clock Counter */
    pTC0->TC_CCR = AT91C_TC_SWTRG;                /* TC_CCR: start counting */

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
    QS_FILTER_OFF(QS_QF_CRIT_ENTRY);
    QS_FILTER_OFF(QS_QF_CRIT_EXIT);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);

    return (uint8_t)1;            /* indicate successfull QS initialization */
}
/*..........................................................................*/
void QS_onCleanup(void) {
}
/*..........................................................................*/
void QS_onFlush(void) {
    uint16_t nBytes = 0xFFFF;
    uint8_t const *block;
    while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXBUFE) == 0) {   /* busy? */
    }
    if ((block = QS_getBlock(&nBytes)) != (uint8_t *)0) {
        AT91C_BASE_DBGU->DBGU_TPR = (uint32_t)block;
        AT91C_BASE_DBGU->DBGU_TCR = (uint32_t)nBytes;
        nBytes = 0xFFFF;
        if ((block = QS_getBlock(&nBytes)) != (uint8_t *)0) {
            AT91C_BASE_DBGU->DBGU_TNPR = (uint32_t)block;
            AT91C_BASE_DBGU->DBGU_TNCR = (uint32_t)nBytes;
        }
    }
}
/*..........................................................................*/
/* NOTE: getTime is invoked within a critical section (inetrrupts disabled) */
__attribute__ ((section (".text.fastcode")))
uint32_t QS_onGetTime(void) {
    AT91PS_TC pTC0  = AT91C_BASE_TC0;  /* TC0 used for timestamp generation */
    uint32_t now = pTC0->TC_CV;                    /* get the counter value */
                                          /* did the timer overflow 0xFFFF? */
    if ((pTC0->TC_SR & AT91C_TC_COVFS) != 0) {
        l_timeOverflow += (uint32_t)0x10000;    /* account for the overflow */
    }
    return l_timeOverflow + now;
}
#endif                                                             /* Q_SPY */
/*--------------------------------------------------------------------------*/
