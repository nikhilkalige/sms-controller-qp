/**
 ******************************************************************************
 *
 * @file       app.c
 * @author     Lonewolf
 * @brief      Application functions
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#include "app.h"
#include "gsm.h"
#include "softserial.h"
#include "emon.h"
#include "edb.h"
#include "util.h"
#include "app_settings.h"

#define SOFT_DEBUG 1
#define DEBUG 1

#define VRED    1
#define VYELLOW 1
#define VBLUE   1
#define IRMS    1

#define PORT_MOTOR_ON   PORTD
#define PORT_MOTOR_OFF  PORTD

#define PIN_MOTOR_ON   1
#define PIN_MOTOR_OFF  2

static QState initiate_app(App *const me);
static QState init(App *const me);
//static QState active(App *const me);
static QState app_idle(App *const me);
static QState reciever(App *const me);
static QState validate(App *const me);
static QState action_status(App *const me);
static QState action_menu(App *const me);
static QState action_on(App *const me);
static QState action_off(App *const me);
static QState menu_parse(App *const me);

static QState change_pass(App *const me);
static QState add_no(App *const me);
static QState del_no(App *const me);
static QState en_dis_pwd(App *const me);
static QState enable_broadcast(App *const me);
static QState status_freq(App *const me);
static QState set_time(App *const me);
static QState generic_menu_handler(App *const me);
//extern gsm_driver_messages SMS_UNREAD;

/************************************************************************************************************************************
                                                    ***** Public Funtions *****
************************************************************************************************************************************/

void App_ctor(void)
{
    QActive_ctor(&app_dev.super, Q_STATE_CAST(&initiate_app));
}

/************************************************************************************************************************************
                                                    ***** Private Funtions *****
************************************************************************************************************************************/
static void update_session_expired(uint8_t bit)
{
    if (bit)
    {
        app_dev.session_expired |= _BV((app_dev.current_userid - 1));
    }
    else
    {
        app_dev.session_expired &= ~(_BV((app_dev.current_userid - 1)));
    }
}

static void update_session_menu(uint8_t bit)
{
    if (bit)
    {
        app_dev.session_expired |= (_BV((app_dev.current_userid - 1)) << 4);
    }
    else
    {
        app_dev.session_expired &= ~((_BV((app_dev.current_userid - 1))) << 4);
    }
}

