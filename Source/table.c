/*****************************************************************************
* Product: DPP example
* Last Updated for Version: 4.2.00
* Date of the Last Update:  Jul 14, 2011
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
#include "qp_port.h"                                 /* QP port header file */
#include "dpp.h"                   /* application events and active objects */
#include "bsp.h"                       /* Board Support Package header file */
#include "stm32_eval.h"
#include <stdio.h>


Q_DEFINE_THIS_FILE

/* Active object class -----------------------------------------------------*/
typedef struct TableTag {
    QActive super;
    uint8_t fork[N_PHILO];
    uint8_t isHungry[N_PHILO];
    uint8_t udpCtr;
} Table;

static QState Table_initial(Table *me, QEvent const *e);
static QState Table_serving(Table *me, QEvent const *e);

static void Table_displayInit(Table *me);
static void Table_displayPhilStat(Table *me, uint8_t n, char const *stat);
static void Table_displayIPAddr(Table *me, char const *ip_addr);
static void Table_displayUdpText(Table *me, char const *text);
static void Table_displayCgiText(Table *me, char const *text);

#define RIGHT(n_) ((uint8_t)(((n_) + (N_PHILO - 1)) % N_PHILO))
#define LEFT(n_)  ((uint8_t)(((n_) + 1) % N_PHILO))
enum ForkState { FREE, USED };

#ifdef Q_SPY
enum AppRecords {                  /* application-specific QS trace records */
    PHILO_STAT = QS_USER,
    CGI_TEXT,
    UDP_TEXT,
};
#endif

/* Local objects -----------------------------------------------------------*/
static Table l_table;     /* the single instance of the Table active object */

/* Global-scope objects ----------------------------------------------------*/
QActive * const AO_Table = (QActive *)&l_table;      /* "opaque" AO pointer */

