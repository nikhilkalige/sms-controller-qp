/**
 ******************************************************************************
 *
 * @file       App.h
 * @author     Lonewolf
 * @brief      Application Header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#ifndef APP_H_
#define APP_H_


#include <qpn_port.h>


void App_ctor(void);

typedef enum
{
    CHANGE_PASS = 1,
    ADD_NO,
    DEL_NO,
    EN_DIS_PASS,
    EN_BROADCAST,
    STATUS_FREQ,
    SET_TIME,
    GPRS,
    EN_TCP,
    COSM,
    PING_FREQ,
    CALIBRATE,
    RESET
}
Menu_Option;


typedef struct user_tag
{
    unsigned char phone_no[15];
    unsigned char password[5];
    uint8_t pwd_present;
} user;

typedef struct
{
    uint8_t pass_present;
    uint8_t broadcast_mssg;
    uint8_t no_users;
    uint8_t status_freq; // In Minutes
} settings;

typedef struct
{
    uint8_t session_timing;
    Menu_Option menu_option;
} user_session;

typedef struct App_tag
{
    QActive super;
    /* Public Members */
    uint8_t buffer[100];
    /* Energy measurements */
    uint16_t VIrms[4];
    user user_settings;
    settings system_settings;
    unsigned char current_phone_no[15];
    unsigned char mssg_buf[75];
    uint8_t motor_on;
    uint8_t i_generic;
    uint8_t current_userid;
    uint8_t session_expired;
    user_session session_details[4];
} App;

App app_dev;

#endif                  /* app.h */