/**
 ******************************************************************************
 *
 * @file       gsm.c
 * @author     Lonewolf
 * @brief      GSM functions
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/


#include "gsm.h"
#include "com.h"
#include <gsm_settings.h>

#define RX_NOT_STARTED      0
#define RX_ALREADY_STARTED  1
uint8_t com_buff[COM_BUFFER_LENGTH];
typedef struct Gsm_tag
{
    QActive super;
    /* Public Members */
    Com *p_comm;
    QActive *master;
    fifobuf buffer;
    volatile uint8_t current_op;
    volatile uint8_t tx_buffer_full;
    struct
    {
        uint8_t response;
        uint8_t ix;
        uint8_t busy;
        uint8_t timeout;
        uint8_t *master_buffer;
    } control;
} Gsm;

static Gsm gsm_dev;


static QState GSM_initial(Gsm *const me);
static QState GSM_input(Gsm *const me);
static QState GSM_transmit(Gsm *const me);
static QState GSM_recieve(Gsm *const me);
static QState GSM_process(Gsm *const me);

static void comm_rx_handler(void);
static uint8_t check_result_codes(uint8_t *p_in, uint8_t* message_id);
static void parse_input_command(uint8_t result_index);
//static uint8_t parse_command(uint8_t* p_data, struct at_response_code* table, uint8_t* message_id);
static void send_command_timeout(uint8_t* data, uint16_t timeout, uint8_t in_flash);
static void send_command(uint8_t* data, uint8_t in_flash);
static void stop_command_timout(void);
static void sms_send_command();
static void sms_delete_command(uint8_t pos);
static void sms_delete_all_command();

static void network_status_read_command();
#if 0
/* Functions related to GSM */
static QState process_command();
static void trasmit_command();
static void process_data(void *next_state);
static uint8_t compare_string(char const *buffer, char const *string);
static void set_timeouts(GSM_timeouts start_comm_tmout, GSM_timeouts max_interchar_tmout);
void init_statemachine();
//extern uint8_t commandFlag;
//static uint8_t gsmBuffer[GSM_BUFFER_SIZE];
#endif

static QState init_initial(Gsm *const me);
static QState inactive_super(Gsm *const me);
static QState inactive_systemsetup(Gsm *const me);
static QState inactive_opencom(Gsm *const me);
static QState inactive_powering_on(Gsm *const me);
static QState inactive_check_comlink(Gsm *const me);
static QState inactive_device_config(Gsm *const me);

static QState active_super(Gsm *const me);
static QState active_initial(Gsm *const me);
static QState active_idle(Gsm *const me);
static QState active_network_status(Gsm *const me);
static QState active_time_get(Gsm *const me);
static QState active_time_set(Gsm *const me);
static QState active_sms_delete(Gsm *const me);
static QState active_sms_send(Gsm *const me);
static QState active_sms_read(Gsm *const me);
static QState active_sms_presence(Gsm *const me);

static QState init_inactive(Gsm *const me);
static QState init_powering_on(Gsm *const me);
static QState init_device_config(Gsm *const me);
static QState active_super(Gsm *const me);

static QState active_initial(Gsm *const me);
static QState on_active(Gsm *const me);
static QState on_active_cmd(Gsm *const me);





/************************************************************************************************************************************
                                                    ***** Public Funtions *****
************************************************************************************************************************************/



/************************************************************************************************************************************
                                                    ***** Private Funtions *****
************************************************************************************************************************************/

