/*****************************************************************************
* Product: BSP for DPP on eZ430-RF2500, QK kernel, TI CCS MSP430 compiler
* Last Updated for Version: 4.5.00
* Date of the Last Update:  May 18, 2012
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* Contact information:
* Quantum Leaps Web sites: http://www.quantum-leaps.com
*                          http://www.state-machine.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

#define BSP_MCK       8000000U
#define BSP_SMCLK     8000000U
#define BSP_ACLK      12000U

#define LED0_on()     (P1OUT |= (uint8_t)BIT0)
#define LED0_off()    (P1OUT &= (uint8_t)~BIT0)

#define LED1_on()     (P1OUT |= (uint8_t)BIT1)
#define LED1_off()    (P1OUT &= (uint8_t)~BIT1)

#ifdef Q_SPY
    QSTimeCtr QS_tickTime_;
    static uint8_t const l_timerA_ISR = 0;

    enum AppRecords {                 /* application-specific trace records */
        PHILO_STAT = QS_USER
    };
#endif


/*..........................................................................*/
#pragma vector = TIMERA0_VECTOR
__interrupt void timerA_ISR(void) {
#ifdef NDEBUG
    __low_power_mode_off_on_exit();
#endif

    QK_ISR_ENTRY();                            /* inform QK about ISR entry */

#ifdef Q_SPY
    TACTL &= ~TAIFG;                    /* clear the interrupt pending flag */
    QS_tickTime_ +=
       (((BSP_SMCLK / 8) + BSP_TICKS_PER_SEC/2) / BSP_TICKS_PER_SEC) + 1;
#endif

    QF_TICK(&l_timerA_ISR);

    QK_ISR_EXIT();                              /* inform QK about ISR exit */
}
/*..........................................................................*/
#pragma vector = PORT1_VECTOR
__interrupt void port1_ISR(void) {                           /* for testing */
    static const QEvt tstEvt = { MAX_SIG, 0U, 0U };

#ifdef NDEBUG
    __low_power_mode_off_on_exit();
#endif

    P1IFG &= ~BIT2;                               /* clear interrupt source */

    QK_ISR_ENTRY();                     /* inform QK kernel about ISR entry */

    QACTIVE_POST(AO_Table, &tstEvt, (void *)0);

    QK_ISR_EXIT();                       /* inform QK kernel about ISR exit */
}
/*..........................................................................*/
void BSP_init(void) {
    WDTCTL   = (WDTPW | WDTHOLD);                               /* Stop WDT */

    /* configure the Basic Clock Module */
    DCOCTL   = CALDCO_8MHZ;                              /* Set DCO to 8MHz */
    BCSCTL1  = CALBC1_8MHZ;

    TACTL    = (ID_3 | TASSEL_2 | MC_1);       /* SMCLK, /8 divider, upmode */
    TACCR0   = (((BSP_SMCLK / 8) + BSP_TICKS_PER_SEC/2) / BSP_TICKS_PER_SEC);

    P1DIR   |= (BIT0 | BIT1);               /* P1.0 and P1.1 outputs (LEDs) */

    P1DIR  &= ~BIT2;                             /* P1.2 input (Switch TS1) */
    P1REN  |=  BIT2;                     /* enable pull-up resistor on P1.2 */
    P1SEL  &= ~BIT2;                         /* enable I/O function on P1.2 */
    P1IES  |=  BIT2;                     /* interrupt edge select high->low */
    P1IFG  &= ~BIT2;                              /* clear interrupt source */

    if (QS_INIT((void *)0) == 0) {    /* initialize the QS software tracing */
        Q_ERROR();
    }

    QS_OBJ_DICTIONARY(&l_timerA_ISR);
}
/*..........................................................................*/
void QF_onStartup(void) {
    TACCTL0 = CCIE;                       /* Timer_A CCR0 interrupt enabled */

    P1IE   |= BIT2;                   /* P1.2 interrupt enable (Switch TS1) */
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
void QK_onIdle(void) {

    QF_INT_DISABLE();
    LED1_on();                                    /* switch LED1 on and off */
    LED1_off();
    QF_INT_ENABLE();

#ifdef Q_SPY
    if (((IFG2 & UCA0TXIFG)) != 0) {
        uint16_t b;

        QF_INT_DISABLE();
        b = QS_getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {
            UCA0TXBUF = (uint8_t)b;         /* stick the byte to the TX BUF */
        }
    }
#elif defined NDEBUG
    __low_power_mode_1();            /* Enter LPM1; also UNLOCKS interrupts */
#endif
}
/*..........................................................................*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    (void)file;                                   /* avoid compiler warning */
    (void)line;                                   /* avoid compiler warning */
    QF_INT_DISABLE();             /* make sure that interrupts are disabled */
    for (;;) {
    }
}
/*..........................................................................*/
void BSP_displyPhilStat(uint8_t n, char const *stat) {
    if (n == 0) {                                           /* for Philo[0] */
        if (stat[0] == 'e') {                    /* ON when Philo[0] eating */
            LED0_on();
        }
        else {
            LED0_off();
        }
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n])  /* application-specific record begin */
        QS_U8(1, n);                                  /* Philosopher number */
        QS_STR(stat);                                 /* Philosopher status */
    QS_END()
}

/*--------------------------------------------------------------------------*/
#ifdef Q_SPY
/*..........................................................................*/
uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[256];                    /* buffer for Quantum Spy */
    QS_initBuf(qsBuf, sizeof(qsBuf));

    /* configure USART0 */
    P3SEL   |= BIT4;                                   /* P3.4 = USART0 TXD */
    UCA0CTL1 = UCSSEL_2;                                           /* SMCLK */
    UCA0BR0  = 52;                                        /* 9600 from 8MHz */
    UCA0BR1  = UCBRS0 | UCOS16;
    UCA0MCTL = UCBRS_2;
    UCA0CTL1 &= ~UCSWRST;                  /* initialize USCI state machine */

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

    return (uint8_t)1;                                    /* return success */
}
/*..........................................................................*/
void QS_onCleanup(void) {
}
/*..........................................................................*/
QSTimeCtr QS_onGetTime(void) {            /* invoked with interrupts locked */
    if ((TACTL & TAIFG) == 0) {                   /* interrupt not pending? */
        return QS_tickTime_ + TAR;
    }
    else {      /* the rollover occured, but the timerA_ISR did not run yet */
        return QS_tickTime_
           + (((BSP_SMCLK / 8) + BSP_TICKS_PER_SEC/2) / BSP_TICKS_PER_SEC) + 1
           + TAR;
    }
}
/*..........................................................................*/
void QS_onFlush(void) {
    uint16_t b;
    while ((b = QS_getByte()) != QS_EOD) { /* next QS trace byte available? */
        while ((IFG2 & UCA0TXIFG) == 0) {                  /* TX not ready? */
        }
        UCA0TXBUF = (uint8_t)b;             /* stick the byte to the TX BUF */
    }
}
#endif                                                             /* Q_SPY */
/*--------------------------------------------------------------------------*/
