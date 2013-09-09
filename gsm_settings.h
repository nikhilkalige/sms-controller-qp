/**
 ******************************************************************************
 *
 * @file       gsm_settings.h
 * @author     Lonewolf
 * @brief      GSM Settings and Strings
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
#ifndef GSM_SETTINGS_H_
#define GSM_SETTINGS_H_

#include "settings.h"


/* Final Result Codes and Unsolicited Responses */
const uint8_t frc1[] PROGMEM = "+CME ERROR:";
const uint8_t frc2[] PROGMEM = "+CMS ERROR:";
const uint8_t frc3[] PROGMEM = "OK";
const uint8_t frc4[] PROGMEM = "ERROR";

/* Hardware Configuration */
#define GSM_PWR_DDR     DDRB
#define GSM_BAUD        115200
#define GSM_PWR_PORT    PORTB
#define GSM_PWRKEY      (1 << 1)

enum frc_codes
{
    EMPTY = 0,
    CME,
    CMS,
    OK
};

enum gsm_operations
{
    GSM_OP_NONE = 1,
    GSM_OP_SMS,
    GSM_OP_NETWORK,
    GSM_OP_ACC,
    GSM_OP_GPRS
};
#if 0
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
#endif
typedef struct final_result_codes_t
{
    uint8_t *p_string;
    uint8_t  event_id;
} final_result_codes;

struct at_response_code
{
    uint8_t *p_string;
    uint8_t event_id;
    uint8_t message_id;
};


struct at_response_code frc_table[] =
{
    (uint8_t *)"OK", EVENT_GSM_ACK_RESPONSE, GSM_MSG_OK,
    (uint8_t *)"+CMTI:", EVENT_GSM_SMS_RECIEVE_URC, 0,
    (uint8_t *)"+CMS ERROR:", EVENT_GSM_ERROR_RESPONSE, GSM_MSG_ERROR,
    (uint8_t *)"+CME ERROR:", EVENT_GSM_ERROR_RESPONSE, GSM_MSG_ERROR,
    (uint8_t *)"ERROR", EVENT_GSM_ERROR_RESPONSE, GSM_MSG_ERROR,
    // array terminator !!!
    (uint8_t *) 0, 0, 0
};

struct at_response_code sms_table[] =
{
    (uint8_t *)"> ", EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_PROMPT,
    (uint8_t *)"+CMGS:", EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_QUEUED,
    (uint8_t *)"+CMGR: \"REC UNREAD\"", EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_REC_UNREAD,
    (uint8_t *)"+CMGR: \"REC READ\"", EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_REC_READ,
    (uint8_t *)"+CMGR:", EVENT_GSM_SMS_RESPONSE,  GSM_MSG_SMS_UNKOWN_MESAGE,
    (uint8_t *)"+CMGL:", EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_LIST,
    // array terminator !!!
    (uint8_t *) 0, 0, 0
};

/* To accomodate those which donot have a closing \r\n */
struct at_response_code special_table[] =
{
    (uint8_t *)"> ", EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_PROMPT
};

struct at_response_code gprs_table[] =
{
    (uint8_t *)"> ", EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_PROMPT,
    (uint8_t *) "CONNECT OK", EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_CONNECT,
    (uint8_t *) "SHUT OK", EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_CLOSE,
    (uint8_t *) "SEND OK", EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_SEND_ACK,
    (uint8_t *) "STATE: IP INITIAL", EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_STATUS_INITIAL,
    (uint8_t *) "STATE: IP START", EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_STATUS_START,
    (uint8_t *) "STATE: IP GPRSACT", EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_STATUS_GPRSACT,
    (uint8_t *) "STATE: IP STATUS", EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_STATUS_STATUS,
    (uint8_t *) "CONNECT FAIL", EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_CONNECT_ERROR,
    (uint8_t *) "+PDP: DEACT", EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_CLOSED,
    // array terminator !!!
    (uint8_t *) 0, 0, 0
};

struct at_response_code acc_table[] =
{
    (uint8_t *)"+CCLK:", EVENT_GSM_CLOCK_RESPONSE, GSM_CLOCK_READ,
    // array terminator !!!
    (uint8_t *) 0, 0, 0
};