static void comm_rx_handler(void)
{
    uint8_t temp, rd;
    rd = gsm_dev.buffer.wr;
    temp = gsm_dev.buffer.buf_ptr[rd];
    uint8_t *p_data = gsm_dev.p_comm->rx.p_data;
    uint8_t *p_str;
    uint8_t *p_end;
    uint8_t size = 10;
    uint8_t i, str_size, parsed_command,message_id;
    i = 0;
    /* Search for \r\n<Result codes>\r\n */
    while(i < size)
    {
        if(p_data[i++] == '\r')
        {
            if(p_data[i++] == '\n')
            {
                p_str = &p_data[i];
                p_end = (uint8_t*)memchr(p_str, '\r', (size - i));
                if(p_end == 0)
                {
                    break;
                }
                str_size = p_end - p_str;
                /* Setup parameters for next round */
                i+= 1 + str_size;
                if(p_data[i] == '\n')
                {
                    i++;
                }
                //result_index = check_result_codes(p_str);
                parsed_command = check_result_codes(p_str, &message_id);
                switch(parsed_command)
                {
                    case EVENT_GSM_SMS_RESPONSE:
                    {
                        /* Pass the message id*/
                        QActive_post((QActive *)&gsm_dev, parsed_command, (uint32_t)((message_id << 16) + p_str));
                        break;
                    }
                    case EVENT_GSM_CLOCK_RESPONSE:
                    {
                        QActive_post((QActive *)&gsm_dev, parsed_command, (uint32_t)((message_id << 16) + p_str));
                        break;
                    }
                    case 0:
                    {
                        /* Handler for no event*/
                        break;
                    }
                    default:
                    {
                        /*  Handles the following events
                        *   EVENT_GSM_NETWORK_RESPONSE
                        *
                        *
                        */
                        QActive_post((QActive *)&gsm_dev, parsed_command, (uint32_t)((message_id << 16)));
                        if(parsed_command == EVENT_GSM_ACK_RESPONSE)
                        {
                            stop_command_timout();
                        }
                        break;
                    }
                } // End of switch
            }
        }
    } // while i<size
    /* TODO: make provisions regarding open/close clear of rx buffer */
}

static uint8_t parse_command(uint8_t* p_data, struct at_response_code *table, uint8_t* message_id)
{
    uint8_t i = 0;
    uint8_t j;
    do
    {
        j = strlen((char*)(table[i].p_string));

        if((memcmp(table[i].p_string, p_data, j)) == 0)
        {
            *message_id = table[i].message_id;
            return table[i].event_id;
        }
        i++;
    }
    while(table[i].p_string != 0);
    return 0;
}

static uint8_t check_result_codes(uint8_t *p_in, uint8_t* message_id)
{
    // uint8_t i = 0;
    // uint8_t j;
    // uint8_t error_code[4];
    uint8_t event_id;
    const struct  at_response_code *p_table;
    switch (gsm_dev.current_op)
    {
        case GSM_SMS_OP:
        {
            p_table = &sms_table[0];
            break;
        }
        case GSM_NETWORK_OP:
        {
            p_table = &network_table[0];
            break;
        }
        case GSM_ACC_OP:
        {
            p_table = &acc_table[0];
            break;
        }
        default:
        {
            p_table = &frc_table[0];
            break;
        }
    }
    event_id = parse_command(p_in, p_table, message_id);
    if(!event_id)
    {
        event_id = parse_command(p_in,  &frc_table[0], message_id);
    }
    return event_id;
}

static void send_command(uint8_t* data, uint8_t in_flash)
{
    send_command_timeout(data, 1000, in_flash);
}

static void send_command_timeout(uint8_t* data, uint16_t timeout, uint8_t in_flash)
{
    /* Fill up the buffer and send event */
    uint8_t length;
    if(in_flash)
    {
        length = strlen_P((char *)data);
    }
    else
    {
        length = strlen((char *)data);
    }
    if((gsm_dev.p_comm->tx.payload_size + length) > gsm_dev.p_comm->tx.size)
    {
        /* Raise tx buffer full flag and request timeout */
        gsm_dev.tx_buffer_full = 1;
        QActive_arm((QActive *)&gsm_dev, 1000);
        return;
    }
    cli();
    if(gsm_dev.p_comm->tx.payload_size == 0)
    {
        gsm_dev.p_comm->tx.payload_size = length;
        sei();
        if(in_flash)
        {
            strcpy_P((char *)gsm_dev.p_comm->tx.p_data, (char *)data);
        }
        else
        {
            strcpy((char *)gsm_dev.p_comm->tx.p_data, (char *)data);
        }
    }
    else
    {
        gsm_dev.p_comm->tx.payload_size += length;
        sei();
        if(in_flash)
        {
            strcat_P((char *)gsm_dev.p_comm->tx.p_data, (char *)data);
        }
        else
        {
            strcat((char *)gsm_dev.p_comm->tx.p_data, (char *)data);
        }
    }

    QActive_post((QActive *)gsm_dev.p_comm, EVENT_COM_SEND_REQUEST, 0);
}

static void stop_command_timout(void)
{
    QActive_disarm((QActive *)&gsm_dev);
}



