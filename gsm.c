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
#include "gsm_settings.h"
#include "softserial.h"

#define RX_NOT_STARTED      0
#define RX_ALREADY_STARTED  1

#define DEBUG_1        0
#define DEBUG_2        0
//uint8_t com_buff[COM_BUFFER_LENGTH];


typedef struct Gsm_tag
{
    QActive super;
    /* Public Members */
    Com *p_comm;
    App *master;
    volatile uint8_t tx_buffer_full;
    struct
    {
        volatile uint8_t current_op;
        uint8_t response;
        uint8_t ix;
        uint8_t busy;
        uint8_t timeout;
        uint8_t *master_buffer;
        uint8_t *phone_no;
        uint8_t *mssg_buf;
        uint8_t use_flash;
    } control;
} Gsm;

Gsm gsm_dev;

static QState GSM_initial(Gsm *const me);
static QState GSM_input(Gsm *const me);
static QState GSM_transmit(Gsm *const me);
static QState GSM_recieve(Gsm *const me);
static QState GSM_process(Gsm *const me);

static void comm_rx_handler(uint8_t size);
static uint8_t check_result_codes(uint8_t *p_in, uint8_t *message_id);
static void parse_input_command(uint8_t result_index);
static void generic_exit_operations(void);
//static uint8_t parse_command(uint8_t* p_data, struct at_response_code* table, uint8_t* message_id);
static void send_command_timeout(uint8_t *data, uint16_t timeout, uint8_t in_flash);
static void send_command(uint8_t *data, uint8_t in_flash);
static void stop_command_timout(void);
static void sms_send_command();
static void sms_delete_command(uint8_t *pos);
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
static QState inactive_init(Gsm *const me);
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
static QState active_sms_extract_time(Gsm *const me);
static QState active_sms_extract_mssg(Gsm *const me);
static QState active_sms_send_support(Gsm *const me);

static QState init_inactive(Gsm *const me);
static QState init_powering_on(Gsm *const me);
static QState init_device_config(Gsm *const me);
static QState active_super(Gsm *const me);

static QState active_initial(Gsm *const me);
static QState on_active(Gsm *const me);
static QState on_active_cmd(Gsm *const me);


static QState active_gprs_setup(Gsm *const me);
static QState active_gprs_start(Gsm *const me);
static QState active_gprs_open_socket(Gsm *const me);
static QState active_gprs_send_data(Gsm *const me);
static QState active_gprs_close_socket(Gsm *const me);

/************************************************************************************************************************************
                                                    ***** Public Funtions *****
************************************************************************************************************************************/

void GSM_ctor(void)
{
    QActive_ctor(&gsm_dev.super, Q_STATE_CAST(&inactive_init));
}

void GSM_config(App *master, Com *com_drv)
{
    gsm_dev.master = master;
    gsm_dev.p_comm = com_drv;
    gsm_dev.control.current_op = 0;
    gsm_dev.control.use_flash = 0;
    gsm_dev.control.master_buffer = master->buffer;
    gsm_dev.control.phone_no = master->current_phone_no;
    gsm_dev.control.mssg_buf = master->mssg_buf;
}

/************************************************************************************************************************************
                                                    ***** Private Funtions *****
************************************************************************************************************************************/

