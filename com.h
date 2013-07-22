/**
 ******************************************************************************
 *
 * @file       com.h
 * @author     Lonewolf
 * @brief      COM Header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#ifndef _COM_H_
#define _COM_H_

#include "settings.h"

#define COM_RX_TIMEOUT		(BSP_TICKS_PER_SEC * 10)/1000 // 10ms
#define COM_RX_BUFFER_SIZE 	100
#define COM_TX_BUFFER_SIZE	100

typedef struct Com_tag
{
    QActive super;
    /* Public Members */
    QActive* master;
    struct
    {
        uint8_t rx_timeout;
    } config;
    struct
    {
        uint8_t *p_data;
        uint8_t size;
        uint8_t payload_size;
    } tx;
    struct
    {
        uint8_t *p_data;
        uint8_t size;
        volatile uint8_t timeout;
    } rx;
} Com;

void Com_init(uint8_t *tx_buffer, uint8_t tx_size, uint8_t *rx_buffer, uint8_t rx_size);


#endif                 /* com.h */