static uint8_t check_menu_entry()
{
    if (!(app_dev.session_expired & ((_BV((app_dev.current_userid - 1))) << 4)))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint8_t extract_numbers(char *string, uint16_t *number)
{
    uint8_t length, return_value, i;
    return_value = 0;
    * number = 0;
    length  = strlen(string);
    if (length <= 5)
    {
        for (i = 0; i < length; i++)
        {
            if ((string[i]  < '0') || (string[i] > '9'))
            {
                break;
            }
        }
        if (i == (length))
        {
            i = 0;
            length--;
            while (i < length)
            {
                *number += (string[i] - 0x30);
                *number *= 10;
                i++;
            }
            *number += (string[i] - 0x30);
            return_value = 1;
        }
    }
    return return_value;
}

void update_system_settings(void)
{
    /*    db.open(EEPROM_SETTINGS_HEAD);
        db.updateRec(1,EDB_REC Module.Core_Values);
        return;*/
    edb_open(EEPROM_SETTINGS_HEAD);
    edb_updaterec(1, EDB_REC app_dev.system_settings);
    return;
}

static void print_eventid(uint8_t id)
{
    Softserial_print("-");
    Softserial_print_byte((uint8_t)id);
    Softserial_println("");
}

static uint8_t validate_user(void)
{
    uint8_t pos = 1;
    user tmp;
    edb_open(EEPROM_USER_HEAD);
    while (pos <= app_dev.system_settings.no_users)
    {
        edb_readrec(pos, EDB_REC tmp);
#ifdef SOFT_DEBUG
        Softserial_print((char *)tmp.phone_no);
        Softserial_print("  ");
        Softserial_print((char *)tmp.password);
        Softserial_print("  ");
        if (tmp.pwd_present)
        {
            Softserial_println("EN");
        }
        else
        {
            Softserial_println("DI");
        }
        Softserial_println(app_dev.current_phone_no);
#endif
        if (strstr((char *)app_dev.current_phone_no, (char *)tmp.phone_no) != NULL)
        {
            app_dev.current_userid = tmp.id;
            Softserial_println("phone match");
            /* Phone no found now check password */
            if (tmp.pwd_present)
            {
                char *pos;
                pos = strchr((char *)app_dev.mssg_buf, ' ');
                if (pos != NULL)
                {
                    *pos  = 0x00;
#ifdef SOFT_DEBUG
                    Softserial_print("   - PASS STRING  -");
                    Softserial_println((char *)app_dev.mssg_buf);
#endif
                    if (!(strcmp((char *)app_dev.mssg_buf, (char *)tmp.password)))
                    {
                        strcpy((char *)app_dev.mssg_buf, (pos + 1));
#ifdef SOFT_DEBUG
                        Softserial_println((char *)app_dev.mssg_buf);
                        Softserial_println(" - PASS");
#endif
                        return 1;
                    }
                }
            }
            else
            {
                return 1;
            }
            break;
        }
        pos++;
    }
    return 0;
}

static void initialize_system()
{
    uint8_t tmp;
    tmp = eeprom_read_byte((uint8_t *)FIRST_BOOT_ADD);
    if (tmp != (uint8_t)FIRST_BOOT_VALUE)
    {
        /*  The device is booting for the first time */
        /*  Create necessary database files in the EEPROM */
        /*  Update the FIRST_BOOT_VALUE */
#ifdef SOFT_DEBUG
        Softserial_println("FIRST BOOT");
#endif
        edb_create(EEPROM_USER_HEAD, USER_SIZE, (uint16_t)sizeof(user));
        edb_create(EEPROM_SETTINGS_HEAD, SETTINGS_SIZE, (uint16_t)sizeof(settings));

        /*  Add User  */
        user temp;
        temp.id = 1;
        strcpy_P((char *)temp.phone_no, PSTR("+919731472140"));
        strcpy_P((char *)temp.password, PSTR("1111"));
        Softserial_println((char *)temp.phone_no);
        Softserial_println((char *)temp.password);
        temp.pwd_present = 0;
        edb_open(EEPROM_USER_HEAD);
        edb_appendrec(EDB_REC temp);

        settings tmp_1;
        tmp_1.pass_present = false;
        tmp_1.broadcast_mssg = false;
        tmp_1.no_users = 1;
        tmp_1.status_freq = 0;
        edb_open(EEPROM_SETTINGS_HEAD);
        edb_appendrec(EDB_REC tmp_1);

        eeprom_write_byte((uint8_t *)FIRST_BOOT_ADD, FIRST_BOOT_VALUE);
    }
    /* Initialize the Class Module with values from EEPROM */
    edb_open(EEPROM_SETTINGS_HEAD);
    edb_readrec(1, EDB_REC app_dev.system_settings);
    //Module.Next_Status_Time = Module.Core_Values.Status_Freq * 60000;
}

static void perform_action(void)
{
    convert_uppercase((char *)app_dev.mssg_buf);
    if (strstr((char *)app_dev.mssg_buf, "STATUS") != NULL)
    {
        QActive_post((QActive *)&app_dev, EVENT_APP_ACTION_STATUS, 0);
    }
    else if (strstr((char *)app_dev.mssg_buf, "ON") != NULL)
    {
        QActive_post((QActive *)&app_dev, EVENT_APP_ACTION_ON, 0);
    }
    else if (strstr((char *)app_dev.mssg_buf, "OFF") != NULL)
    {
        QActive_post((QActive *)&app_dev, EVENT_APP_ACTION_OFF, 0);
    }
    else if (strstr((char *)app_dev.mssg_buf, "MENU") != NULL)
    {
        QActive_post((QActive *)&app_dev, EVENT_APP_ACTION_MENU, 0);
    }
    else
    {
        /*  what to do when all above are false */
        QActive_post((QActive *)&app_dev, EVENT_APP_ACTION_INVALID, 0);
    }
}

static void vi_string(unsigned char *string)
{
    unsigned char vi[7];
    uint8_t i;
    for (i = 0; i < 3; i++)
    {
        itoa((int)app_dev.VIrms[i], (char *)vi, 10);
        strcat((char *)string, (char *)vi);
        strcat_P((char *)string, PSTR("V "));
    }
    itoa((int)app_dev.VIrms[i], (char *)vi, 10);
    strcat((char *)string, (char *)vi);
    strcat_P((char *)string, PSTR("A"));
}

uint8_t update_menu_state(char *string)
{
    Softserial_print("Menu:str-");
    Softserial_print(string);
    Softserial_print("-");
    uint8_t length , i , value;
    length = strlen(string);
    value = 0;
    for (i = 0; i < length - 1; i++)
    {
        value = value + (string[i]) - 0x30;
        value *= 10;
    }
    value = value + (string[length - 1]) - 0x30;
    Softserial_print_byte(value);
    Softserial_println("");
    return value;
}

static void display_eeprom_database()
{
    uint8_t i;
    user tmp;
    Softserial_println_flash(PSTR("\nSYSTEM SETTINGS"));
    Softserial_print_flash(PSTR("Password-"));
    if (app_dev.system_settings.pass_present)
    {
        Softserial_println_flash(PSTR("Present"));
    }
    else
    {
        Softserial_println_flash(PSTR("Absent"));
    }
    Softserial_print_flash(PSTR("Broadcast-"));
    if (app_dev.system_settings.broadcast_mssg)
    {
        Softserial_println_flash(PSTR("Present"));
    }
    else
    {
        Softserial_println_flash(PSTR("Absent"));
    }
    Softserial_print("No User-");
    Softserial_print_byte((uint8_t)app_dev.system_settings.no_users);
    Softserial_println("");

    Softserial_println_flash(PSTR("USERS"));
    edb_open(EEPROM_USER_HEAD);
    for (i = 1; i <= app_dev.system_settings.no_users; i++)
    {
        edb_readrec(i, EDB_REC tmp);
        Softserial_print("Id-");
        Softserial_print_byte(tmp.id);
        Softserial_print(": ");
        Softserial_println((char *)tmp.phone_no);
        Softserial_println((char *)tmp.password);
        Softserial_print_flash(PSTR("Password-"));
        if (tmp.pwd_present)
        {
            Softserial_println_flash(PSTR("Present"));
        }
        else
        {
            Softserial_println_flash(PSTR("Absent"));
        }
    }
}

#if 0
static void handle_menu_option(uint8_t option)
{
    switch (option)
    {
        case CHANGE_PASS:
        {
            strcpy_P((char *)app_dev.mssg_buf, (char *)Rep1);
            //Module.Module_Status = SEND;
            update_session_menu(1);
            break;
        }
        case ADD_NO:
        {
            if (app_dev.system_settings.no_users == MAX_NO_USERS)
            {
                strcpy_P((char *)app_dev.mssg_buf, PSTR("MAX USERS EXCEEDED"));
                update_session_menu(0);
                update_session_expired(0);
                //Module.Menu_User_No[0] = 0x00;
            }
            else
            {
                strcpy_P((char *)app_dev.mssg_buf, (char *)Rep2);
                update_session_menu(1);
            }
            //Module.Module_Status = SEND;
            break;
        }

        case DEL_NO:
        {
            //Extract the phone no list from Memory and index and send it
            if (app_dev.system_settings.no_users > 1 )
            {
                app_dev.mssg_buf[0] = '\0';
                char ch[2] = "1";
                char phone[15];
                uint8_t i;
                User temp;
                db.open(EEPROM_USER_HEAD);
                for (i = 1; i < app_dev.system_settings.no_users; i++)
                {
                    strcat(app_dev.mssg_buf, ch);
                    ch[0]++;
                    strcat((char *)app_dev.mssg_buf, ".");
                    db.readRec(i + 1, EDB_REC temp);
                    strcat((char *)app_dev.mssg_buf, (char *)temp.Phone);
                    strcat((char *)app_dev.mssg_buf, (char *)"\r");
                }
                strcat_P((char *)app_dev.mssg_buf, (char *)Rep3);
                update_session_menu(1);
            }
            else
            {
                strcpy_P((char *)app_dev.mssg_buf, PSTR("No user to DELETE"));
                //Exit from MENU
                update_session_menu(0);
                update_session_expired(0);
                //Module.Menu_User_No[0] = 0x00;
            }
            //Module.Module_Status = SEND;
            break;
        }

        case EN_DIS_PASS:
        {
            strcpy_P((char *)app_dev.mssg_buf, Rep4);
            //Module.Module_Status = SEND;
            update_session_menu(1);
            break;
        }

        case EN_BROADCAST:
        {
            strcpy_P((char *)app_dev.mssg_buf, Rep5);
            //Module.Module_Status = SEND;
            update_session_menu(1);
            break;
        }

        case STATUS_FREQ:
        {
            strcpy_P((char *)app_dev.mssg_buf, Rep6);
            //Module.Module_Status = SEND;
            update_session_menu(1);
            break;
        }

        case SET_TIME:
        {
            if (gsm.SetTime())
            {
                app_dev.mssg_buf[0] = 0;
                gsm.GetTime((char *)app_dev.mssg_buf, 20);
                strcat((char *)app_dev.mssg_buf, "\n");
                strcat_P((char *)app_dev.mssg_buf, Rep7);
            }
            else
            {
                strcpy_P((char *)app_dev.mssg_buf, PSTR("ERROR"));
            }
            //Module.Module_Status = SEND;
            update_session_expired(0);
            //Module.Menu_User_No[0] = 0x00;
            update_session_menu(0);
            break;
        }

        default:
        {
            // Invalid Option Discontinue MENU Operations Alert the USER
            strcpy_P((char *)app_dev.mssg_buf, PSTR("INVALID_COMMAND"));
            //Module.Module_Status = SEND;
            // Set Menu to default
            update_session_expired(0);
            //Module.Menu_User_No[0] = 0x00;
            update_session_menu(0);
#ifdef SOFT_DEBUG
            //Softserial_print(Module.Menu.Current_Option);
            Softserial_println("    ERROR");
#endif
            break;
        }
    }
}
#endif
/************************************************************************************************************************************
                                                    ***** State Machines *****
************************************************************************************************************************************/

static QState initiate_app(App *const me)
{
    return Q_TRAN(&init);
}

static QState init(App *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            QActive_post((QActive *)&gsm_dev, EVENT_SYSTEM_START_AO, 0);
            Softserial_println("INIT APP TIME");
            QActive_arm((QActive *)me, 1 sec);
            initialize_system();
#ifdef DEBUG
            display_eeprom_database();
#endif
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            Softserial_println("INIT APP T RECIEVED");
            QActive_post((QActive *)&gsm_dev, EVENT_SYSTEM_GSM_INIT, 0);
            //return Q_TRAN(&set_time);
            return Q_HANDLED();
        }
        case EVENT_GSM_INIT_DONE:
        {
            Softserial_println("GSM INIT DONE");
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_NETWORK_READ_REQUEST, 0);
            return Q_HANDLED();
        }
        case EVENT_GSM_NETWORK_ERROR:
        {
            /* Retry after 30 seconds */
            QActive_arm((QActive *)me, 30 sec);
            return Q_HANDLED();
        }
        case EVENT_GSM_NETWORK_CONNECTED:
        {
            /* Registration Complete */
            me->mssg_buf[0] = ((uint16_t)&Menu_Strings & 0xFF);
            me->mssg_buf[1] = (((uint16_t)&Menu_Strings >> 8) & 0xFF);
            return Q_TRAN(&app_idle);
        }
    }
    return Q_SUPER(&QHsm_top);
}

