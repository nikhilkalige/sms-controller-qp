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



Com com_drv;

static QState Com_initial(Com *const me);
static QState Com_open(Com *const me);

static void open();

/************************************************************************************************************************************
                                                    ***** Public Funtions *****
************************************************************************************************************************************/

void Com_init(uint8_t *tx_buffer, uint8_t tx_size, uint8_t *rx_buffer, uint8_t rx_size)
{
    com_drv.tx.p_data = tx_buffer;
    com_drv.tx.size = tx_size;
    com_drv.rx.p_data = rx_buffer;
    com_drv.rx.size = rx_size;
    com_drv.config.rx_timeout = 10;
    Serial_init(com_drv.rx.p_data, com_drv.rx.size, &com_drv.config.rx_timeout, com_drv.tx.p_data, com_drv.tx.size, &com_drv.tx.payload_size, &com_drv);

}

/************************************************************************************************************************************
                                                    ***** Private Funtions *****
************************************************************************************************************************************/

static void open()
{
    com_drv.rx.timeout = 0xFF;
    QActive_arm((QActive *)&com_drv, COM_RX_TIMEOUT);
    Serial_config();
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
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case EVENT_COM_OPEN_REQUEST:
        {
            open();
            status = Q_HANDLED();
            break;
        }
        case EVENT_COM_SEND_REQUEST:
        {
            send();
            status = Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_arm((QActive *) me, COM_RX_TIMEOUT);
            cli();
            if(me->rx.timeout != 0xFF)
            {
                if((++me->rx.timeout) == me->config.rx_timeout)
                {
                    QActive_post((QActive *)me->master, EVENT_COM_DATA_AVAILABLE, 0);
                }
            }
            sei();
            status = Q_HANDLED();
            break;
        }
        case EVENT_SERIAL_SEND_DONE:
        {
            QActive_post((QActive *)me->master, EVENT_COM_SEND_DONE, 0);
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}