static void send_at_command(void)
{
    send_command((uint8_t *)"AT\r\n", false);
}

static void network_status_read_command()
{
    send_command((uint8_t *)CREG, 1);
}

static void sms_send_command()
{
    uint8_t length;
    length = strlen((char*)gsm_dev.control.master_buffer);
    send_command_timeout((uint8_t*)CMGS, 0, 1);
    send_command((uint8_t *)gsm_dev.control.master_buffer, 0);
    gsm_dev.control.master_buffer = gsm_dev.control.master_buffer + (length + 1);
}

static void sms_delete_command(uint8_t pos)
{
    send_command((uint8_t *)CMGD,1);
    send_command(&pos, 0);
    send_command_timeout((uint8_t*)"\r\n", 1000, 0);
}

static void get_time_command(void)
{
    send_command((uint8_t*)CCLK_REQ,1);
}

static void set_time_command()
{
    send_command_timeout((uint8_t*)CCLK, 0, 1);
    send_command(gsm_dev.control.master_buffer, 0);
}

static void check_sms_presence_command(uint8_t status)
{
    switch(status)
    {
        case SMS_UNREAD:
            send_command((uint8_t*)CMGL_UNREAD, 1);
            break;
        case SMS_ALL:
            send_command((uint8_t*)CMGL_ALL, 1);
            break;
        case SMS_READ:
            send_command((uint8_t*)CMGL_READ, 1);
            break;
    }
}

static void read_sms_command(uint8_t position)
{
    send_command((uint8_t *)CMGR, 1);
    send_command(&position, 0);
    send_command_timeout((uint8_t*)"\r\n", 1000, 0);
}
/************************************************************************************************************************************
                                                    ***** State Machines *****
************************************************************************************************************************************/

/* Init Super State */


/** INACTIVE STATE **/



static QState inactive_init(Gsm *const me)
{
    return Q_TRAN(&inactive_super);
}

static QState inactive_super(Gsm *const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            status =  Q_TRAN(&inactive_systemsetup);
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

        default:
        {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}

static QState inactive_systemsetup(Gsm *const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            /* Configure the ports */
            GSM_PWR_DDR |= (1 << GSM_PWRKEY);
            status = Q_TRAN(&inactive_opencom);
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

        default:
        {
            status = Q_SUPER(&inactive_super);
            break;
        }
    }
    return status;
}

static QState inactive_opencom(Gsm *const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            QActive_post(gsm_dev.p_comm, EVENT_COM_OPEN_REQUEST, (uint16_t)GSM_BAUD);
            status = Q_HANDLED();
            break;
        }

        case EVENT_COM_OPENED:
        {
            status = Q_TRAN(inactive_powering_on);
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

        default:
        {
            status = Q_SUPER(&inactive_super);
            break;
        }
    }
    return status;
}

static QState inactive_powering_on(Gsm *const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            status = Q_HANDLED();
            break;
        }

        case EVENT_SYSTEM_GSM_INIT:
        {
            GSM_PWR_PORT &= ~GSM_PWRKEY;
            QActive_arm((QActive *)me, 10);
            status = Q_HANDLED();
            break;
        }

        case Q_TIMEOUT_SIG:
        {
            GSM_PWR_PORT |= GSM_PWRKEY;
            QActive_disarm((QActive *)me);
            status = Q_TRAN(&inactive_check_comlink);
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

        default:
        {
            status = Q_SUPER(&inactive_super);
            break;
        }
    }
    return status;
}

static QState inactive_check_comlink(Gsm *const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            send_at_command();
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_TIMEOUT_SIG:
        {
            if(gsm_dev.tx_buffer_full)
            {
                send_at_command();
                status = Q_HANDLED();
            }
            else
            {
                /* no response toggle power again*/
                status = Q_TRAN(&inactive_powering_on);
            }
            break;
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            status = Q_TRAN(&inactive_device_config);
            break;
        }
        case Q_EXIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&inactive_super);
            break;
        }
    }
    return status;
}

