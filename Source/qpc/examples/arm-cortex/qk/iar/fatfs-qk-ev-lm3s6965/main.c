/*****************************************************************************
* Product: FatFS application
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
#include "fatfsmgr.h"
#include "bsp.h"

/* Local-scope objects -----------------------------------------------------*/
static QEvt const *l_FatFsMgrQueueSto[10];
static QSubscrList   l_subscrSto[MAX_PUB_SIG];

static union SmallEvents {
    void   *e0;                                       /* minimum event size */
    uint8_t e1[sizeof(QEvt)];
    uint8_t e2[sizeof(TableEvt)];
    /* ... other event types to go into this pool */
} l_smlPoolSto[20];                     /* storage for the small event pool */

static union MediumEvents {
    void   *e0;                                       /* minimum event size */
    uint8_t e1[sizeof(TextEvt)];
    /* ... other event types to go into this pool */
} l_medPoolSto[4];                     /* storage for the medium event pool */

/*..........................................................................*/
int main(void) {

    FatFsMgr_ctor();         /* instantiate all FatFS-Manager active object */

    BSP_init();                     /* initialize the Board Support Package */

    QF_init();     /* initialize the framework and the underlying RT kernel */

                                                  /* object dictionaries... */
    QS_OBJ_DICTIONARY(l_smlPoolSto);
    QS_OBJ_DICTIONARY(l_medPoolSto);
    QS_OBJ_DICTIONARY(l_FatFsMgrQueueSto);

    QF_psInit(l_subscrSto, Q_DIM(l_subscrSto));   /* init publish-subscribe */

                                               /* initialize event pools... */
    QF_poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));
    QF_poolInit(l_medPoolSto, sizeof(l_medPoolSto), sizeof(l_medPoolSto[0]));

    QActive_start(AO_FatFsMgr, 1,
                  l_FatFsMgrQueueSto, Q_DIM(l_FatFsMgrQueueSto),
                  (void *)0, 0, (QEvt *)0);

    return QF_run();                              /* run the QF application */
}