static void comm_rx_handler(uint8_t size)
{
    uint8_t *p_data = gsm_dev.p_comm->rx.p_data;
    uint8_t *p_str;
    uint8_t *p_end;
    uint8_t i, str_size, parsed_command, message_id;
    uint16_t temp;
    i = 0;
#if 0
    Softserial_println("------- INFO ------");
    Softserial_print("size=");
    Softserial_print_byte(size);
    Softserial_println("");
    Softserial_print_array((char *)p_data, size);
    Softserial_println("---------");
#endif
    /* Search for \r\n<Result codes>\r\n */
    while (i < size)
    {
#if 0
        Softserial_print("i=");
        Softserial_print_byte(i);
        Softserial_println("");
#endif
        if (p_data[i++] == '\r')
        {
            if (p_data[i++] == '\n')
            {
                p_str = &p_data[i];
                p_end = (uint8_t *)memchr(p_str, '\r', (size - i));
                if (p_end == 0)
                {
                    /* TODO: Try if u can optimize this section */
                    if ((gsm_dev.control.current_op == GSM_OP_SMS) || (gsm_dev.control.current_op == GSM_OP_GPRS))
                    {
                        str_size = 2;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    str_size = p_end - p_str;
                    /* Setup parameters for next round */
                }
                i += 1 + str_size;
                if (p_data[i] == '\n')
                {
                    i++;
                }
                if (!str_size)
                {
                    i -= 2;
                }
                Softserial_print_byte(str_size);
                Softserial_print("  ");
                Softserial_print_array((char *)p_str, str_size);
                Softserial_println("");
                parsed_command = check_result_codes(p_str, &message_id);
#if 1
                Softserial_print("evt id= ");
                Softserial_print_byte(parsed_command);
                Softserial_println("");
                Softserial_print("msg id= ");
                Softserial_print_byte(message_id);
                Softserial_println("");
#endif
                switch (parsed_command)
                {
                    case EVENT_GSM_SMS_RESPONSE:
                    {
                        /* Pass the message id*/
                        if (message_id == GSM_MSG_SMS_LIST)
                        {
                            p_str = memchr(p_str, ':', 7);
                            if (p_str != NULL)
                            {
                                temp = atoi((char *)p_str + 1);
                            }
                        }
                        else if ((message_id == GSM_MSG_SMS_REC_READ) || (message_id == GSM_MSG_SMS_REC_UNREAD))
                        {
                            temp = (uint16_t)p_str;
                        }
                        QActive_post((QActive *)&gsm_dev, parsed_command, ((((uint32_t)message_id) << 16) + (uint16_t)temp));
                        break;
                    }
                    case EVENT_GSM_CLOCK_RESPONSE:
                    {
                        QActive_post((QActive *)&gsm_dev, parsed_command, ((((uint32_t)message_id) << 16) + (uint16_t)p_str));
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
                        //QParam temp_id;
                        //temp_id = message_id << 16;
                        //QActive_post((QActive *)&gsm_dev, parsed_command, message_id);
                        QActive_post((QActive *)&gsm_dev, parsed_command, ((uint32_t)message_id) << 16);
                        if (parsed_command == EVENT_GSM_ACK_RESPONSE)
                        {
#ifdef DEBUG_2
                            Softserial_println("ack response");
#endif
                            stop_command_timout();
                        }
                        break;
                    }
                } // End of switch
            }
        }
    } // while i<size
    Com_reopen();
    /* TODO: make provisions regarding open/close clear of rx buffer */
}

static uint8_t parse_command(uint8_t *p_data, struct at_response_code *table, uint8_t *message_id)
{
    uint8_t i = 0;
    uint8_t j;
    do
    {
        j = strlen((char *)(table[i].p_string));
        if ((memcmp(table[i].p_string, p_data, j)) == 0)
        {
            *message_id = table[i].message_id;
            return table[i].event_id;
        }
        i++;
    }
    while (table[i].p_string != 0);
    return 0;
}

static uint8_t check_result_codes(uint8_t *p_in, uint8_t *message_id)
{
    // uint8_t i = 0;
    // uint8_t j;
    // uint8_t error_code[4];
    uint8_t event_id;
    const struct  at_response_code *p_table;
    switch (gsm_dev.control.current_op)
    {
        case GSM_OP_SMS:
        {
            p_table = &sms_table[0];
            break;
        }
        case GSM_OP_NETWORK:
        {
            p_table = &network_table[0];
            break;
        }
        case GSM_OP_ACC:
        {
            p_table = &acc_table[0];
            break;
        }
        case GSM_OP_GPRS:
        {
            p_table = &gprs_table[0];
            break;
        }
        default:
        {
            p_table = &frc_table[0];
            break;
        }
    }
    event_id = parse_command(p_in, (struct at_response_code *)p_table, message_id);
    if (!event_id)
    {
        event_id = parse_command(p_in,  &frc_table[0], message_id);
    }
    return event_id;
}

static void send_command(uint8_t *data, uint8_t in_flash)
{
    send_command_timeout(data, 1 sec, in_flash);
}

static void send_command_timeout(uint8_t *data, uint16_t timeout, uint8_t in_flash)
{
    /* Fill up the buffer and send event */
    uint8_t length;
    if (in_flash)
    {
        length = strlen_P((char *)data);
    }
    else
    {
        length = strlen((char *)data);
    }
#if 0
    if ((gsm_dev.p_comm->tx.payload_size + length) > gsm_dev.p_comm->tx.size)
    {
        /* Raise tx buffer full flag and request timeout */
        gsm_dev.tx_buffer_full = 1;
        QActive_arm((QActive *)&gsm_dev, 1000);
        return;
    }
#endif
#if 0
    Softserial_print("len-");
    Softserial_print_byte(length);
    Softserial_println("");
#endif
    cli();
    length = length + gsm_dev.p_comm->tx.payload_size;
    sei();
    if (length > gsm_dev.p_comm->tx.size)
    {
        length = gsm_dev.p_comm->tx.size;
        if (gsm_dev.p_comm->tx.payload_size == 0)
        {
            if (in_flash)
            {
                // Softserial_println("flash len 1"); Softserial_println_flash(data);
                memcpy_P(gsm_dev.p_comm->tx.p_data, (char *)data, gsm_dev.p_comm->tx.size);
                QActive_post((QActive *)&gsm_dev, EVENT_GSM_BUFFER_FULL, gsm_dev.p_comm->tx.size + 1);
            }
        }
    }
    else
    {
        cli();
        if (gsm_dev.p_comm->tx.payload_size == 0)
        {
            sei();
            if (in_flash)
            {
                //Softserial_println("flash len 2"); Softserial_println_flash(data);
                strcpy_P((char *)gsm_dev.p_comm->tx.p_data, (char *)data);
            }
            else
            {
                strcpy((char *)gsm_dev.p_comm->tx.p_data, (char *)data);
            }
        }
        else
        {
            if (in_flash)
            {
                //Softserial_println("flash len 3"); Softserial_println_flash(data);
                strcat_P((char *)gsm_dev.p_comm->tx.p_data, (char *)data);
            }
            else
            {
                strcat((char *)gsm_dev.p_comm->tx.p_data, (char *)data);
            }
        }
    }
    cli();
    gsm_dev.p_comm->tx.payload_size = length;
    sei();
#if 0
    if (gsm_dev.p_comm->tx.payload_size == 0)
    {
        gsm_dev.p_comm->tx.payload_size = length;
        sei();
        if (in_flash)
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
        if (in_flash)
        {
            strcat_P((char *)gsm_dev.p_comm->tx.p_data, (char *)data);
        }
        else
        {
            strcat((char *)gsm_dev.p_comm->tx.p_data, (char *)data);
        }
    }
#endif
    QActive_post((QActive *)gsm_dev.p_comm, EVENT_COM_SEND_REQUEST, 0);
    QActive_arm((QActive *)&gsm_dev, timeout);
}

static void stop_command_timout(void)
{
    QActive_disarm((QActive *)&gsm_dev);
}

static void generic_exit_operations(void)
{
    gsm_dev.control.busy = false;
    gsm_dev.control.timeout = false;
    gsm_dev.control.current_op = GSM_OP_NONE;
    QActive_disarm((QActive *)&gsm_dev);
}

static void send_at_command(void)
{
    send_command((uint8_t *)"AT\r\n", 0);
}

static void network_status_read_command()
{
    send_command((uint8_t *)CREG, 1);
}

static void sms_send_command()
{
#if 0
    uint8_t length;
    length = strlen((char *)gsm_dev.control.master_buffer);
    send_command_timeout((uint8_t *)CMGS, 0, 1);
    send_command((uint8_t *)gsm_dev.control.master_buffer, 0);
    send_command((uint8_t *)"\"\r\n", 0);
    gsm_dev.control.master_buffer = gsm_dev.control.master_buffer + (length + 1);
#endif
    send_command_timeout((uint8_t *)CMGS, 0, 1);
    send_command((uint8_t *)gsm_dev.control.phone_no, 0);
    send_command((uint8_t *)"\"\r\n", 0);
}

static void sms_delete_command(uint8_t *pos)
{
    send_command((uint8_t *)CMGD, 1);
    send_command(pos, 0);
    send_command_timeout((uint8_t *)"\r\n", 1000, 0);
}

static void sms_delete_all_command()
{
    send_command((uint8_t *)CMGDA, 1);
}

static void get_time_command(void)
{
    send_command((uint8_t *)CCLK_REQ, 1);
}

static void set_time_command()
{
    send_command_timeout((uint8_t *)CCLK, 0, 1);
    send_command(gsm_dev.control.mssg_buf, 0);
    send_command_timeout((uint8_t *)"\r\n", 1000, 0);
}

static void check_sms_presence_command(uint8_t status)
{
    switch (status)
    {
        case SMS_UNREAD:
            send_command((uint8_t *)CMGL_UNREAD, 1);
            break;
        case SMS_ALL:
            send_command((uint8_t *)CMGL_ALL, 1);
            break;
        case SMS_READ:
            send_command((uint8_t *)CMGL_READ, 1);
            break;
    }
}

static void read_sms_command(uint8_t *position)
{
    send_command((uint8_t *)CMGR, 1);
    send_command(position, 0);
    send_command_timeout((uint8_t *)"\r\n", 1000, 0);
}

static void print_eventid(uint8_t id)
{
    Softserial_print("-");
    Softserial_print_byte((uint8_t)id);
    Softserial_println("");
}

static void attach_gprs_command()
{
    send_command_timeout((uint8_t *)CGATT, 10 sec, 1);
}

static void set_gprs_parameters()
{
    send_command((uint8_t *)CSTT, 1);
    send_command((uint8_t *)gsm_dev.control.mssg_buf, 0);
    send_command_timeout((uint8_t *)"\r\n", 7 sec, 0);
}
static void start_gprs_command()
{
    send_command_timeout((uint8_t *)CIICR, 20 sec, 1);
}
static void open_tcp_socket_command()
{
    send_command((uint8_t *)CIPSTART, 1);
    send_command((uint8_t *)gsm_dev.control.mssg_buf, 0);
    send_command_timeout((uint8_t *)"\r\n", 7 sec, 0);
}

static void send_gprs_data()
{
    send_command_timeout((uint8_t *)CIPSEND, 7 sec, 1);
}

static void close_tcp_socket_command()
{
    send_command_timeout((uint8_t *)CIPSHUT, 3 sec, 1);
}

static void send_gprs_status_command()
{
    send_command_timeout((uint8_t *)CIPSTATUS, 5 sec, 1);
}

static void get_gprs_ip_command()
{
    send_command_timeout((uint8_t *)CIFSR, 5 sec , 1);
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
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return  Q_TRAN(&inactive_systemsetup);
        }
        case EVENT_COM_DATA_AVAILABLE:
        {
#ifdef DEBUG_2
            Softserial_println("data recieved");
#endif
#if 0
            uint8_t size = (uint8_t)Q_PAR(me);
            Softserial_print_byte(size);
            Softserial_println("");
            Softserial_print_array(gsm_dev.p_comm->rx.p_data, size);
            Softserial_println("");
#endif
            comm_rx_handler((uint8_t)Q_PAR(me));
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        default:
        {
            return Q_SUPER(&QHsm_top);
        }
    }
}