static QState app_idle(App *const me)
{
#if 0
    Softserial_print("state:idle");
    print_eventid(Q_SIG(me));
#endif
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_NEW_RECIEVED:
        {
            /* New sms recieved, find the position of the new sms */
#ifdef DEBUG
            Softserial_println("New SMS");
#endif
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_CHECK_PRESENCE, SMS_UNREAD);
            //QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_READ_REQUEST, 0x01);
            //return Q_HANDLED();
            return Q_TRAN(&reciever);
#if 0
            strcpy_P(me->current_phone_no, PSTR("+919731472140"));
            return Q_TRAN(&action_menu);
#endif
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}

static QState reciever(App *const me)
{
    uint8_t index;
#if 0
    Softserial_print("state:reciever");
    print_eventid(Q_SIG(me));
#endif

    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_FOUND:
        {
#ifdef DEBUG
            //  Softserial_println("sms found");
#endif
            /* Unread SMS found */
            index = Q_PAR(me);
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_READ_REQUEST, index);
#ifdef DEBUG
            // Softserial_println("sms found end");
#endif
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_READ_DONE:
        {
            Softserial_println("read done");
            ///strcpy((char *)me->current_phone_no, (char *)me->buffer);
            //index = strlen((char *)me->buffer);
            // strcpy((char *)(char *)me->mssg_buf, (char *)(me->buffer + index + 1));
#if 0
            return Q_TRAN(&validate);
#endif
#if 0
            strcpy_P(me->current_phone_no, PSTR("+919731472140"));
            return Q_TRAN(&action_status);
#endif
            return Q_TRAN(&action_menu);
        }
        case EVENT_GSM_SMS_NOT_FOUND:
        {
            return Q_TRAN(&app_idle);
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&app_idle);
}

