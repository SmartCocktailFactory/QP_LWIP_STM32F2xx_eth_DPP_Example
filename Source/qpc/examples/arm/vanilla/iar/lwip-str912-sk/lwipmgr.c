/*****************************************************************************
* Product: lwIP-Manager Active Object
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
#define LWIP_ALLOWED

#include "qp_port.h"                                             /* QP-port */
#include "dpp.h"                   /* application events and active objects */
#include "bsp.h"                                   /* Board Support Package */

#include "lwip.h"                                             /* lwIP stack */
#include "httpd.h"                                      /* lwIP application */

#include <string.h>
#include <stdio.h>

//Q_DEFINE_THIS_FILE

            /* application signals cannot overlap the device-driver signals */
Q_ASSERT_COMPILE(MAX_SIG < DEV_DRIVER_SIG);

#define LWIP_SLOW_TICK_MS       TCP_TMR_INTERVAL

#define MAC_ADDR0 0x00
#define MAC_ADDR1 0x02
#define MAC_ADDR2 0x04
#define MAC_ADDR3 0x08
#define MAC_ADDR4 0x0A
#define MAC_ADDR5 0x0D


/* Active object class -----------------------------------------------------*/
typedef struct LwIPMgrTag {
    QActive super;                                   /* derive from QActive */

    QTimeEvt te_LWIP_SLOW_TICK;
    struct netif   *netif;
    struct udp_pcb *upcb;
    uint32_t        ip_addr;    /* IP address in the native host byte order */

#if LWIP_TCP
    uint32_t tcp_tmr;
#endif
#if LWIP_ARP
    uint32_t arp_tmr;
#endif
#if LWIP_DHCP
    uint32_t dhcp_fine_tmr;
    uint32_t dhcp_coarse_tmr;
#endif
#if LWIP_AUTOIP
    uint32_t auto_ip_tmr;
#endif
} LwIPMgr;

static QState LwIPMgr_initial(LwIPMgr *me, QEvt const *e);
static QState LwIPMgr_running(LwIPMgr *me, QEvt const *e);

/* Local objects -----------------------------------------------------------*/
static LwIPMgr l_lwIPMgr;              /* the single instance of LwIPMgr AO */

/* Global-scope objects ----------------------------------------------------*/
QActive * const AO_LwIPMgr = (QActive *)&l_lwIPMgr;     /* "opaque" pointer */

/* Server-Side Include (SSI) demo ..........................................*/
static char const * const ssi_tags[] = {
    "s_xmit",
    "s_rexmit",
    "s_recv",
    "s_fw",
    "s_drop",
    "s_chkerr",
    "s_lenerr",
    "s_memerr",
    "s_rterr",
    "s_proerr",
    "s_opterr",
    "s_err",
};
static int ssi_handler(int iIndex, char *pcInsert, int iInsertLen);

/* Common Gateway Iinterface (CG) demo .....................................*/
static char const *cgi_display(int index, int numParams,
                               char const *param[],
                               char const *value[]);
static tCGI const cgi_handlers[] = {
    { "/display.cgi", &cgi_display },
};