struct at_response_code network_table[] =
{
    (uint8_t *)"+CREG: 0,0", EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_CREG_NEWTWORK_SEARCHING_IN_IDLE,
    (uint8_t *)"+CREG: 0,1", EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_CREG_NETWORK_READY_LOCAL,
    (uint8_t *)"+CREG: 0,2", EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_CREG_SEARCHING_NETWORK,
    (uint8_t *)"+CREG: 0,3", EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_CREG_NETWORK_ACCESS_DENIED,
    (uint8_t *)"+CREG: 0,5", EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_CREG_NETWORK_READY_ROAMING,
    (uint8_t *)"+CGATT: 0", EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_NETWORK_NOT_ATTACHED,
    (uint8_t *)"+CGATT: 1", EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_NETWORK_ATTACHED,
    // array terminator !!!
    (uint8_t *) 0, 0, 0
};

const uint8_t *module_init_table[] =
{
    (uint8_t *)"ATE0\r\n",
    /* SMS Related */
    (uint8_t *)"AT+CNMI=2,1\r\n",
    (uint8_t *)"AT+CMGF=1\r\n",
    (uint8_t *)"AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n",
    (uint8_t *)"AT+CMGDA=\"DEL ALL\"\r\n",
    (uint8_t *)""
};

PROGMEM const char  READY [] = "Call Ready";
PROGMEM const char   ATE_0[] = "ATE0";
PROGMEM const char   AT_IPR_115200[] = "AT+IPR=115200";
PROGMEM const char   AT_IPR_19200[] = "AT+IPR=19200";
PROGMEM const char   CREG [] = "AT+CREG?\r\n";
PROGMEM const char   CREG_1 [] = "+CREG: 0,1";
PROGMEM const char   CREG_2 [] = "+CREG: 0,5";
PROGMEM const char   CMGS [] = "AT+CMGS=\"";
PROGMEM const char   _CMGS [] = "+CMGS";
PROGMEM const char   CNMI [] = "AT+CNMI=2,1";
PROGMEM const char   CMGF [] = "AT+CMGF=1";
PROGMEM const char   CPMS [] = "AT+CPMS=\"SM\",\"SM\",\"SM\"";
PROGMEM const char   _CPMS [] = "+CPMS";
PROGMEM const char   CPMS_REQ[] = "AT+CPMS?";
PROGMEM const char   CMGL_UNREAD [] = "AT+CMGL=\"REC UNREAD\"\r\n";
PROGMEM const char   CMGL_READ [] = "AT+CMGL=\"REC READ\"\r\n";
PROGMEM const char   CMGL_ALL [] = "AT+CMGL=\"ALL\"\r\n";
PROGMEM const char   _CMGL [] = "+CMGL:";
PROGMEM const char   CMGR [] = "AT+CMGR=";
PROGMEM const char   _CMGR [] = "+CMGR";
PROGMEM const char   ERROR [] = "ERROR";
PROGMEM const char   UNREAD [] = "\"REC UNREAD\"";
PROGMEM const char   READ [] = "\"REC READ\"";
PROGMEM const char   CMGD [] = "AT+CMGD=";
PROGMEM const char   CCLK[] = "AT+CCLK=";
PROGMEM const char   CCLK_REQ [] = "AT+CCLK?\r\n";
PROGMEM const char  _CCLK [] = "+CCLK";
PROGMEM const char  CMGDA[] = "AT+CMGDA=\"DEL ALL\"\r\n";
PROGMEM const char  CGATT[] = "AT+CGATT=1\r\n";
PROGMEM const char  CSTT[] = "AT+CSTT=";
PROGMEM const char  _CSTT[] = "AT+CSTT\r\n";
PROGMEM const char  CIICR[] = "AT+CIICR\r\n";
PROGMEM const char  CIPSTART[] = "AT+CIPSTART=";
PROGMEM const char  CIPSEND[] = "AT+CIPSEND\r\n";
PROGMEM const char  CIPSHUT[] = "AT+CIPSHUT\r\n";
PROGMEM const char  CIPSTATUS[] = "AT+CIPSTATUS\r\n";
PROGMEM const char  CIFSR[] = "AT+CIFSR\r\n";

#endif                  /* gsm_settings.h */