static QState inactive_systemsetup(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            /* Configure the ports */
            //GSM_PWR_DDR |= (1 << GSM_PWRKEY);
            // GSM_PWR_PORT &= ~GSM_PWRKEY;
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_SYSTEM_START_AO:
        {
            return Q_TRAN(&inactive_opencom);
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        default:
        {
            return Q_SUPER(&inactive_super);
        }
    }
}

static QState inactive_opencom(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            QActive_post((QActive *)gsm_dev.p_comm, EVENT_COM_OPEN_REQUEST, (uint16_t)GSM_BAUD);
            return Q_HANDLED();
        }
        case EVENT_COM_OPEN_DONE:
        {
            return Q_TRAN(inactive_powering_on);
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        default:
        {
            return Q_SUPER(&inactive_super);
        }
    }
}

static QState inactive_powering_on(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_SYSTEM_GSM_INIT:
        {
            //GSM_PWR_PORT |= GSM_PWRKEY;
            QActive_arm((QActive *)me, 3 sec);
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            //GSM_PWR_PORT &= ~GSM_PWRKEY;
            QActive_disarm((QActive *)me);
            return Q_TRAN(&inactive_check_comlink);
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        default:
        {
            return Q_SUPER(&inactive_super);
        }
    }
}

static QState inactive_check_comlink(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.timeout = 4;
            send_at_command();
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            /* no response toggle power again*/
            if (me->control.timeout--)
            {
                send_at_command();
                return Q_HANDLED();
            }
            else
            {
                QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
                return Q_TRAN(&inactive_powering_on);
            }
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            return Q_TRAN(&inactive_device_config);
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        default:
        {
            return Q_SUPER(&inactive_super);
        }
    }
}