/* UDP handler .............................................................*/
//static void udp_rx_handler(void *arg, struct udp_pcb *upcb,
//                           struct pbuf *p, struct ip_addr *addr, u16_t port);
/*..........................................................................*/
void LwIPMgr_ctor(void) {
    LwIPMgr *me = &l_lwIPMgr;

    QActive_ctor(&me->super, (QStateHandler)&LwIPMgr_initial);
    QTimeEvt_ctor(&me->te_LWIP_SLOW_TICK, LWIP_SLOW_TICK_SIG);
}
/*..........................................................................*/
QState LwIPMgr_initial(LwIPMgr *me, QEvt const *e) {
    uint8_t  macaddr[NETIF_MAX_HWADDR_LEN];

    (void)e;        /* suppress the compiler warning about unused parameter */

    macaddr[0] = (uint8_t)MAC_ADDR0;
    macaddr[1] = (uint8_t)MAC_ADDR1;
    macaddr[2] = (uint8_t)MAC_ADDR2;
    macaddr[3] = (uint8_t)MAC_ADDR3;
    macaddr[4] = (uint8_t)MAC_ADDR4;
    macaddr[5] = (uint8_t)MAC_ADDR5;

                                          /* initialize the Ethernet Driver */
    me->netif = eth_driver_init((QActive *)me, macaddr);

    me->ip_addr = 0xFFFFFFFF;             /* initialize to impossible value */

                                     /* initialize the lwIP applications... */
    httpd_init();         /* initialize the simple HTTP-Deamon (web server) */
    http_set_ssi_handler(&ssi_handler, ssi_tags, Q_DIM(ssi_tags));
    http_set_cgi_handlers(cgi_handlers, Q_DIM(cgi_handlers));

//    me->upcb = udp_new();
//    udp_bind(me->upcb, IP_ADDR_ANY, 777);           /* use port 777 for UDP */
//    udp_recv(me->upcb, &udp_rx_handler, me);

    QS_OBJ_DICTIONARY(&l_lwIPMgr);
    QS_OBJ_DICTIONARY(&l_lwIPMgr.te_LWIP_SLOW_TICK);
    QS_FUN_DICTIONARY(&QHsm_top);
    QS_FUN_DICTIONARY(&LwIPMgr_initial);
    QS_FUN_DICTIONARY(&LwIPMgr_running);

    QS_SIG_DICTIONARY(SEND_UDP_SIG,       (QActive *)me);
    QS_SIG_DICTIONARY(LWIP_SLOW_TICK_SIG, (QActive *)me);
    QS_SIG_DICTIONARY(LWIP_RX_READY_SIG,  (QActive *)me);
    QS_SIG_DICTIONARY(LWIP_TX_READY_SIG,  (QActive *)me);
    QS_SIG_DICTIONARY(LWIP_RX_OVERRUN_SIG,(QActive *)me);

    return Q_TRAN(&LwIPMgr_running);
}
/*..........................................................................*/
QState LwIPMgr_running(LwIPMgr *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            QTimeEvt_postEvery(&me->te_LWIP_SLOW_TICK, (QActive *)me,
                (LWIP_SLOW_TICK_MS * BSP_TICKS_PER_SEC) / 1000);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            QTimeEvt_disarm(&me->te_LWIP_SLOW_TICK);
            return Q_HANDLED();
        }

//        case SEND_UDP_SIG: {
//            if (me->upcb->remote_port != (uint16_t)0) {
//                struct pbuf *p = pbuf_new((u8_t *)((TextEvt const *)e)->text,
//                                      strlen(((TextEvt const *)e)->text) + 1);
//                if (p != (struct pbuf *)0) {
//                    udp_send(me->upcb, p);
//                }
//            }
//            return Q_HANDLED();
//        }

        case LWIP_RX_READY_SIG: {
            eth_driver_read();
            return Q_HANDLED();
        }
        case LWIP_TX_READY_SIG: {
            eth_driver_write();
            return Q_HANDLED();
        }
        case LWIP_SLOW_TICK_SIG: {
                                                 /* has IP address changed? */
            if (me->ip_addr != me->netif->ip_addr.addr) {
                TextEvt *te;
                uint32_t ip_net;    /* IP address in the network byte order */

                me->ip_addr = me->netif->ip_addr.addr; /* save the IP addr. */
                ip_net  = ntohl(me->ip_addr);
                    /* publish the text event to display the new IP address */
                te = Q_NEW(TextEvt, DISPLAY_IPADDR_SIG);
                snprintf(te->text, Q_DIM(te->text), "%d.%d.%d.%d",
                         ((ip_net) >> 24) & 0xFF,
                         ((ip_net) >> 16) & 0xFF,
                         ((ip_net) >> 8)  & 0xFF,
                         ip_net           & 0xFF);
                QF_publish((QEvt *)te);
            }

#if LWIP_TCP
            me->tcp_tmr += LWIP_SLOW_TICK_MS;
            if (me->tcp_tmr >= TCP_TMR_INTERVAL) {
                me->tcp_tmr = 0;
                tcp_tmr();
            }
#endif
#if LWIP_ARP
            me->arp_tmr += LWIP_SLOW_TICK_MS;
            if (me->arp_tmr >= ARP_TMR_INTERVAL) {
                me->arp_tmr = 0;
                etharp_tmr();
            }
#endif
#if LWIP_DHCP
            me->dhcp_fine_tmr += LWIP_SLOW_TICK_MS;
            if (me->dhcp_fine_tmr >= DHCP_FINE_TIMER_MSECS) {
                me->dhcp_fine_tmr = 0;
                dhcp_fine_tmr();
            }
            me->dhcp_coarse_tmr += LWIP_SLOW_TICK_MS;
            if (me->dhcp_coarse_tmr >= DHCP_COARSE_TIMER_MSECS) {
                me->dhcp_coarse_tmr = 0;
                dhcp_coarse_tmr();
            }
#endif
#if LWIP_AUTOIP
            me->auto_ip_tmr += LWIP_SLOW_TICK_MS;
            if (me->auto_ip_tmr >= AUTOIP_TMR_INTERVAL) {
                me->auto_ip_tmr = 0;
                autoip_tmr();
            }
#endif
            return Q_HANDLED();
        }
        case LWIP_RX_OVERRUN_SIG: {
            LINK_STATS_INC(link.err);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}

