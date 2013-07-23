/**
 ******************************************************************************
 *
 * @file       gsm.h
 * @author     Lonewolf
 * @brief      GSM Header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
#ifndef GSM_H_
#define GSM_H_

#include "settings.h"
#include "app.h"
#include "com.h"

/* Size of the GSM Buffer */
#define GSM_BUFFER_SIZE 100
#if 0
typedef enum GSM_timeouts_t
{
    /**************************************************
     Various timeouts in ticks
     * For milliseconds - ( BSP_TICKS_PER_SEC * x)/1000
     * For seconds      - (BSP_TICKS_PER_SEC * x)
     **************************************************/
    START_TINY_COMM_TMOUT = (BSP_TICKS_PER_SEC * 20) / 1000, // 20ms
    START_SHORT_COMM_TMOUT = (BSP_TICKS_PER_SEC * 500) / 1000, // 500ms
    START_LONG_COMM_TMOUT = BSP_TICKS_PER_SEC * 1, // 1s
    START_XLONG_COMM_TMOUT = BSP_TICKS_PER_SEC * 5, // 5s
    START_XXLONG_COMM_TMOUT = BSP_TICKS_PER_SEC * 7, //7s
    MAX_INTERCHAR_TMOUT = (BSP_TICKS_PER_SEC * 20) / 1000, // 20ms
    MAX_MID_INTERCHAR_TMOUT = (BSP_TICKS_PER_SEC * 100) / 1000, // 100ms
    MAX_LONG_INTERCHAR_TMOUT = (int)(BSP_TICKS_PER_SEC * 1.5), // 1.5s
} GSM_timeouts;


/* Defnitions for commands that can be issued to the GSM */
typedef enum GSM_commands_t
{
    SEND_SMS = 1,
    SEND_SMS_DATA,
    SET_TIME,
    GET_TIME
} GSM_commands;

typedef enum internal_state_t
{
    SMS_CMD = 1,
    SMS_DATA,
} internal_state;

/* Structure to hold the config data of GSM */
typedef struct GSM_config_t
{
    void *x;
} GSM_config_descriptor;

/* Structure to hold SMS Data */
typedef struct GSM_sms_t
{
    unsigned char  phone_no[15];
    unsigned char time[21];
} GSM_sms_descriptor;

/* Structure to hold GSM related data for intercommunication between GSM Device and the application */
typedef struct GSM_buffer_t
{
    GSM_commands cmd_byte;
    internal_state state;
    QActive *active_object;
    GSM_config_descriptor config;
    GSM_sms_descriptor sms;
    GSM_timeouts start_comm_tmout;
    GSM_timeouts max_interchar_tmout;
    unsigned char buffer[GSM_BUFFER_SIZE];
} GSM_buffer_descriptor;


GSM_buffer_descriptor GSM_buffer;
#endif
/* Global Functions */
void GSM_parse_command(void *data);
//void GSM_config(QActive* master, QActive* com_drv, uint8_t* buffer);
void GSM_config(App *master, Com *com_drv);
void GSM_ctor(void);

extern struct Gsm_tag gsm_dev;

#endif                  /* gsm.h */