static QState inactive_device_config(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            gsm_dev.control.ix = 0;
            send_command((uint8_t *)module_init_table[gsm_dev.control.ix], 0);
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            gsm_dev.control.ix++;
            if (module_init_table[gsm_dev.control.ix] == "")
            {
                /* Report the application that the init is done */
                QActive_post((QActive *)me->master, EVENT_GSM_INIT_DONE, 0);
                return Q_TRAN(&active_super);
            }
            else
            {
                send_command((uint8_t *)module_init_table[gsm_dev.control.ix], 0);
                return Q_HANDLED();
            }
        }
        case EVENT_GSM_ERROR_RESPONSE:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_INIT_FAILURE, 0);
            return Q_TRAN(&inactive_powering_on);
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_INIT_FAILURE, 0);
            return Q_TRAN(&inactive_powering_on);
        }
        case Q_EXIT_SIG:
        {
            stop_command_timout();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&inactive_super);
}


/*  ACTIVE STATE */
static QState active_super(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_TRAN(&active_idle);
        }
        case EVENT_COM_DATA_AVAILABLE:
        {
#ifdef DEBUG_2
            Softserial_println("data recieved");
#endif
            comm_rx_handler((uint8_t)Q_PAR(me));
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}

static QState active_idle(Gsm *const me)
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
        /* Handle requests from the System */
        case EVENT_GSM_NETWORK_READ_REQUEST:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                me->control.timeout = 1;
                return Q_TRAN(&active_network_status);
            }
        }
        case EVENT_GSM_SMS_READ_EXTRACT_MSSG:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                me->control.response = (uint8_t)Q_PAR(me);
                return Q_TRAN(&active_sms_extract_mssg);
            }
        }
        case EVENT_GSM_SMS_READ_EXTRACT_TIME:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                me->control.response = 1;
                return Q_TRAN(&active_sms_extract_time);
            }
        }
        case EVENT_GSM_SMS_READ_REQUEST:
        {
#ifdef DEBUG_2
            Softserial_println("read request");
#endif
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                me->control.response = (uint8_t)Q_PAR(me);
                return Q_TRAN(&active_sms_read);
            }
        }
        case EVENT_GSM_SMS_DELETE:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                me->control.response = (uint8_t)Q_PAR(me);
                return Q_TRAN(&active_sms_delete);
            }
        }
        case EVENT_GSM_CLOCK_READ:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                return Q_TRAN(&active_time_get);
            }
        }
        case EVENT_GSM_CLOCK_SET:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                return Q_TRAN(&active_time_set);
            }
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_SEND:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                me->control.use_flash = (uint8_t)Q_PAR(me);
                return Q_TRAN(&active_sms_send);
            }
        }
        case EVENT_GSM_SMS_CHECK_PRESENCE:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                me->control.response = (uint8_t)Q_PAR(me);
                return Q_TRAN(&active_sms_presence);
            }
        }
        case EVENT_GSM_SMS_RECIEVE_URC:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_SMS_NEW_RECIEVED, 0);
            return Q_HANDLED();
        }
        case EVENT_GSM_GPRS_START:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                return Q_TRAN(&active_gprs_start);
            }
        }
        case EVENT_GSM_GPRS_SOCKET_OPEN:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                return Q_TRAN(&active_gprs_open_socket);
            }
        }
        case EVENT_GSM_GPRS_SEND_DATA:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                return Q_TRAN(&active_gprs_send_data);
            }
        }
        case EVENT_GSM_GPRS_SETUP:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                return Q_TRAN(&active_gprs_setup);
            }
        }
        case EVENT_GSM_GPRS_SOCKET_CLOSE:
        {
            if (me->control.busy)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_BUSY, 0);
                return Q_HANDLED();
            }
            else
            {
                return Q_TRAN(&active_gprs_close_socket);
            }
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_super);
}