static QState validate(App *const me)
{
    uint8_t index;
#if 0
    Softserial_print("state:validate");
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
            /* Validate the phoneno, password */
            index = 0;
            index = validate_user();
            if (!index)
            {
                Softserial_println("here");
                //return Q_TRAN(&app_idle);
            }
            else
            {
                Softserial_println("here 1");
                convert_uppercase((char *)me->mssg_buf);
                Softserial_println((char *)me->mssg_buf);
                if (strstr((char *)me->mssg_buf, "STATUS") != NULL)
                {
                    return Q_TRAN(&action_status);
                }
                else if (strstr((char *)me->mssg_buf, "ON") != NULL)
                {
                    Softserial_println("going on");
                    return Q_TRAN(&action_on);
                }
                else if (strstr((char *)me->mssg_buf, "OFF") != NULL)
                {
                    Softserial_println("going off");
                    return Q_TRAN(&action_off);
                }
                else if (strstr((char *)me->mssg_buf, "MENU") != NULL)
                {
                    return Q_TRAN(&action_menu);
                }
                else
                {
                    //return Q_TRAN( &app_idle);
                }
                return Q_HANDLED();
            }
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&app_idle);
}

static QState action_status(App *const me)
{
    /******************************************************
    * Check the voltage of the lines and current .
    * Decide whether the motor is on or off based on the current flow
    * If CT is absent, use the Motor_On variable to do the same.
    * Send status report :  Format as follows
    * DD/MM/YYYY HH:MM:SS AM/PM ON/OFF RY YB BR I
    ******************************************************/
    Softserial_print("state:ac_status");
    print_eventid(Q_SIG(me));
    uint8_t adc_pin;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->i_generic = 0;
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_CLOCK_READ, 0);
            return Q_HANDLED();
        }
        case EVENT_GSM_CLOCK_READ_DONE:
        {
            strcpy((char *)me->mssg_buf, (char *)me->buffer);
            QActive_post((QActive *)&emon_dev, EVENT_EMON_READ_ENTITY, VRED);
            return Q_HANDLED();
        }
        case EVENT_EMON_MEASUREMENT_DONE:
        {
            if (!me->i_generic)
            {
                adc_pin = VYELLOW;
            }
            else if (me->i_generic == 1)
            {
                adc_pin = VBLUE;
            }
            else if (!me->i_generic == 2)
            {
                adc_pin = IRMS;
            }
            else
            {
                vi_string(me->mssg_buf);
                Softserial_println((char *)me->mssg_buf);
                QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
                return Q_HANDLED();
            }
            me->VIrms[me->i_generic] = (uint16_t)Q_PAR(me);
            me->i_generic++;
            QActive_post((QActive *)&emon_dev, EVENT_EMON_READ_ENTITY, adc_pin);
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_SENT:
        {
            return Q_TRAN(&app_idle);
        }
        case Q_EXIT_SIG:
        {
            Softserial_println("exit");
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&validate);
}

static QState action_on(App *const me)
{
#if 0
    Softserial_print("state:act on");
    print_eventid(Q_SIG(me));
#endif
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            if (!me->motor_on)
            {
                PORT_MOTOR_ON |= _BV(PIN_MOTOR_ON);
                QActive_arm((QActive *)me, 4 sec);
            }
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            if (me->motor_on)
            {
                QActive_post((QActive *)&gsm_dev, EVENT_GSM_CLOCK_READ, 0);
            }
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            PORT_MOTOR_ON &= ~(_BV(PIN_MOTOR_ON));
            me->motor_on = 1;
            return Q_TRAN(&action_on);
        }
        case EVENT_GSM_CLOCK_READ_DONE:
        {
            strcpy((char *)me->mssg_buf, (char *)me->buffer);
            strcat((char *)me->mssg_buf, " ON");
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_SENT:
        {
            return Q_TRAN(&app_idle);
        }
        case Q_EXIT_SIG:
        {
            QActive_disarm((QActive *)me);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&validate);
}