static QState inactive_device_config(Gsm *const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            gsm_dev.control.ix = 0;
            send_command(module_init_table[gsm_dev.control.ix], 1);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            gsm_dev.control.ix++;
            if(module_init_table[gsm_dev.control.ix] == "")
            {
                /* Report the application that the init is done */
                QActive_post((QActive *)me->master, EVENT_GSM_INIT_DONE, 0);
                status = Q_TRAN(&active_super);
            }
            else
            {
                send_command(module_init_table[gsm_dev.control.ix], 1);
                status = Q_HANDLED();
            }
            break;
        }
        case EVENT_GSM_ERROR_RESPONSE:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_INIT_FAILURE, 0);
            status = Q_TRAN(&inactive_powering_on);
        }
        case Q_TIMEOUT_SIG:
        {
            if(gsm_dev.tx_buffer_full)
            {
                send_command(module_init_table[gsm_dev.control.ix], 1);
                status = Q_HANDLED();
            }
            else
            {
                QActive_post((QActive *)me->master, EVENT_GSM_INIT_FAILURE, 0);
                status = Q_TRAN(&inactive_powering_on);
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            stop_command_timout();
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&inactive_super);
            break;
        }
    }
    return status;
}

static QState init_inactive(Gsm *const me)
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

        case EVENT_SYSTEM_GSM_INIT:
        {
            status = Q_TRAN(&init_powering_on);
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

static QState init_powering_on(Gsm *const me)
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

        default:
        {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}

static QState init_device_config(Gsm *const me)
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

        default:
        {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}


/*  ACTIVE STATE */
static QState active_super(Gsm *const me)
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
            status = Q_TRAN(&active_idle);
            break;
        }
        case Q_EXIT_SIG:
        {
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

static QState active_idle(Gsm *const me)
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
        /* Handle requests from the System */
        case EVENT_GSM_NETWORK_READ_REQUEST:
        {
            if(me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                status = Q_HANDLED();
            }
            else
            {
                status = Q_TRAN(&active_network_status);
            }
            break;
        }
        case EVENT_GSM_SMS_DELETE:
        {
            if(me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                status = Q_HANDLED();
            }
            else
            {
                me->control.response = (uint8_t)Q_PAR(me);
                status = Q_TRAN(&active_sms_delete);
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&active_super);
            break;
        }
    }
    return status;
}

static QState active_network_status(Gsm *const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            network_status_read_command();
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_TIMEOUT_SIG:
        {
            if(me->control.timeout)
            {
                network_status_read_command();
                status = Q_HANDLED();
            }
            else
            {
                QActive_post((QActive *)me->master, EVNET_GSM_MODULE_FAILURE, 0);
                status = Q_TRAN(&active_idle);
            }
            break;
        }
        case EVENT_GSM_NETWORK_RESPONSE:
        {
            gsm_dev.control.response = (uint8_t)(Q_PAR(me) >> 16);
            status = Q_HANDLED();
            break;
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            status = Q_HANDLED();
            if(me->control.response)
            {
                switch(me->control.ix)
                {
                    case GSM_MSG_CREG_NETWORK_READY_LOCAL:
                    case GSM_MSG_CREG_NETWORK_READY_ROAMING:
                        QActive_post((QActive *)me->master, EVENT_GSM_NETWORK_CONNECTED, 0);
                        status = Q_TRAN(&active_idle);
                        break;
                    case GSM_MSG_CREG_NEWTWORK_SEARCHING_IN_IDLE:
                    case GSM_MSG_CREG_NETWORK_ACCESS_DENIED:
                        QActive_post((QActive *)me->master, EVENT_GSM_NETWORK_ERROR, 0);
                        status = Q_TRAN(&active_idle);
                        break;
                    case GSM_MSG_CREG_SEARCHING_NETWORK:
                        me->control.timeout = true;
                        QActive_arm(me, 5);
                        status = Q_HANDLED();
                        break;
                    default:
                        QActive_post((QActive *)me->master, EVENT_GSM_NETWORK_ERROR, 0);
                        status = Q_TRAN(&active_idle);
                        break;
                }
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            me->control.busy = false;
            me->control.timeout = false;
            QActive_disarm(me);
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&active_idle);
            break;
        }
    }
    return status;
}