static QState active_network_status(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_NETWORK;
            network_status_read_command();
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            if (me->control.timeout--)
            {
                network_status_read_command();
                return Q_HANDLED();
            }
            else
            {
                QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
                return Q_TRAN(&active_idle);
            }
        }
        case EVENT_GSM_NETWORK_RESPONSE:
        {
            //me->control.response = (uint8_t)Q_PAR(me);
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
#ifdef DEBUG_2
            Softserial_print("gsm event msg id= ");
            Softserial_print_byte(me->control.response);
            Softserial_println("");
#endif
#if 0
            uint8_t a = (uint8_t)Q_PAR(me);
            Softserial_print("gsm event msg id= ");
            Softserial_print_byte(a);
            Softserial_println("");
#endif
            return Q_HANDLED();
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
#ifdef DEBUG_2
            Softserial_println("ack event network");
#endif
            if (me->control.response)
            {
                switch (me->control.response)
                {
                    case GSM_MSG_CREG_NETWORK_READY_LOCAL:
                    case GSM_MSG_CREG_NETWORK_READY_ROAMING:
#ifdef DEBUG_1
                        Softserial_println("local-roaming");
#endif
                        QActive_post((QActive *)me->master, EVENT_GSM_NETWORK_CONNECTED, 0);
                        return Q_TRAN(&active_idle);
                        break;
                    case GSM_MSG_CREG_NEWTWORK_SEARCHING_IN_IDLE:
                    case GSM_MSG_CREG_NETWORK_ACCESS_DENIED:
#ifdef DEBUG_1
                        Softserial_println("idle-denied");
#endif
                        QActive_post((QActive *)me->master, EVENT_GSM_NETWORK_ERROR, 0);
                        return Q_TRAN(&active_idle);
                        break;
                    case GSM_MSG_CREG_SEARCHING_NETWORK:
#ifdef DEBUG_1
                        Softserial_println("searching");
#endif
                        me->control.timeout = true;
                        QActive_arm((QActive *)me, 5 sec);
                        return Q_HANDLED();
                        break;
                    default:
#ifdef DEBUG_1
                        Softserial_println("network error");
#endif
                        QActive_post((QActive *)me->master, EVENT_GSM_NETWORK_ERROR, 0);
                        return Q_TRAN(&active_idle);
                        break;
                }
            }
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_super);
}

static QState active_sms_send(Gsm *const me)
{
#if 0
    Softserial_print("s:sms send");
    print_eventid(Q_SIG(me));
#endif
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_SMS;
            sms_send_command();
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVNET_GSM_MODULE_FAILURE, 0);
            return Q_TRAN(&active_idle);
        }
        case EVENT_GSM_BUFFER_FULL:
        {
            me->control.use_flash = Q_PAR(me);
            return Q_TRAN(&active_sms_send_support);
        }
        case EVENT_GSM_SMS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if (me->control.response == GSM_MSG_SMS_PROMPT)
            {
                QActive_disarm((QActive *)me);
#if 0
                send_command_timeout(me->control.master_buffer, 0, 0);
#endif
                if (me->control.use_flash)
                {
                    Softserial_println_flash(me->control.mssg_buf[1] << 8 | me->control.mssg_buf[0]);
                    send_command_timeout((uint8_t *)(me->control.mssg_buf[1] << 8 | me->control.mssg_buf[0]), 0, 1);
                }
                else
                {
                    strcat((char *)me->control.mssg_buf, "\x1A");
                    send_command_timeout(me->control.mssg_buf, 0, 0);
                }
#if 0
                strcat((char *)me->control.mssg_buf, "\x1A");
                send_command_timeout(me->control.mssg_buf, 0, 0);
                *me->control.master_buffer = 0x1A;
                *(me->control.master_buffer + 1) = 0;
                send_command_timeout(me->control.master_buffer, 5 sec, 0);
#endif
            }
            return Q_HANDLED();
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            if (me->control.response)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_SMS_SENT, 0);
            }
            else
            {
                QActive_post((QActive *)me->master, EVNET_GSM_MODULE_FAILURE, 0);
            }
            return Q_TRAN(&active_idle);
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_super);
}

static QState active_sms_send_support(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case EVENT_COM_SEND_DONE:
        {
            if (me->control.use_flash)
            {
                QActive_disarm((QActive *)me);
                send_command_timeout((uint8_t *)(me->control.mssg_buf[1] << 8 | me->control.mssg_buf[0]) + me->control.use_flash, 0, 1);
                *me->control.master_buffer = 0x1A;
                *(me->control.master_buffer + 1) = 0;
                send_command_timeout(me->control.master_buffer, 5 sec, 0);
                me->control.use_flash = 0;
            }
            return Q_HANDLED();
        }

    }
    return Q_SUPER(&active_sms_send);
}

static QState active_sms_delete(Gsm *const me)
{
    uint8_t index[3];
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_SMS;
            if (me->control.response)
            {
                itoa(me->control.response, (char *)index, 10);
                sms_delete_command(index);
            }
            else
            {
                sms_delete_all_command();
            }
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVNET_GSM_MODULE_FAILURE, 0);
            return Q_TRAN(&active_idle);
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_SMS_DELETE_DONE, 0);
            return Q_TRAN(&active_idle);
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_super);
}

static QState active_time_get(Gsm *const me)
{
    uint8_t *msg;
    uint8_t *p_char;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_ACC;
            get_time_command();
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            return Q_TRAN(&active_idle);
        }
        case EVENT_GSM_CLOCK_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
#if 0
            Softserial_print("clock event id= ");
            Softserial_print_byte(me->control.response);
            Softserial_println("");
#endif
            //me->control.response = (uint8_t)Q_PAR(me);
            msg = (uint8_t *)Q_PAR(me);
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
            p_char = memchr(msg, '"', 23);
            if (p_char != NULL)
            {
                *(p_char - 3) = 0;
                strcpy((char *)me->control.master_buffer, (char *)msg);
                return Q_HANDLED();
            }
            /* TODO : issue present here */
            return Q_TRAN(&active_idle);
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            if (me->control.response)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_CLOCK_READ_DONE, 0);
            }
            else
            {
                QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            }
            return Q_TRAN(&active_idle);
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_super);
}