static QState action_off(App *const me)
{
#if 0
    Softserial_print("state:act off");
    print_eventid(Q_SIG(me));
#endif
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            if (me->motor_on)
            {
                PORT_MOTOR_OFF |= _BV(PIN_MOTOR_OFF);
                QActive_arm((QActive *)me, 4 sec);
            }
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            if (!me->motor_on)
            {
                QActive_post((QActive *)&gsm_dev, EVENT_GSM_CLOCK_READ, 0);
            }
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            PORT_MOTOR_OFF &= ~(_BV(PIN_MOTOR_OFF));
            me->motor_on = 0;
            return Q_TRAN(&action_off);
        }
        case EVENT_GSM_CLOCK_READ_DONE:
        {
            strcpy((char *)(char *)me->mssg_buf, (char *)me->buffer);
            strcat((char *)me->mssg_buf, " OFF");
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_SENT:
        {
            return Q_TRAN(&app_idle);
        }
        case Q_EXIT_SIG:
        {
            QActive_disarm((QActive *)me);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&validate);
}

static void menu_settings_display()
{
    Softserial_print_byte(app_dev.session_expired);
    Softserial_print(" -Userid-");
    Softserial_print_byte(app_dev.current_userid);
    Softserial_print(" -Session-");
    if (app_dev.session_expired & _BV((app_dev.current_userid - 1)))
    {
        Softserial_print("Old   ");
    }
    else
    {
        Softserial_print("New  ");
    }
    Softserial_print("Menu-");
    if (app_dev.session_expired & (_BV((app_dev.current_userid - 1)) << 4))
    {
        Softserial_println("2nd");
    }
    else
    {
        Softserial_println("1st");
    }
}

/* TODO: make changes in the super state to handle sessions */
static QState action_menu(App *const me)
{
#if 1
    Softserial_print("state:act menu");
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
            update_session_expired(1);
            /* Check for ongoing session */
            if (!(me->session_expired & _BV((me->current_userid - 1))))
            {
                QActive_post((QActive *)me, EVENT_APP_START_SESSION, 0);
            }
            else
            {
                /* Menu has already been entered, so parse the command */
                me->current_userid = 1;
                Softserial_println("Old Session");
                menu_settings_display();
                return Q_TRAN(&menu_parse);
            }
            return Q_HANDLED();
        }
        case EVENT_APP_START_SESSION:
        {
            /* Set session bit, update timings and send menu list of commands message */
            Softserial_println("New Session");
            update_session_expired(1);
            update_session_menu(0);
            me->session_details[(me->current_userid - 1)].session_timing = 5;
            me->session_details[(me->current_userid - 1)].menu_option = 0;
            me->mssg_buf[0] = ((uint16_t)&Menu_Strings & 0xFF);
            me->mssg_buf[1] = (((uint16_t)&Menu_Strings >> 8) & 0xFF);
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 1);
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_SENT:
        {
            return Q_TRAN(&app_idle);
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&validate);
}

static QState menu_parse(App *const me)
{
#if 1
    Softserial_print("state:menu par");
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
            menu_settings_display();
            Softserial_println((char *)me->mssg_buf);
            /* Check if the menu is entered for the first time */
            if (!(me->session_expired & ((_BV((me->current_userid - 1))) << 4)))
            {
                me->session_details[(me->current_userid - 1)].menu_option = update_menu_state((char *)me->mssg_buf);
            }
            switch (me->session_details[(me->current_userid - 1)].menu_option)
            {
                case CHANGE_PASS:
                    Softserial_println("Password");
                    return Q_TRAN(&change_pass);
                case ADD_NO:
                    Softserial_println("Add no");
                    return Q_TRAN(&add_no);
                case DEL_NO:
                    Softserial_println("Del no");
                    return Q_TRAN(&del_no);
                case EN_DIS_PASS:
                    Softserial_println("EN Pass");
                    return Q_TRAN(&en_dis_pwd);
                case EN_BROADCAST:
                    Softserial_println("En braod");
                    return Q_TRAN(&enable_broadcast);
                case STATUS_FREQ:
                    Softserial_println("Status fr");
                    return Q_TRAN(&status_freq);
                case SET_TIME:
                    Softserial_println("Set time");
                    return Q_TRAN(&set_time);
                default:
                    Softserial_println("Generic");
                    return Q_TRAN(&generic_menu_handler);
            }
        }
    }
    return Q_SUPER(&action_menu);
}

static QState change_pass(App *const me)
{
    char *pos;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            if (!check_menu_entry)
            {
                strcpy_P((char *)me->mssg_buf, Rep1);
                update_session_menu(1);
            }
            else
            {
                /* Check for space at the end or middle and terminate the string at its first occurence */
                pos = strchr((char *)me->mssg_buf, ' ');
                if (pos != NULL)
                {
                    *pos  = 0x00;
                }
                /* Check if the length is less than 20 */
                uint8_t length;
                length = strlen((char *)me->mssg_buf);
                if ((length < (MAX_PASSWORD_LENGTH + 1)) && (length))
                {
                    /* Change the PASSWORD */
#if 0
                    Pass temp;
                    edb_open(EEPROM_PASS_HEAD);
                    strcpy((char *)me->user_settings.password, (char *)me->mssg_buf);
                    edb_updaterec(1, EDB_REC temp);
#endif
                    strcat_P((char *)me->mssg_buf, PSTR(" is the NEW PASSWORD"));
                }
                else
                {
                    strcpy_P((char *)me->mssg_buf, PSTR("INVALID"));
                }
                update_session_expired(0);
            }
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&menu_parse);
}