/*..........................................................................*/
void Table_ctor(void) {
    uint8_t n;
    Table *me = &l_table;

    QActive_ctor(&me->super, (QStateHandler)&Table_initial);

    for (n = 0; n < N_PHILO; ++n) {
        me->fork[n] = FREE;
        me->isHungry[n] = 0;
    }
    me->udpCtr = 0;
}
/*..........................................................................*/
QState Table_initial(Table *me, QEvent const *e) {
    (void)e;        /* suppress the compiler warning about unused parameter */

    /* Initialize the OLED display */
    Table_displayInit(me);

    QActive_subscribe((QActive *)me, DONE_SIG);
    QActive_subscribe((QActive *)me, DISPLAY_IPADDR_SIG);
    QActive_subscribe((QActive *)me, DISPLAY_CGI_SIG);
    QActive_subscribe((QActive *)me, DISPLAY_UDP_SIG);

    QS_OBJ_DICTIONARY(&l_table);
    QS_FUN_DICTIONARY(&QHsm_top);
    QS_FUN_DICTIONARY(&Table_initial);
    QS_FUN_DICTIONARY(&Table_serving);

    QS_SIG_DICTIONARY(DONE_SIG,            0);            /* global signals */
    QS_SIG_DICTIONARY(EAT_SIG,             0);
    QS_SIG_DICTIONARY(DISPLAY_IPADDR_SIG,  0);
    QS_SIG_DICTIONARY(DISPLAY_CGI_SIG,     0);
    QS_SIG_DICTIONARY(DISPLAY_UDP_SIG,     0);

    QS_SIG_DICTIONARY(HUNGRY_SIG,          me);    /* signal just for Table */
	QS_SIG_DICTIONARY(TERMINATE_SIG, 	   0);


    QActive_subscribe((QActive *)me, DONE_SIG);
    QActive_subscribe((QActive *)me, TERMINATE_SIG);

    return Q_TRAN(&Table_serving);
}
/*..........................................................................*/
QState Table_serving(Table *me, QEvent const *e) {
    uint8_t n, m;
    TableEvt *pe;

    switch (e->sig) {
        case HUNGRY_SIG: {
            n = ((TableEvt const *)e)->philoNum;
                      /* phil ID must be in range and he must be not hungry */
            Q_ASSERT((n < N_PHILO) && (!me->isHungry[n]));

            Table_displayPhilStat(me, n, "hungry  ");
            m = LEFT(n);
            if ((me->fork[m] == FREE) && (me->fork[n] == FREE)) {
                me->fork[m] = me->fork[n] = USED;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = n;
                QF_PUBLISH((QEvent *)pe, me);
                Table_displayPhilStat(me, n, "eating  ");
            }
            else {
                me->isHungry[n] = 1;
            }
            return Q_HANDLED();
        }
        case DONE_SIG: {
            n = ((TableEvt const *)e)->philoNum;
                      /* phil ID must be in range and he must be not hungry */
            Q_ASSERT((n < N_PHILO) && (!me->isHungry[n]));

            Table_displayPhilStat(me, n, "thinking");
            m = LEFT(n);
                                      /* both forks of Phil[n] must be used */
            Q_ASSERT((me->fork[n] == USED) && (me->fork[m] == USED));

            me->fork[m] = me->fork[n] = FREE;
            m = RIGHT(n);                       /* check the right neighbor */
            if (me->isHungry[m] && (me->fork[m] == FREE)) {
                me->fork[n] = me->fork[m] = USED;
                me->isHungry[m] = 0;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QF_PUBLISH((QEvent *)pe, me);
                Table_displayPhilStat(me, m, "eating  ");
            }
            m = LEFT(n);                         /* check the left neighbor */
            n = LEFT(m);                  /* left fork of the left neighbor */
            if (me->isHungry[m] && (me->fork[n] == FREE)) {
                me->fork[m] = me->fork[n] = USED;
                me->isHungry[m] = 0;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QF_PUBLISH((QEvent *)pe, me);
                Table_displayPhilStat(me, m, "eating  ");
            }
            return Q_HANDLED();
        }
		case TERMINATE_SIG: {
            QF_stop();
            return Q_HANDLED();
        }
        case DISPLAY_IPADDR_SIG: {
            Table_displayIPAddr(me, ((TextEvt *)e)->text);
            return Q_HANDLED();
        }
        case DISPLAY_CGI_SIG: {
            Table_displayCgiText(me, ((TextEvt *)e)->text);
            return Q_HANDLED();
        }
        case DISPLAY_UDP_SIG: {
            TextEvt *te;

            Table_displayUdpText(me, ((TextEvt *)e)->text);
            ++me->udpCtr;

            te = Q_NEW(TextEvt, SEND_UDP_SIG);
            snprintf(te->text, Q_DIM(te->text), "%s-%d",
                     ((TextEvt const *)e)->text, (int)me->udpCtr);
            QACTIVE_POST(AO_LwIPMgr, (QEvent *)te, me);    /* post directly */

            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}

/* helper functions for the display ........................................*/
                     
void Table_displayInit(Table *me) {
    /* init LCD display here, if available */

    if (QS_INIT((void *)0) == 0) {    /* initialize the QS software tracing */
        Q_ERROR();
    }

#if 0  /* alu: disabled */
    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
#endif
}

/*..........................................................................*/
static void Table_displayPhilStat(Table *me, uint8_t n, char const *stat) {
#if 0  /* alu: disabled because no LCD display is available */
	LCD_DisplayChar(Line6, (3*16*n + 5*16), stat[0]);
#endif
    QS_BEGIN(PHILO_STAT, AO_Philo[n])  /* application-specific record begin */
        QS_U8(1, n);                                  /* Philosopher number */
        QS_STR(stat);                                 /* Philosopher status */
    QS_END()
}
/*..........................................................................*/
static void Table_displayIPAddr(Table *me, char const *ip_addr) {
#if 0  /* alu: disabled because no LCD display is available */
    LCD_DisplayStringLine(Line3, ip_addr                 );
#endif
}
/*..........................................................................*/
void Table_displayCgiText(Table *me, char const *text) {
#if 0  /* alu: disabled because no LCD display is available */
    LCD_DisplayStringLine(Line4, text                 );
#endif
    QS_BEGIN(CGI_TEXT, 0)              /* application-specific record begin */
        QS_STR(text);                                          /* User text */
    QS_END()
}
/*..........................................................................*/
void Table_displayUdpText(Table *me, char const *text) {
#if 0  /* alu: disabled because no LCD display is available */
   LCD_DisplayStringLine(Line5, text                 );
#endif
    QS_BEGIN(UDP_TEXT, 0)              /* application-specific record begin */
        QS_STR(text);                                          /* User text */
    QS_END()
}


