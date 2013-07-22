/**
 ******************************************************************************
 *
 * @file       settings.h
 * @author     Lonewolf
 * @brief      System Settings and Inlcudes header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>

/* Sys timer tick per seconds */
#define BSP_TICKS_PER_SEC    100

/* Uncomment inorder to include Quantum Leaps */
#define Q_LEAPS


/* Inlcude Quantum Leaps related files */
#ifdef Q_LEAPS
#include <qpn_port.h>
#endif


/* System Peripheral Headers */
#include "serial.h"

/* Generic Utilities Headers */
#include "fifo_buffer.h"
//#include "util.h"


#ifdef Q_LEAPS
enum system_events
{
    TIME_TICK_SIG = Q_USER_SIG,     // time tick for all classes that sign up

    // Events related to system
    EVENT_SYSTEM_GSM_INIT,

    // Events related to UART
    EVENT_SERIAL_SEND_DONE,
    EVENT_SERIAL_,

    // Events related to COM
    EVENT_COM_OPEN_REQUEST,
    EVENT_COM_OPENED,
    EVENT_COM_SEND_REQUEST,
    EVENT_COM_SEND_DONE,
    EVENT_COM_RX_TIMEOUT,
    EVENT_COM_DATA_AVAILABLE,

    // Events related to GSM
    EVENT_GSM_ERROR_RESPONSE,
    EVENT_GSM_ACK_RESPONSE,
    EVENT_GSM_SMS_RESPONSE,
    EVENT_GSM_NETWORK_RESPONSE,
    EVENT_GSM_CLOCK_RESPONSE,
    EVENT_GSM_NETWORK_READ_REQUEST,
    EVENT_GSM_SMS_DELETE,
    EVENT_GSM_CLOCK_SET_DONE,
    EVENT_GSM_CLOCK_READ_DONE,
    EVENT_GSM_MODULE_FAILURE,
    EVENT_GSM_INIT_FAILURE,
    EVENT_GSM_BUSY,
    EVNET_GSM_MODULE_FAILURE,
    EVENT_GSM_NETWORK_CONNECTED,
    EVENT_GSM_NETWORK_ERROR,
    EVENT_GSM_SMS_FOUND,
    EVENT_GSM_SMS_NOT_FOUND,
    EVENT_GSM_INIT_DONE,
    EVENT_GSM_SMS_SENT,
    EVENT_GSM_SMS_DELETE_DONE,
    EVENT_GSM_SMS_READ_DONE,

    SERIAL_TRANSMIT_SIG,    // Serial transmit signal
    SERIAL_RECIEVE_SIG,     // Serial recieve signal
    GSM_PROCESS_SIG,        // Start processing the command
    GSM_DONE_SIG,           // GSM Finished processing
    GSM_SUCCESS_EVENT,
    GSM_FAILURE_EVENT,
    MAX_PUB_SIG,            // the last published signal
};
#endif


#endif         /* settings.h */