static QState add_no(App *const me)
{
    char *pos;
    user tmp;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            if (!check_menu_entry())
            {
                if (me->system_settings.no_users == MAX_NO_USERS)
                {
                    strcpy_P((char *)me->mssg_buf, PSTR("MAX USERS EXCEEDED"));
                    update_session_menu(0);
                    update_session_expired(0);
                    /* Module.Menu_User_No[0] = 0x00; */
                }
                else
                {
                    strcpy_P((char *)me->mssg_buf, (char *)Rep2);
                    update_session_menu(1);
                }
            }
            else
            {
                /*  Add the No to EEPROM and update no of users */
                /*  Check for space at the end or middle and terminate the string at its first occurence */
                pos = strchr((char *)me->mssg_buf, ' ');
                if (pos != NULL)
                {
                    *pos  = 0x00;
                }
                uint8_t length;
                length = strlen((char *)me->mssg_buf);
                if ((length < MAX_NO_LENGTH) && (me->system_settings.no_users  < MAX_NO_USERS) && (length))
                {
                    /*  Add no */
                    me->system_settings.no_users++;
                    update_system_settings();
                    edb_open(EEPROM_USER_HEAD);
                    strcpy((char *)tmp.phone_no, (char *)me->mssg_buf);
                    tmp.id = me->system_settings.no_users;
                    tmp.password[0] = 0;
                    tmp.pwd_present = 0;
#if 0
                    Softserial_println("add no settings")                    ;
                    Softserial_println(me->mssg_buf);
                    Softserial_println(tmp.phone_no);
#endif
                    edb_appendrec(EDB_REC tmp);
                    edb_readrec(me->system_settings.no_users, EDB_REC tmp);
                    strcpy((char *)me->mssg_buf, (char *)tmp.phone_no);
                    strcat_P((char *)me->mssg_buf, PSTR("    ADDED"));
                }
                else
                {
                    strcpy_P((char *)me->mssg_buf, PSTR("INVALID"));
                }
                update_session_expired(0);
            }
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&menu_parse);
}

/* TODO: Update userid after delete */
static QState del_no(App *const me)
{
    char ch[2];
    uint8_t i;
    user tmp;
    char *pos;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            if (!check_menu_entry())
            {
                if (me->system_settings.no_users > 1 )
                {
                    me->mssg_buf[0] = '\0';
                    strcpy(ch, "1");
                    edb_open(EEPROM_USER_HEAD);
                    for (i = 1; i < me->system_settings.no_users; i++)
                    {
                        strcat((char *)me->mssg_buf, ch);
                        ch[0]++;
                        strcat((char *)me->mssg_buf, ".");
                        edb_readrec(i + 1, EDB_REC tmp);
                        strcat((char *)me->mssg_buf, (char *)tmp.phone_no);
                        strcat((char *)me->mssg_buf, (char *)"\r");
                    }
                    strcat_P((char *)me->mssg_buf, (char *)Rep3);
                    update_session_menu(1);
                }
                else
                {
                    strcpy_P((char *)me->mssg_buf, PSTR("No user to DELETE"));
                    //Exit from MENU
                    update_session_expired(0);
                }
            }
            else
            {
                // Delete the NO and Update no of users
                // Check for space at the end or middle and terminate the string at its first occurence
                pos = strchr((char *)me->mssg_buf, ' ');
                if (pos != NULL)
                {
                    *pos  = 0x00;
                }
                // Convert the first Character to Hex from Ascii
                me->mssg_buf[0] = me->mssg_buf[0] - 0x30;
                if ((me->mssg_buf[0] != 0) && (me->mssg_buf[0] < me->system_settings.no_users))
                {
                    i = me->mssg_buf[0];
                    edb_open(EEPROM_USER_HEAD);
                    edb_readrec((i + 1), EDB_REC tmp);
                    strcpy((char *)me->mssg_buf, (char *)tmp.phone_no);
                    edb_deleterec((i + 1));
                    strcat_P((char *)me->mssg_buf , PSTR("  has been Deleted"));

                    me->system_settings.no_users--;
                    update_system_settings();
                }
                else
                {
                    strcpy_P((char *)me->mssg_buf, PSTR("INVALID"));
                }
                update_session_expired(0);
            }
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&menu_parse);
}

static QState en_dis_pwd(App *const me)
{
    char *pos;
    uint8_t length;
    user temp;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            if (!check_menu_entry())
            {
                strcpy_P((char *)app_dev.mssg_buf, Rep4);
                update_session_menu(1);
            }
            else
            {
                // Check for EN or DI in the string and Update Pass_Enable accordingly
                // Check for space at the end or middle and terminate the string at its first occurence
                pos = strchr((char *)me->mssg_buf, ' ');
                if (pos != NULL)
                {
                    *pos = 0x00;
                }
                length = strlen((char *)me->mssg_buf);
                if ((length < 3) && (length))
                {
                    /* Read current user from memory and update the settings TODO */
                    convert_uppercase((char *)me->mssg_buf);
                    edb_open(EEPROM_USER_HEAD);
                    edb_readrec(me->current_userid - 1, EDB_REC temp);
                    if (strstr_P((char *)me->mssg_buf, PSTR("EN")))
                    {
                        temp.pwd_present = 1;
                        strcpy_P((char *)me->mssg_buf, PSTR("PASSWORD ENABLED"));
                    }
                    else if (strstr_P((char *)me->mssg_buf, PSTR("DI")))
                    {
                        strcpy_P((char *)me->mssg_buf, PSTR("PASSWORD DISABLED"));
                        temp.pwd_present = 0;
                    }
                    else
                    {
                        strcpy_P((char *)me->mssg_buf, PSTR("INVALID"));
                    }
                    edb_updaterec(me->current_userid, EDB_REC temp);
                    update_system_settings();
                }
                else
                {
                    strcpy_P((char *)me->mssg_buf, PSTR("INVALID"));
                }
                update_session_expired(0);
            }
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&menu_parse);
}

