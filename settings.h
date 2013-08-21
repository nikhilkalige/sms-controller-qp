/**
 ******************************************************************************
 *
 * @file       settings.h
 * @author     Lonewolf
 * @brief      System Settings and Inlcudes header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
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

/* Define if using Quantum Kernel */
#undef QPK

/* Generic Utilities Headers */
#include "fifo_buffer.h"
//#include "util.h"

/* Timer delay Generation
 * Minimum Resolution = 1/BSP_TICKS_PER_SEC
 * No of ticks = Time in seconds / Minimum Resolution
 */

#define ms       * BSP_TICKS_PER_SEC/1000
#define sec      * BSP_TICKS_PER_SEC


#ifdef Q_LEAPS
enum system_events
{
    TIME_TICK_SIG = Q_USER_SIG,     // time tick for all classes that sign up

    // Events related to system
    EVENT_SYSTEM_GSM_INIT,
    EVENT_SYSTEM_START_AO,

    // Events related to UART
    EVENT_SERIAL_SEND_DONE,
    EVENT_SERIAL_,

    // Events related to COM
    EVENT_COM_OPEN_REQUEST,
    EVENT_COM_OPEN_DONE,
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
    EVENT_GSM_SMS_READ_REQUEST,
    EVENT_GSM_SMS_DELETE,
    EVENT_GSM_SMS_CHECK_PRESENCE,
    EVENT_GSM_CLOCK_READ,
    EVENT_GSM_CLOCK_SET,
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
    EVENT_GSM_SMS_SEND,

    /* ADC RELATED EVENTS */
    EVENT_ADC_COVERSION_COMPLETE,

    /* ENERGY MONITOR RELATED EVENTS */
    EVENT_EMON_READ_VOLTAGE,
    EVENT_EMON_READ_ENTITY,
    EVENT_EMON_READ_CURRENT,
    EVENT_EMON_VCC_READ_DONE,
    EVENT_EMON_ZERO_CROSS_FOUND,
    EVENT_EMON_MEASUREMENT_DONE,
    EVENT_EMON_ERROR,
    EVENT_EMON_DUMMY_READ,
    EVENT_EMON_DUMMY_READ_DONE,
    EVENT_EMON_START,

    SERIAL_TRANSMIT_SIG,    // Serial transmit signal
    SERIAL_RECIEVE_SIG,     // Serial recieve signal
    GSM_PROCESS_SIG,        // Start processing the command
    GSM_DONE_SIG,           // GSM Finished processing
    GSM_SUCCESS_EVENT,
    GSM_FAILURE_EVENT,
    MAX_PUB_SIG,            // the last published signal
};
#endif

enum gsm_driver_messages
{
    GSM_MSG_NONE = 1,
    // general
    GSM_MSG_OK,
    //GSM_MSG_AT,
    //GSM_MSG_ATE0,
    GSM_MSG_ERROR,
    GSM_MSG_ERROR_NETWORK_IS_DOWN,
    GSM_MSG_ERROR_OPERATION_FAILURE,
    GSM_MSG_ERROR_SIMCARD_NOT_INSERTED,
    GSM_MSG_ERROR_SIMCARD_PIN_DENNY,
    GSM_MSG_ERROR_SMS_IX_NOT_ALLOWED,
    // pin
    GSM_MSG_PIN_READY,
    GSM_MSG_PIN_SIM,
    GSM_MSG_PIN_SIM_2,
    GSM_MSG_PIN_PUK,
    GSM_MSG_PIN_PUK_2,
    /* CREG -- NETWORK REGISTRATION */
    GSM_MSG_CREG_NEWTWORK_SEARCHING_IN_IDLE,
    GSM_MSG_CREG_NETWORK_READY_LOCAL,
    GSM_MSG_CREG_SEARCHING_NETWORK,
    GSM_MSG_CREG_NETWORK_ACCESS_DENIED,
    GSM_MSG_CREG_NETWORK_READY_ROAMING,
    // SISI : service (socket) status
    GSM_MSG_SOCKET_UP,
    GSM_MSG_SOCKET_DOWN,
    GSM_MSG_NO_SOCKET_OPERATION,
    GSM_MSG_SOCKET_CONNECTING,
    GSM_MSG_SOCKET_CLOSING,
    //
    GSM_CLOCK_READ,
    //GSM_MSG_DATA_STREAM,
    SMS_UNREAD,
    SMS_READ,
    SMS_ALL,
    GSM_MSG_SCID_INFO,
    GSM_MSG_SMS_PROMPT,
    GSM_MSG_SMS_QUEUED,
    GSM_MSG_BAD_SMS_MESSAGE,
    GSM_MSG_SMS_REC_READ,
    GSM_MSG_SMS_REC_UNREAD,
    GSM_MSG_SMS_UNKOWN_MESAGE,
    GSM_MSG_SMS_NO_MESAGE,
    GSM_MSG_SMS_LIST,
    GSM_MSG_SOCKET_NOT_READY,
    GSM_MSG_DATA_TRANSFER_STATUS,
    //GSM_MSG_GPRS_CONNECTED_STATUS_READY,
    GSM_MSG_GPRS_CONNECTED_STATUS_NOT_READY,
    GSM_MSG_GPRS_DEATTACHED,
    GSM_MSG_SIGNAL_LEVEL,
    GSM_MSG_HW_TEMPERATURE,
    GSM_MSG_GPRS_STATUS,
    GSM_MSG_ERROR_GPTS_NOT_ALLOWED,
    GSM_MSG_CONTEXT_STATUS,
    GSM_MSG_APP_PAYLOAD_AVAILABLE,
    //
    // gsm internal state machine
    // power
    GSM_MSG_POWER_DOWN_REQUEST_ACKNOWLOGED,
    GSM_MSG_POWER_DOWN_PROCESSING,
    GSM_MSG_POWER_ON_DONE,
    GSM_MSG_SIMCARD_INITIALIZNG_IN_PROCESS,
    GSM_MSG_SIMCARD_INITIALIZNG_DONE,
    GSM_MSG_NETWORK_CONNECTION_STATUS_READY,
    GSM_MSG_NETWORK_CONNECTION_STATUS_NOT_CONNECTED,
    GSM_MSG_NETWORK_NOT_ATTACHED,
    GSM_MSG_NETWORK_ATTACHED
};



#endif         /* settings.h */