static QState active_sms_presence(Gsm *const me)
{
    unsigned char index;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_SMS;
            check_sms_presence_command(me->control.response);
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            return Q_TRAN(&active_idle);
        }
        case EVENT_GSM_SMS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if (me->control.response == GSM_MSG_SMS_LIST)
            {
                index = (uint8_t)Q_PAR(me);
                QActive_post((QActive *)me->master, EVENT_GSM_SMS_FOUND, index); // TODO: convert to ascii
            }
            return Q_HANDLED();
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            if (!me->control.response)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_SMS_NOT_FOUND, 0);
            }
            return Q_TRAN(&active_idle);
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_super);
}

static QState active_sms_read(Gsm *const me)
{
    uint8_t index[3];
    char *p_char;
    char *p_char1;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
#ifdef DEBUG_2
            Softserial_println("reading");
#endif
            me->control.busy = true;
            me->control.current_op = GSM_OP_SMS;
            if (me->control.response)
            {
                itoa(me->control.response, (char *)index, 10);
                read_sms_command(index);
                /* TODO: Send invalid command error */
            }
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
#if 1
        case EVENT_GSM_SMS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if ((me->control.response == GSM_MSG_SMS_REC_READ) || (me->control.response == GSM_MSG_SMS_REC_UNREAD))
            {
                /* response for new SMS:
                <CR><LF>+CMGR: "REC UNREAD","+XXXXXXXXXXXX",,"02/03/18,09:54:28+40"<CR><LF>
                There is SMS text<CR><LF>OK<CR><LF>
                <CR><LF>+CMGR: "REC READ","+XXXXXXXXXXXX",,"02/03/18,09:54:28+40"<CR><LF>
                There is SMS text<CR><LF>*/

                p_char = (char *)Q_PAR(me);
                p_char = memchr(p_char, ',', 23);
                p_char1 = p_char + 2;
                p_char = memchr(p_char1, '"', 15);
                if (p_char == NULL)
                {
                    return Q_HANDLED();
                }
                *p_char = 0;
                //strcpy((char *)me->control.master_buffer, (char *)p_char1);
                strcpy((char *)me->control.phone_no, (char *)p_char1);
                //len = strlen((char *)p_char1);
                p_char = memchr(p_char + 1, '\n', 29);
                if (p_char == NULL)
                {
                    return Q_HANDLED();
                }
                p_char++;
                p_char1 = memchr((char *)(p_char), 0x0d, 130);
                if (p_char1 != NULL)
                {
                    // finish the SMS text string
                    // because string must be finished for right behaviour
                    // of next strcpy() function
                    *p_char1 = 0;
                    //p_char1 = me->control.master_buffer + len + 1;
                    strcpy((char *)me->control.mssg_buf, (char *)(p_char));
                }
            }
            return Q_HANDLED();
        }
#endif
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            return Q_TRAN(&active_idle);
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
#ifdef DEBUG_2
            Softserial_println("reading ack");
#endif
            if (me->control.response)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_SMS_READ_DONE, 0);
            }
            else
            {
                QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            }
            return Q_TRAN(&active_idle);
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_super);
}

static QState active_sms_extract_time(Gsm *const me)
{
    uint8_t *p_char;
    uint8_t *p_char1;
    uint8_t index[3];
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
#ifdef DEBUG_2
            Softserial_println("reading");
            me->control.busy = true;
#endif
            me->control.current_op = GSM_OP_SMS;
            strcpy((char *)index, "1");
            read_sms_command(index);
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            //return Q_TRAN(&active_sms_read);
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if ((me->control.response == GSM_MSG_SMS_REC_READ) || (me->control.response == GSM_MSG_SMS_REC_UNREAD))
            {
                /* response for new SMS:
                <CR><LF>+CMGR: "REC UNREAD","+XXXXXXXXXXXX","","02/03/18,09:54:28+40"<CR><LF>
                There is SMS text<CR><LF>OK<CR><LF>
                <CR><LF>+CMGR: "REC READ","+XXXXXXXXXXXX","","02/03/18,09:54:28+40"<CR><LF>
                There is SMS text<CR><LF>*/

                p_char = (uint8_t *)Q_PAR(me);
                p_char = memchr(p_char, ',', 23);
                p_char++;
                p_char = memchr(p_char, ',', 23);
                if (*(p_char + 4) == '"')
                {
                    p_char += 5;
                    p_char1 = memchr(p_char, '"', 22);
                    if (p_char1 == NULL)
                    {
                        me->control.response = 0;
                        QActive_post((QActive *)me->master, EVENT_GSM_PARSING_ERROR, 0);
                        return Q_HANDLED();
                    }
                    *(p_char1 + 1) = 0; // end of string
                    p_char--;
                }
                else
                {
                    me->control.response = 0;
                }
                strcpy((char *)me->control.mssg_buf, (char *)p_char);
                Softserial_println((char *)me->control.mssg_buf);
            }
            return Q_HANDLED();
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
#ifdef DEBUG_2
            Softserial_println("reading ack");
#endif
            if (me->control.response)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_SMS_READ_DONE, 0);
            }
            else
            {
                QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            }
            return Q_TRAN(&active_idle);
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_idle);
}

