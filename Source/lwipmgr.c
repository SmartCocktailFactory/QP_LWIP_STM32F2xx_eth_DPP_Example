/*****************************************************************************
* Product: lwIP-Manager Active Object
* Last Updated for Version: 4.2.00
* Date of the Last Update:  Jul 25, 2011
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
#define LWIP_ALLOWED

#include "qp_port.h"                                             /* QP-port */
#include "dpp.h"                   /* application events and active objects */
#include "bsp.h"                                   /* Board Support Package */

#include "lwip.h"                                             /* lwIP stack */
#include "httpd.h"                                      /* lwIP application */

#include <string.h>
#include <stdio.h>

Q_DEFINE_THIS_FILE

            /* application signals cannot overlap the device-driver signals */
Q_ASSERT_COMPILE(MAX_SIG < DEV_DRIVER_SIG);

#define FLASH_USERREG0          (*(uint32_t const *)0x400FE1E0)
#define FLASH_USERREG1          (*(uint32_t const *)0x400FE1E4)
#define LWIP_SLOW_TICK_MS       TCP_TMR_INTERVAL

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

static QState LwIPMgr_initial(LwIPMgr *me, QEvent const *e);
static QState LwIPMgr_running(LwIPMgr *me, QEvent const *e);

/* Local objects -----------------------------------------------------------*/
static LwIPMgr l_lwIPMgr;              /* the single instance of LwIPMgr AO */

/* Global-scope objects ----------------------------------------------------*/
QActive * const AO_LwIPMgr = (QActive *)&l_lwIPMgr;     /* "opaque" pointer */

/* Server-Side Include (SSI) demo ..........................................*/
static char const * const ssi_tags[] = {
    "s_xmit",
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
static void udp_rx_handler(void *arg, struct udp_pcb *upcb,
                           struct pbuf *p, struct ip_addr *addr, u16_t port);
/*..........................................................................*/
void LwIPMgr_ctor(void) {
    LwIPMgr *me = &l_lwIPMgr;

    QActive_ctor(&me->super, (QStateHandler)&LwIPMgr_initial);
    QTimeEvt_ctor(&me->te_LWIP_SLOW_TICK, LWIP_SLOW_TICK_SIG);
}
/*..........................................................................*/
QState LwIPMgr_initial(LwIPMgr *me, QEvent const *e) {
    uint8_t  macaddr[NETIF_MAX_HWADDR_LEN];

    (void)e;        /* suppress the compiler warning about unused parameter */

    /* Configure the hardware MAC address for the Ethernet Controller */

    /*
     * Set up the MAC address and make sure it's not all FF values
     * TODO: This will evetually be read somewhere
     */

    /* the MAC address must have been programmed! */
    Q_ASSERT((STATIC_IPADDR0 != 0xFF) &&
    		 (STATIC_IPADDR1 != 0xFF) &&
    		 (STATIC_IPADDR2 != 0xFF) &&
    		 (STATIC_IPADDR3 != 0xFF));

    macaddr[0] = DEF_MAC_ADDR0;
    macaddr[1] = DEF_MAC_ADDR1;
    macaddr[2] = DEF_MAC_ADDR2;
    macaddr[3] = DEF_MAC_ADDR3;
    macaddr[4] = DEF_MAC_ADDR4;
    macaddr[5] = DEF_MAC_ADDR5;

                                          /* initialize the Ethernet Driver */
    me->netif = eth_driver_init((QActive *)me, macaddr);

    me->ip_addr = 0xFFFFFFFF;             /* initialize to impossible value */

                                     /* initialize the lwIP applications... */
    httpd_init();         /* initialize the simple HTTP-Deamon (web server) */
    http_set_ssi_handler(&ssi_handler, ssi_tags, Q_DIM(ssi_tags));
    http_set_cgi_handlers(cgi_handlers, Q_DIM(cgi_handlers));

    me->upcb = udp_new();
    udp_bind(me->upcb, IP_ADDR_ANY, 777);           /* use port 777 for UDP */
    udp_recv(me->upcb, &udp_rx_handler, me);

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
QState LwIPMgr_running(LwIPMgr *me, QEvent const *e) {
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

        case SEND_UDP_SIG: {
            if (me->upcb->remote_port != (uint16_t)0) {
                struct pbuf *p = pbuf_new((u8_t *)((TextEvt const *)e)->text,
                                      strlen(((TextEvt const *)e)->text) + 1);
                if (p != (struct pbuf *)0) {
                    udp_send(me->upcb, p);
                    printf("Sent: %s\n", ((TextEvt const *)e)->text);
                    pbuf_free(p);                   /* don't leak the pbuf! */
                }
            }
            return Q_HANDLED();
        }

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
                QF_PUBLISH((QEvent *)te, me);
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
    STAT_COUNTER value = 0;

    switch (iIndex) {
        case 0:                                                 /* s_xmit   */
            value = stats->xmit;
            break;
        case 1:                                                 /* s_recv   */
            value = stats->recv;
            break;
        case 2:                                                 /* s_fw     */
            value = stats->fw;
            break;
        case 3:                                                 /* s_drop   */
            value = stats->drop;
            break;
        case 4:                                                 /* s_chkerr */
            value = stats->chkerr;
            break;
        case 5:                                                 /* s_lenerr */
            value = stats->lenerr;
            break;
        case 6:                                                 /* s_memerr */
            value = stats->memerr;
            break;
        case 7:                                                 /* s_rterr  */
            value = stats->rterr;
            break;
        case 8:                                                 /* s_proerr */
            value = stats->proterr;
            break;
        case 9:                                                 /* s_opterr */
            value = stats->opterr;
            break;
        case 10:                                                /* s_err    */
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
            QF_PUBLISH((QEvent *)te, AO_LwIPMgr);
            return "/thank_you.htm";
        }
    }
    return (char *)0;/*no URI, HTTPD will send 404 error page to the browser*/
}

/* UDP receive handler -----------------------------------------------------*/
static void udp_rx_handler(void *arg, struct udp_pcb *upcb,
                           struct pbuf *p, struct ip_addr *addr, u16_t port)
{
    TextEvt *te = Q_NEW(TextEvt, DISPLAY_UDP_SIG);
    strncpy(te->text, (char *)p->payload, Q_DIM(te->text));
    QF_PUBLISH((QEvent *)te, AO_LwIPMgr);

    udp_connect(upcb, addr, port);            /* connect to the remote host */
    pbuf_free(p);                                   /* don't leak the pbuf! */
}