static QState enable_broadcast(App *const me)
{
    char *pos;
    uint8_t length;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            if (!check_menu_entry())
            {
                strcpy_P((char *)app_dev.mssg_buf, Rep5);
                update_session_menu(1);
            }
            else
            {
                // Check for EN or DI in the string and Update Broadcast_Enable accordingly
                // Check for space at the end or middle and terminate the string at its first occurence
                pos = strchr((char *)me->mssg_buf, ' ');
                if (pos != NULL)
                {
                    *pos = 0x00;
                }
                length = strlen((char *)me->mssg_buf);
                if ((length < 3) && (length))
                {
                    convert_uppercase((char *)me->mssg_buf);
                    if (strstr_P((char *)me->mssg_buf, PSTR("EN")))
                    {
                        strcpy_P((char *)me->mssg_buf, PSTR("BROADCAST ENABLED"));
                        me->system_settings.broadcast_mssg = true;
                    }
                    else if (strstr_P((char *)me->mssg_buf, PSTR("DI")))
                    {
                        strcpy_P((char *)me->mssg_buf, PSTR("BROADCAST DISABLED"));
                        me->system_settings.broadcast_mssg = false;
                    }
                    else
                    {
                        strcpy_P((char *)me->mssg_buf, PSTR("INVALID"));
                    }
                    update_system_settings();
                }
                else
                {
                    strcpy_P((char *)me->mssg_buf, PSTR("INVALID"));
                }
                update_session_expired(0);
            }
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&menu_parse);
}

static QState status_freq(App *const me)
{
    char *pos;
    char num[4];
    uint8_t length;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            if (!check_menu_entry())
            {
                strcpy_P((char *)app_dev.mssg_buf, Rep6);
                update_session_menu(1);
            }
            else
            {
                // Check for Minutes , Terminate higher values at 255 , If 0 disable Auto Status Messages
                // Check for space at the end or middle and terminate the string at its first occurence
                pos = strchr((char *)me->mssg_buf, ' ');
                if (pos != NULL)
                {
                    *pos = 0x00;
                }
                length = strlen((char *)me->mssg_buf);
                if ((length <= 3) && (length))
                {
                    uint16_t freq;
                    if (extract_numbers((char *)me->mssg_buf, &freq))
                    {
                        me->system_settings.status_freq = (uint8_t)freq;
                        /*Module.Next_Status_Time = me->system_settings.status_freq * 60000;*/
                        itoa(me->system_settings.status_freq, num, 10);
                        strcpy_P((char *)me->mssg_buf, PSTR("Status Mssg will be sent at interval of "));
                        strcat((char *)me->mssg_buf, num);
                        strcat_P((char *)me->mssg_buf, PSTR(" Minutes"));
                    }
                    else
                    {
                        strcpy_P((char *)me->mssg_buf, PSTR("INVALID"));
                    }
                    update_system_settings();
                }
                else
                {
                    strcpy_P((char *)me->mssg_buf, PSTR("INVALID"));
                }
                update_session_expired(0);
            }
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&menu_parse);
}

static QState set_time(App *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            // if (!check_menu_entry())
            //  {
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_READ_EXTRACT_TIME, 0);
            // }
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_READ_DONE:
        {
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_CLOCK_SET, 0);
            return Q_HANDLED();
        }
        case EVENT_GSM_CLOCK_SET_DONE:
        {
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_CLOCK_READ, 0);
            return Q_HANDLED();
        }
        case EVENT_GSM_CLOCK_READ_DONE:
        {
            strcat((char *)me->mssg_buf, "\n");
            strcat_P((char *)me->mssg_buf, Rep7);
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
            return Q_HANDLED();
        }
        case EVENT_GSM_PARSING_ERROR:
        {
            strcpy_P((char *)app_dev.mssg_buf, PSTR("ERROR"));
            return Q_TRAN(&app_idle);
        }
        case Q_EXIT_SIG:
        {
            update_session_expired(0);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&menu_parse);
}

static QState generic_menu_handler(App *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            if (!check_menu_entry())
            {
                // Invalid Option Discontinue MENU Operations Alert the USER
                strcpy_P((char *)me->mssg_buf, PSTR("INVALID_COMMAND"));
#ifdef SOFT_DEBUG
                //Softserial_print(Module.Menu.Current_Option);
                Softserial_println("    ERROR");
#endif
            }
            else
            {
                strcpy_P((char *)me->mssg_buf, PSTR("INVALID_COMMAND"));
#ifdef SOFT_DEBUG
                //Softserial_print(Module.Menu.Current_Option);
                Softserial_println("    ERROR");
#endif
            }
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            update_session_expired(0);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&menu_parse);
}

#if 0
/********************************************************
    This function returns the current and voltage in three lines
    The string format returned will be as follows
    rrrV yyyV bbbV iiiA
    Since V or I max will be of 5 characters
    Please make sure the string has atleast 28 uint8_ts of free space.
    Also note that strcat is used inplace of strcpy
    ********************************************************/