static QState active_sms_extract_mssg(Gsm *const me)
{
    uint8_t *p_char;
    uint8_t *p_char1;
    uint8_t len;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_TRAN(&active_sms_read);
        }
        case EVENT_GSM_SMS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if ((me->control.response == GSM_MSG_SMS_REC_READ) || (me->control.response == GSM_MSG_SMS_REC_UNREAD))
            {
                /* response for new SMS:
                <CR><LF>+CMGR: "REC UNREAD","+XXXXXXXXXXXX",,"02/03/18,09:54:28+40"<CR><LF>
                There is SMS text<CR><LF>OK<CR><LF>
                <CR><LF>+CMGR: "REC READ","+XXXXXXXXXXXX",,"02/03/18,09:54:28+40"<CR><LF>
                There is SMS text<CR><LF>*/

                p_char = (uint8_t *)Q_PAR(me);
                p_char = memchr(p_char, ',', 23);
                p_char1 = p_char + 2;
                p_char = memchr(p_char1, '"', 15);
                if (p_char == NULL)
                {
                    return Q_HANDLED();
                }
                *p_char = 0;
                strcpy((char *)me->control.master_buffer, (char *)p_char1);
                len = strlen((char *)p_char1);
                p_char = memchr(p_char + 1, '\n', 29);
                if (p_char == NULL)
                {
                    return Q_HANDLED();
                }
                p_char++;
                p_char1 = memchr((char *)(p_char), 0x0d, 130);
                if (p_char1 != NULL)
                {
                    // finish the SMS text string
                    // because string must be finished for right behaviour
                    // of next strcpy() function
                    *p_char1 = 0;
                    p_char1 = me->control.master_buffer + len + 1;
                    strcpy((char *)p_char1, (char *)(p_char));
                }
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_idle);
}

static QState active_time_set(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_ACC;
            /* Format: "02/03/18,09:54:28+40" ie. "yy/mm/dd,hh:mm:ss+zz" */
            set_time_command();
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_MODULE_FAILURE, 0);
            return Q_TRAN(&active_idle);
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_CLOCK_SET_DONE, 0);
            return Q_TRAN(&active_idle);
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_super);
}

static QState active_gprs_super(Gsm *const me)
{
#if 1
    Softserial_print("s:g sup");
    print_eventid(Q_SIG(me));
#endif
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
        case Q_TIMEOUT_SIG:
        case EVENT_GSM_ERROR_RESPONSE:
        {
            Softserial_println("time gprs");
            QActive_post((QActive *)me->master, EVENT_GSM_GPRS_FAILURE, 0);
            return Q_TRAN(&active_idle);
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_idle);
}

static QState active_gprs_setup_interim(Gsm *const me)
{
    switch Q_SIG(me)
    {
        case Q_TIMEOUT_SIG:
        {
            set_gprs_parameters();
            return Q_TRAN(&active_gprs_setup);
        }
        case Q_EXIT_SIG:
        {
            QActive_disarm((QActive *)me);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_gprs_setup);
}

static QState active_gprs_setup(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_GPRS;
            me->control.timeout = 1;
            attach_gprs_command();
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            if (me->control.timeout)
            {
                me->control.timeout = 0;
                QActive_arm((QActive *)me, 3 sec);
                return Q_TRAN(&active_gprs_setup_interim);
            }
            else
            {
                QActive_post((QActive *)me->master, EVENT_GSM_GPRS_SETUP_DONE, 0);
                return Q_TRAN(&active_idle);
            }
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_gprs_super);
}

static QState active_gprs_start_interim(Gsm *const me)
{
    switch Q_SIG(me)
    {
        case Q_TIMEOUT_SIG:
        {
            send_gprs_status_command();
            return Q_TRAN(&active_gprs_start);
        }
    }
    return Q_SUPER(&active_gprs_start);
}

static QState active_gprs_start(Gsm *const me)
{
#if 1
    Softserial_print("s:g sta");
    print_eventid(Q_SIG(me));
#endif
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_GPRS;
            send_gprs_status_command();
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            if (me->control.response == GSM_GPRS_STATUS_START)
            {
                me->control.response = 0;
                send_gprs_status_command();
                return Q_HANDLED();
            }
            //QActive_post((QActive *)me->master, EVENT_GSM_GPRS_START_DONE, 0);
            //return Q_TRAN(&active_idle);
        }
        case EVENT_GSM_GPRS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if (me->control.response == GSM_GPRS_STATUS_START)
            {
                start_gprs_command();
            }
            else if (me->control.response == GSM_GPRS_STATUS_GPRSACT)
            {
                get_gprs_ip_command();
                return Q_TRAN(&active_gprs_start_interim);
            }
            else if (me->control.response == GSM_GPRS_STATUS_STATUS)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_GPRS_START_DONE, 0);
                return Q_TRAN(&active_idle);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_gprs_super);
}

static QState active_gprs_open_socket(Gsm *const me)
{
#if 1
    Softserial_print("s:g op");
    print_eventid(Q_SIG(me));
#endif
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_GPRS;
            send_gprs_status_command();
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_ACK_RESPONSE:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_GPRS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if ((me->control.response == GSM_GPRS_STATUS_STATUS) || (me->control.response == GSM_GPRS_STATUS_INITIAL))
            {
                open_tcp_socket_command();
            }
            else if (me->control.response == GSM_GPRS_CONNECT)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_GPRS_SOCKET_OPEN_DONE, 0);
                return Q_TRAN(&active_idle);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_gprs_super);
}

