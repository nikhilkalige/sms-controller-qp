/**
 ******************************************************************************
 *
 * @file       com.c
 * @author     Lonewolf
 * @brief      COM Driver functions
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#include "com.h"
#include "serial.h"
#include "softserial.h"

static QState Com_initial(Com *const me);
static QState Com_open(Com *const me);

static void open();

/************************************************************************************************************************************
                                                    ***** Public Funtions *****
************************************************************************************************************************************/

void Com_ctor()
{
    QActive_ctor(&com_drv.super, Q_STATE_CAST(&Com_initial));
}

void Com_init(QActive *master, uint8_t *tx_buffer, uint8_t tx_size, uint8_t *rx_buffer, uint8_t rx_size)
{
    com_drv.master = master;
    com_drv.tx.p_data = tx_buffer;
    com_drv.tx.size = tx_size;
    com_drv.rx.p_data = rx_buffer;
    com_drv.rx.size = rx_size;
    com_drv.config.rx_timeout = 20;
    Serial_init(com_drv.rx.p_data, com_drv.rx.size, (uint8_t *)&com_drv.rx.timeout, com_drv.tx.p_data, com_drv.tx.size, &com_drv.tx.payload_size, (QActive *)&com_drv);
}

void Com_reopen()
{
    com_drv.rx.timeout = 0xFF;
    Serial_restart_rx();
}
/************************************************************************************************************************************
                                                    ***** Private Funtions *****
************************************************************************************************************************************/

static void open()
{
    com_drv.rx.timeout = 0xFF;
    QActive_arm((QActive *)&com_drv, COM_RX_TIMEOUT);
    Serial_config();
    QActive_post((QActive *)com_drv.master, EVENT_COM_OPEN_DONE, 0);
}

static void send()
{
    Serial_enable_transmitter();
}


/************************************************************************************************************************************
                                                    ***** State Machines *****
************************************************************************************************************************************/

static QState Com_initial(Com *const me)
{
    return Q_TRAN(&Com_open);
}

static QState Com_open(Com *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_COM_OPEN_REQUEST:
        {
            open();
            return Q_HANDLED();
        }
        case EVENT_COM_SEND_REQUEST:
        {
            send();
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_arm((QActive *)me, COM_RX_TIMEOUT);
            cli();
            if (me->rx.timeout != 0xFF)
            {
                if ((++me->rx.timeout) == me->config.rx_timeout)
                {
                    uint8_t size;
                    size = Serial_read_size();
                    QActive_post((QActive *)me->master, EVENT_COM_DATA_AVAILABLE, size);
                }
            }
            sei();
            return Q_HANDLED();
        }
        case EVENT_SERIAL_SEND_DONE:
        {
            QActive_post((QActive *)me->master, EVENT_COM_SEND_DONE, 0);
            return Q_HANDLED();
        }
        default:
        {
            return Q_SUPER(&QHsm_top);
        }
    }
}