static QState active_sms_send(Gsm *const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            sms_send_command();
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVNET_GSM_MODULE_FAILURE, 0);
            status = Q_TRAN(&active_idle);
            break;
        }
        case EVENT_GSM_SMS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if(me->control.response == GSM_MSG_SMS_PROMPT)
            {
                send_command_timeout(me->control.master_buffer, 1000, 0);
            }
            status = Q_HANDLED();
            break;
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            if(me->control.response)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_SMS_SENT, 0);
            }
            else
            {
                QActive_post((QActive *)me->master, EVNET_GSM_MODULE_FAILURE, 0);
            }
            status = Q_TRAN(&active_idle);
            break;
        }
        case Q_EXIT_SIG:
        {
            me->control.busy = false;
            me->control.timeout = false;
            QActive_disarm(me);
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&active_idle);
            break;
        }
    }
    return status;
}

static QState active_sms_delete(Gsm *const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            if(me->control.response == '0')
            {
                sms_delete_all_command();
            }
            else
            {
                sms_delete_command(me->control.response);
            }
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVNET_GSM_MODULE_FAILURE, 0);
            status = Q_TRAN(&active_idle);
            break;
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_SMS_DELETE_DONE, 0);
            status = Q_TRAN(&active_idle);
            break;
        }
        case Q_EXIT_SIG:
        {
            me->control.busy = false;
            me->control.timeout = false;
            QActive_disarm(me);
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&active_idle);
            break;
        }
    }
    return status;
}

static QState active_time_get(Gsm *const me)
{
    QState status;
    unsigned char *msg;
    unsigned char *p_char;
    unsigned char *p_char1;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            get_time_command();
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            status = Q_TRAN(&active_idle);
            break;
        }
        case EVENT_GSM_CLOCK_RESPONSE:
        {
            me->control.response = (uint8_t)Q_PAR(me) >> 16;
            msg = (uint16_t)Q_PAR(me);
            // +CCLK: "YY/MM/DD,HH:MM:SS+/-ZZ"
            //<CR><LF>OK<CR><LF>
            //
            //  Example Below
            //+CCLK: "12/10/19,18:39:51+08"
            //
            //OK
            // find out what was received exactly
            p_char = memchr(msg, '"', 23);
            msg = p_char + 1;
            p_char = memchar(msg,'"',23);
            if(p_char != NULL)
            {
                *(p_char - 3) = 0;
                strcpy(me->control.master_buffer,msg);
            }
            else
            {
                Q_TRAN(&active_idle);
            }
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            if(me->control.response)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_CLOCK_READ_DONE, 0);
            }
            else
            {
                QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            }
            status = Q_TRAN(&active_idle);
            break;
        }
        case Q_EXIT_SIG:
        {
            me->control.busy = false;
            me->control.timeout = false;
            QActive_disarm(me);
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&active_idle);
            break;
        }
    }
    return status;
}

static QState active_sms_presence(Gsm *const me)
{
    QState status;
    unsigned char *p_char;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            check_sms_presence_command(me->control.response);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            status = Q_TRAN(&active_idle);
            break;
        }
        case EVENT_GSM_SMS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if(me->control.response == GSM_MSG_SMS_LIST)
            {
                p_char = (uint16_t)Q_PAR(me->control.master_buffer);
                QActive_post((QActive *)me->master, EVENT_GSM_SMS_FOUND, p_char); // TODO: convert to ascii
            }
            status = Q_HANDLED();
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            if(!me->control.response)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_SMS_NOT_FOUND, 0);
                status = Q_TRAN(&active_idle);
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            me->control.busy = false;
            me->control.timeout = false;
            QActive_disarm(me);
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&active_idle);
            break;
        }
    }
    return status;
}