static QState measure_vi(App *const me)
{
    uint8_t adc_pin;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            me->i_generic = 0;
            QActive_post((QActive *)&emon_dev, EVENT_EMON_READ_ENTITY, VRED);
            return Q_HANDLED();
        }
        case EVENT_EMON_MEASUREMENT_DONE:
        {
            if (!me->i_generic)
            {
                adc_pin = VYELLOW;
            }
            else if (me->i_generic == 1)
            {
                adc_pin = VBLUE;
            }
            else if (!me->i_generic == 2)
            {
                adc_pin = IRMS;
            }
            else
            {
                return Q_TRAN(action_status);
            }
            me->VIrms[me->i_generic] = (uint16_t)Q_PAR(me);
            me->i_generic++;
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}

uint8_t num = 0;
#endif
#if 0
static QState init(App *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            QActive_post((QActive *)&gsm_dev, EVENT_SYSTEM_START_AO, 0);
            Softserial_println("INIT APP TIME");
            QActive_arm((QActive *)me, 1 sec);
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            Softserial_println("INIT APP T RECIEVED");
            /* Added for emon test */
            QActive_post((QActive *)&emon_dev, EVENT_EMON_READ_ENTITY, 0);
#if 0
            QActive_post((QActive *)&gsm_dev, EVENT_SYSTEM_GSM_INIT, 0);
            /* QActive_arm((QActive *)me, 1 sec); */
#endif
            return Q_HANDLED();
        }
        case EVENT_EMON_MEASUREMENT_DONE:
        {
            if (!num)
            {
                Softserial_print("Vrms - ");
                Softserial_print_uint8_t((uint16_t)Q_PAR(me));
                QActive_post((QActive *)&emon_dev, EVENT_EMON_READ_ENTITY, 1);

                num++;
            }
            else if (num == 1)
            {
                Softserial_print("\t");
                Softserial_print_uint8_t((uint16_t)Q_PAR(me));
                QActive_post((QActive *)&emon_dev, EVENT_EMON_READ_ENTITY, 2);
                num++;
            }
            else if (num == 2)
            {
                Softserial_print("\t");
                Softserial_print_uint8_t((uint16_t)Q_PAR(me));
                Softserial_println("");
                QActive_post((QActive *)&emon_dev, EVENT_EMON_READ_ENTITY, 6);
                num++;
            }
            else if (num == 3)
            {
                Softserial_print("Irms - ");
                Softserial_print_uint8_t((uint16_t)Q_PAR(me));
                Softserial_println("");
                QActive_post((QActive *)&emon_dev, EVENT_EMON_READ_ENTITY, 0);
                num++;
            }
            return Q_HANDLED();
        }
        case EVENT_GSM_INIT_DONE:
        {
            Softserial_println("GSM INIT DONE");
            return Q_TRAN(&active);
        }
        case EVENT_GSM_INIT_FAILURE:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_MODULE_FAILURE:
        {
            Softserial_println("module error");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QActive_disarm((QActive *)me);
            return Q_HANDLED();
        }
        default:
        {
            return Q_SUPER(&QHsm_top);
        }
    }
}

static QState active(App *const me)
{
    uint8_t temp;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            Softserial_println("Posting GSM init ");
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_NETWORK_READ_REQUEST, 0);
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_BUSY:
        {
            QActive_arm((QActive *)me, 1);
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            return Q_HANDLED();
        }
        case EVNET_GSM_MODULE_FAILURE:
        {
            // Error
            // Holy cow
            Softserial_println("module error");
            return Q_HANDLED();
        }
        case EVENT_GSM_NETWORK_ERROR:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_NETWORK_CONNECTED:
        {
#if 1
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_READ_REQUEST, 13);
#endif
#if 0
            /* Test for Delete all SMS */
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_DELETE, 13);
#endif
#if 0
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_CHECK_PRESENCE, SMS_UNREAD);
#endif
#if 0
            strcpy(me->buffer, "9901606873");
            uint8_t len;
            uint8_t *p;
            len = strlen("9901606873");
            p = me->buffer + len + 1;
            strcpy(p, "Hi Wanderer");
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_SMS_SEND, 0);
#endif
            //QActive_post((QActive *)&gsm_dev, EVENT_GSM_CLOCK_READ, 0);
            // Ready to roll my friend
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_READ_DONE:
        {
            temp = strlen((char *)me->buffer);
            Softserial_println("SMS Read");
            Softserial_print((char *)me->buffer);
            Softserial_print("----");
            Softserial_println((char *)(me->buffer + temp + 1));
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_DELETE_DONE:
        {
            Softserial_println("Delete Success");
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_FOUND:
        {
            temp = (uint8_t)Q_PAR(me);
            Softserial_print("sms found: ");
            Softserial_print_uint8_t(temp);
            Softserial_println("");
            return Q_HANDLED();
        }
        case EVENT_GSM_CLOCK_READ_DONE:
        {
            Softserial_println("time read success");
            Softserial_println((char *)me->buffer);
            strcpy((char *)me->buffer, "\"13/08/03,19:32:00+10\"");
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_CLOCK_SET, 0);
            return Q_HANDLED();
        }
        case EVENT_GSM_SMS_SENT:
        {
            Softserial_println("SMS Sent Bro");
            return Q_HANDLED();
        }
        case EVENT_GSM_CLOCK_SET_DONE:
        {
            Softserial_println("time set success");
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
#endif