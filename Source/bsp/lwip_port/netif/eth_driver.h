/*****************************************************************************
 * lwIP Ethernet CMSIS-compliant driver for STM3220G-eval for use
 * with Quantum Leaps (www.state-machine.com) kernel.
 * Copyright Datacard Corporation 1999-2012. All rights reserved.
 * Author: Harry Rostovtsev
 * Email:  harry_rostovtsev at datacard.com, rost0031 at gmail.com
 ****************************************************************************/

/*****************************************************************************
* Product: Ethernet driver for lwIP with QP
* Last Updated for Version: 4.0.03
* Date of the Last Update:  Mar 23, 2009
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2009 Quantum Leaps, LLC. All rights reserved.
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
#ifndef eth_driver_h
#define eth_driver_h
/****************************************************************************/
/*                             QP includes                                  */
/****************************************************************************/
#include "qp_port.h"

/****************************************************************************/
/*                            LwIp includes                                 */
/****************************************************************************/
#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "netif/etharp.h"

/****************************************************************************/
/*                         ST Library includes                              */
/****************************************************************************/
#include "bsp.h"
#include "stm32f2xx.h"
#include "stm32f2x7_eth.h"
#include "stm32f2x7_eth_bsp.h"


/****************************************************************************/
/*                            General includes                              */
/****************************************************************************/
/* TODO: Maybe get rid of this so we use
 * the memcpy defined by the cortex-m3 assembly code
 */
#include <string.h>                     /* for memcpy() */


/****************************************************************************/
/*                                Defines                                   */
/****************************************************************************/

/* Setup processing for PTP (IEEE-1588). */
#if LWIP_PTPD
void lwIPHostGetTime(u32_t *time_s, u32_t *time_ns);
#endif

#ifndef TX_PBUF_QUEUE_LEN
#define TX_PBUF_QUEUE_LEN 8
#endif

#define PbufQueue_isEmpty(me_) ((me_)->qwrite == (me_)->qread)

/****************************************************************************/
/*                            Local variables                               */
/****************************************************************************/
/* Helper queue of pbufs */
typedef struct PbufQueueTag {
    struct pbuf *ring[TX_PBUF_QUEUE_LEN];
    uint8_t qwrite;
    uint8_t qread;
    uint8_t overflow;
} PbufQueue;

enum EthDriverSignals {
    LWIP_SLOW_TICK_SIG = DEV_DRIVER_SIG,
    LWIP_RX_READY_SIG,
    LWIP_TX_READY_SIG,
    LWIP_RX_OVERRUN_SIG
};

/****************************************************************************/
/*                           Function Declarations                          */
/****************************************************************************/

struct netif *eth_driver_init(QActive *active, u8_t macaddr[NETIF_MAX_HWADDR_LEN]);
void eth_driver_read(void);
void eth_driver_write(void);

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif);


/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
void low_level_init(struct netif *netif);


err_t ethernetif_output(struct netif *netif, struct pbuf *p);

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
struct pbuf *low_level_receive(void);

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
void low_level_transmit(struct netif *netif, struct pbuf *p);

#ifdef Q_SPY
    static uint8_t l_Ethernet_IRQHandler;
#endif

#endif                                                      /* eth_driver_h */