static QState active_sms_read(Gsm *const me)
{
    uint8_t *p_char;
    uint8_t *p_char1;
    uint8_t len;
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            read_sms_command(me->control.response);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case EVENT_GSM_SMS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if((me->control.response == GSM_MSG_SMS_REC_READ) || (me->control.response == GSM_MSG_SMS_REC_UNREAD))
            {
                /* response for new SMS:
                <CR><LF>+CMGR: "REC UNREAD","+XXXXXXXXXXXX",,"02/03/18,09:54:28+40"<CR><LF>
                There is SMS text<CR><LF>OK<CR><LF>
                <CR><LF>+CMGR: "REC READ","+XXXXXXXXXXXX",,"02/03/18,09:54:28+40"<CR><LF>
                There is SMS text<CR><LF>*/

                p_char = (uint16_t)Q_PAR(me);
                p_char = memchr(p_char, ',', 5);
                p_char1 = p_char + 2;
                p_char = memchr(p_char1, '"', 20);
                if(p_char != NULL)
                {
                    *p_char = 0;
                    strcpy(me->control.master_buffer, p_char1);
                    len = strlen(p_char1);
                }
                p_char = memchr(p_char1+1, 0x0A, 25);
                if(p_char != NULL)
                {
                    p_char++;
                }
                p_char1 = memchar((char *)(p_char), 0x0d, 130);
                if (p_char1 != NULL)
                {
                    // finish the SMS text string
                    // because string must be finished for right behaviour
                    // of next strcpy() function
                    *p_char1 = 0;
                }
                p_char1 = me->control.master_buffer + len + 1;
                strcpy(p_char1, (char *)(p_char));
            }
            status = Q_HANDLED();
            break;
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            status = Q_TRAN(&active_idle);
            break;
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            if(me->control.response)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_SMS_READ_DONE, 0);
            }
            else
            {
                QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            }
            status = Q_TRAN(&active_idle);
            break;
        }
        case Q_EXIT_SIG:
        {
            me->control.busy = false;
            me->control.timeout = false;
            QActive_disarm(me);
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&active_idle);
            break;
        }
    }
    return status;
}

static QState active_time_set(Gsm *const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            set_time_command();
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            status = Q_TRAN(&active_idle);
            break;
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_CLOCK_SET_DONE, 0);

            status = Q_TRAN(&active_idle);
            break;
        }
        case Q_EXIT_SIG:
        {
            me->control.busy = false;
            me->control.timeout = false;
            QActive_disarm(me);
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&active_idle);
            break;
        }
    }
    return status;
}




































#if 0

void init_statemachine()
{
    switch (GSM_buffer.cmd_byte)
    {
        case SEND_SMS:
            GSM_buffer.state = SMS_CMD;
            break;
        default:
            break;
    }
}
void parse_command(void *data)
{
    uint8_t i;
    /* GSM Buffer is filled based on the cmd byte */
    switch (GSM_buffer.cmd_byte)
    {
        case SEND_SMS:
        {
            /* "Phone No""Data to be sent" */
            i = strlen((char *) data);
            strcpy((char *) GSM_buffer.sms.phone_no, (char *) data);
            strcpy((char *) GSM_buffer.buffer, &data[i]);
            break;
        }

        case SET_TIME:
        {
            /* Time is set as string in appropriate form required by GSM Module */
            strcpy((char *) GSM_buffer.buffer, (char *) data);
            break;
        }

        case GET_TIME:
        {
            /* Time is set as string in appropriate form required by GSM Module */
            strcpy((char *) data, (char *) GSM_buffer.buffer);
            break;
        }

        default:
        {
            break;
        }
    } // End of switch
}

static QState process_command()
{
    QState status;
    switch (GSM_buffer.cmd_byte)
    {
        case SEND_SMS:
        {
            /* Transit to Trasmit state */
            status = &GSM_transmit;
            break;
        }

        case SET_TIME:
        {
            /* Transit to Trasmit state */
            status = &GSM_transmit;
            break;
        }

        case GET_TIME:
        {
            /* Transit to Trasmit state */
            status = &GSM_transmit;
            break;
        }

        default:
        {
            break;
        }
    } // End of switch
    return status;
}

static void trasmit_command()
{
    uint8_t tmp;
    switch (GSM_buffer.state)
    {
        case SMS_CMD:
        {
            set_timeouts(START_LONG_COMM_TMOUT, MAX_INTERCHAR_TMOUT);
            Serial_SendStringNonBlocking(&AO_GSM, "AT+CMGS=\"");
            Serial_SendStringNonBlocking(&AO_GSM, GSM_buffer.sms.phone_no);
            Serial_SendStringNonBlocking(&AO_GSM, "\"\r");
            break;
        }

        case SMS_DATA:
        {
            set_timeouts(START_XXLONG_COMM_TMOUT, MAX_INTERCHAR_TMOUT);
            tmp = 0x1A;
            Serial_SendStringNonBlocking(&AO_GSM, (char *) GSM_buffer.buffer);
            Serial_SendBufferNonBlocking(&AO_GSM, (char *) &tmp, 1);
        }
#if 0
        case SET_TIME:
        {
            break;
        }

        case GET_TIME:
        {
            break;
        }
#endif
        default:
        {
            break;
        }
    } // End of switch
}