/* HTTPD customizations ----------------------------------------------------*/

/* Server-Side Include (SSI) handler .......................................*/
static int ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
    struct stats_proto *stats = &lwip_stats.link;
    STAT_COUNTER value;

    switch (iIndex) {
        case 0:                                                 /* s_xmit   */
            value = stats->xmit;
            break;
        case 1:                                                 /* s_rexmit */
            value = stats->rexmit;
            break;
        case 2:                                                 /* s_recv   */
            value = stats->recv;
            break;
        case 3:                                                 /* s_fw     */
            value = stats->fw;
            break;
        case 4:                                                 /* s_drop   */
            value = stats->drop;
            break;
        case 5:                                                 /* s_chkerr */
            value = stats->chkerr;
            break;
        case 6:                                                 /* s_lenerr */
            value = stats->lenerr;
            break;
        case 7:                                                 /* s_memerr */
            value = stats->memerr;
            break;
        case 8:                                                 /* s_rterr  */
            value = stats->rterr;
            break;
        case 9:                                                 /* s_proerr */
            value = stats->proterr;
            break;
        case 10:                                                /* s_opterr */
            value = stats->opterr;
            break;
        case 11:                                                /* s_err    */
            value = stats->err;
            break;
    }

    return snprintf(pcInsert, MAX_TAG_INSERT_LEN, "%d", value);
}

/* Common Gateway Iinterface (CG) handler ..................................*/
static char const *cgi_display(int index, int numParams,
                               char const *param[],
                               char const *value[])
{
    int i;
    for (i = 0; i < numParams; ++i) {
        if (strstr(param[i], "text") != (char *)0) {   /* param text found? */
            TextEvt *te = Q_NEW(TextEvt, DISPLAY_CGI_SIG);
            strncpy(te->text, value[i], Q_DIM(te->text));
            QF_publish((QEvt *)te);
            return "/thank_you.htm";
        }
    }
    return (char *)0;/*no URI, HTTPD will send 404 error page to the browser*/
}

/* UDP receive handler -----------------------------------------------------*/
//static void udp_rx_handler(void *arg, struct udp_pcb *upcb,
//                           struct pbuf *p, struct ip_addr *addr, u16_t port)
//{
//    TextEvt *te = Q_NEW(TextEvt, DISPLAY_UDP_SIG);
//    strncpy(te->text, (char *)p->payload, Q_DIM(te->text));
//    QF_publish((QEvt *)te);
//
//    udp_connect(upcb, addr, port);            /* connect to the remote host */
//    pbuf_free(p);                                   /* don't leak the pbuf! */
//}