static QState active_gprs_send_interim(Gsm *const me)
{
    switch Q_SIG(me)
    {
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)me->master, EVENT_GSM_GPRS_SEND_DATA_DONE, 0);
            return Q_TRAN(&active_idle);
        }
    }
    return  Q_SUPER(&active_gprs_send_data);
}

static QState active_gprs_send_data(Gsm *const me)
{
    Softserial_print_byte(me->control.response);
#if 1
    Softserial_print(" -s:g se");
    print_eventid(Q_SIG(me));
#endif
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_GPRS;
            me->control.ix = 0;
            send_gprs_data();
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_COM_SEND_DONE:
        {
            if (me->control.ix)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_GPRS_BUFFER_EMPTY, 0);
            }
            return Q_HANDLED();
        }
        case EVENT_GSM_GPRS_BUFFER:
        {
            me->control.response = (uint8_t)(Q_PAR(me));
            if ((uint8_t)Q_PAR(me))
            {
                Softserial_println("11");
                me->control.ix = 0xFF;
                send_command_timeout(me->control.mssg_buf, 6 sec, 0);
            }
            else
            {
                Softserial_println("00");
                me->control.ix = 0;
                strcpy((char *)me->control.mssg_buf, "\x1A");
                send_command_timeout(me->control.mssg_buf, 0, 0);
            }
        }
        case EVENT_GSM_GPRS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if (me->control.response == GSM_GPRS_SEND_ACK)
            {
                QActive_arm((QActive*)me, 15 sec);
                return Q_TRAN(&active_gprs_send_interim);
            }
            else if (me->control.response == GSM_GPRS_PROMPT)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_GPRS_BUFFER_EMPTY, 0);
            }
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_gprs_super);
}

static QState active_gprs_close_socket(Gsm *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->control.busy = true;
            me->control.current_op = GSM_OP_GPRS;
            close_tcp_socket_command();
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_GPRS_RESPONSE:
        {
            me->control.response = (uint8_t)(Q_PAR(me) >> 16);
            if (me->control.response == GSM_GPRS_CLOSE)
            {
                QActive_post((QActive *)me->master, EVENT_GSM_GPRS_SOCKET_CLOSE_DONE, 0);
                return Q_TRAN(&active_idle);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            generic_exit_operations();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&active_gprs_super);
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
        }
        case SET_TIME:
        {
            /* Time is set as string in appropriate form required by GSM Module */
            strcpy((char *) GSM_buffer.buffer, (char *) data);
        }
        case GET_TIME:
        {
            /* Time is set as string in appropriate form required by GSM Module */
            strcpy((char *) data, (char *) GSM_buffer.buffer);
        }

        default:
        {
        }
    } // End of switch
}

static QState process_command()
{

    switch (GSM_buffer.cmd_byte)
    {
        case SEND_SMS:
        {
            /* Transit to Trasmit state */
            status = &GSM_transmit;
        }
        case SET_TIME:
        {
            /* Transit to Trasmit state */
            status = &GSM_transmit;
        }
        case GET_TIME:
        {
            /* Transit to Trasmit state */
            status = &GSM_transmit;
        }

        default:
        {
        }
    } // End of switch

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
        }
        case GET_TIME:
        {
        }
#endif
        default:
        {
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
        }
        case GET_TIME:
        {
        }
#endif
        default:
        {
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

    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            status = Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
        }
        case GSM_PROCESS_SIG:
        {
            //PORTD ^= (1 << 2);
            /* Process the cmd byte, and perform necessary action */
            init_statemachine();
            status = Q_TRAN(process_command());
        }

        default:
        {
            status = Q_SUPER(&QHsm_top);
        }
    }

}


static QState GSM_transmit(GSM *const me)
{

    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            /* Send the data to GSM Module */
            trasmit_command();
            status = Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
        }
        case SERIAL_TRANSMIT_SIG:
        {
            /* Recieved on completion of Serial transmit, goto Serial Recieve state */
            status = Q_TRAN(&GSM_recieve);
        }

        default:
        {
            status = Q_SUPER(&QHsm_top);
        }
    }

}


static QState GSM_recieve(GSM *const me)
{

    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            /* Start the maximum start of reception timeout */
            QActive_arm((QActive *)me, GSM_buffer.start_comm_tmout);
            status = Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
        }
        case SERIAL_RECIEVE_SIG:
        {
            /* Recieved on each Serial Recieve */
            //PORTD ^= (1 << 3);
            QActive_arm((QActive *)me, GSM_buffer.max_interchar_tmout);
            status = Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            /* Start of communication / Intercharacter timeout has occured */
            status = Q_TRAN(&GSM_process);
            //PORTD ^= (1 << 4);
        }

        default:
        {
            status = Q_SUPER(&QHsm_top);
        }
    }

}


static QState GSM_process(GSM *const me)
{

    void *next_state;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            Serial_SendStringNonBlocking((QActive *)&AO_GSM, "Done baby \r\n");
            process_data(next_state);
            status = Q_TRAN(next_state);
        }
        case Q_INIT_SIG:
        {
        }

        default:
        {
            status = Q_SUPER(&QHsm_top);
        }
    }

}

/*
    switch(GSM_buffer.cmd_byte)
    {
        case SEND_SMS:
        {
       }
        case SET_TIME:
        {
       }
        case GET_TIME:
        {
       }

        default:
        {
       }
    } // End of switch
*/
#endif