static void process_data(void *next_state)
{
    /* Create a temp buffer to handle the data recieved */
    unsigned char tmp[100];
    uint8_t status;
    status = 2;
    /* Copy the data from Serial buffer and process it */
    switch (GSM_buffer.state)
    {
        case SMS_CMD:
        {
            if (compare_string((char *)tmp, ">"))
            {
                GSM_buffer.state = SMS_DATA;
                next_state = &GSM_transmit;
            }
            else
            {
                status = 0;
            }
            break;
        }

        case SMS_DATA:
        {
            if (compare_string((char *)tmp, "+CMGS"))
            {
                /* Mssg sent so intimate the application, raise event */
                status = 1;
            }
            else
            {
                status = 0;
            }
        }
#if 0
        case SET_TIME:
        {
            break;
        }

        case GET_TIME:
        {
            break;
        }
#endif
        default:
        {
            break;
        }
    } // End of switch

    if (status == 1)
    {
        QActive_post(GSM_buffer.active_object, GSM_SUCCESS_EVENT, 0);
        next_state = &GSM_input;
    }
    else if (!status)
    {
        QActive_post(GSM_buffer.active_object, GSM_FAILURE_EVENT, 0);
        next_state = &GSM_input;
    }
}

uint8_t compare_string(char const *buffer, char const *string)
{
    char *ch;
    uint8_t ret_val = 0;
    ch = strstr(buffer, string);
    if (ch != NULL)
    {
        ret_val = 1;
    }
    return (ret_val);
}

static void set_timeouts(GSM_timeouts start_comm_tmout, GSM_timeouts max_interchar_tmout)
{
    GSM_buffer.start_comm_tmout = start_comm_tmout;
    GSM_buffer.max_interchar_tmout = max_interchar_tmout;
    return;
}

void GSM_ctor(void)
{
    QActive_ctor(&AO_GSM.super, Q_STATE_CAST(&GSM_initial));
}


static QState GSM_initial(GSM *const me)
{
    //PORTD ^= (1 << 2);
    return Q_TRAN(&GSM_input);
}


static QState GSM_input(GSM *const me)
{
    QState status;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            status = Q_HANDLED();
            break;
        }

        case Q_INIT_SIG:
        {
            break;
        }

        case GSM_PROCESS_SIG:
        {
            //PORTD ^= (1 << 2);
            /* Process the cmd byte, and perform necessary action */
            init_statemachine();
            status = Q_TRAN(process_command());
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


static QState GSM_transmit(GSM *const me)
{
    QState status;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            /* Send the data to GSM Module */
            trasmit_command();
            status = Q_HANDLED();
            break;
        }

        case Q_INIT_SIG:
        {
            break;
        }

        case SERIAL_TRANSMIT_SIG:
        {
            /* Recieved on completion of Serial transmit, goto Serial Recieve state */
            status = Q_TRAN(&GSM_recieve);
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


static QState GSM_recieve(GSM *const me)
{
    QState status;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            /* Start the maximum start of reception timeout */
            QActive_arm((QActive *)me, GSM_buffer.start_comm_tmout);
            status = Q_HANDLED();
            break;
        }

        case Q_INIT_SIG:
        {
            break;
        }

        case SERIAL_RECIEVE_SIG:
        {
            /* Recieved on each Serial Recieve */
            //PORTD ^= (1 << 3);
            QActive_arm((QActive *)me, GSM_buffer.max_interchar_tmout);
            status = Q_HANDLED();
            break;
        }

        case Q_TIMEOUT_SIG:
        {
            /* Start of communication / Intercharacter timeout has occured */
            status = Q_TRAN(&GSM_process);
            //PORTD ^= (1 << 4);
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


static QState GSM_process(GSM *const me)
{
    QState status;
    void *next_state;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            Serial_SendStringNonBlocking((QActive *)&AO_GSM, "Done baby \r\n");
            process_data(next_state);
            status = Q_TRAN(next_state);
            break;
        }

        case Q_INIT_SIG:
        {
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

/*
    switch(GSM_buffer.cmd_byte)
    {
        case SEND_SMS:
        {
            break;
        }

        case SET_TIME:
        {
            break;
        }

        case GET_TIME:
        {
            break;
        }

        default:
        {
            break;
        }
    } // End of switch
*/
#endif