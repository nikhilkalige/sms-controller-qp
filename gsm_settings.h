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


PROGMEM const char READY [] = "Call Ready";
PROGMEM const char ATE_0[] = "ATE0\r\n";
PROGMEM const char AT_IPR_115200[] = "AT+IPR=115200";
PROGMEM const char AT_IPR_19200[] = "AT+IPR=19200";

PROGMEM const char CREG [] = "AT+CREG?\r\n";
PROGMEM const char _CREG0[]  = "+CREG: 0,0";
PROGMEM const char _CREG1[]  = "+CREG: 0,1";
PROGMEM const char _CREG2[]  = "+CREG: 0,2";
PROGMEM const char _CREG3[]  = "+CREG: 0,3";
PROGMEM const char _CREG5[]  = "+CREG: 0,5";

PROGMEM const char CNMI[] = "AT+CNMI=2,1\r\n";
PROGMEM const char CMGS[] = "AT+CMGS=\"";
PROGMEM const char _CMGS[] = "+CMGS";
PROGMEM const char _CMGS1[]  = "+CMGS:";
PROGMEM const char _CMGR_UN[]  = "+CMGR: \"REC UNREAD\"";
PROGMEM const char _CMGR_READ[]  = "+CMGR: \"REC READ\"";
PROGMEM const char _CMGR1[]  = "+CMGR:";
PROGMEM const char _CMGL1[]  = "+CMGL:";
PROGMEM const char CMGF[] = "AT+CMGF=1\r\n";
PROGMEM const char CPMS[] = "AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n";
PROGMEM const char _CPMS[] = "+CPMS";
PROGMEM const char CPMS_REQ[] = "AT+CPMS?";
PROGMEM const char CMGL_UNREAD[] = "AT+CMGL=\"REC UNREAD\"\r\n";
PROGMEM const char CMGL_READ[] = "AT+CMGL=\"REC READ\"\r\n";
PROGMEM const char CMGL_ALL[] = "AT+CMGL=\"ALL\"\r\n";
PROGMEM const char _CMGL[] = "+CMGL:";
PROGMEM const char CMGR[] = "AT+CMGR=";
PROGMEM const char _CMGR[] = "+CMGR";
PROGMEM const char ERROR[] = "ERROR";
PROGMEM const char UNREAD[] = "\"REC UNREAD\"";
PROGMEM const char READ[] = "\"REC READ\"";
PROGMEM const char CMGD[] = "AT+CMGD=";
PROGMEM const char CMGDA[] = "AT+CMGDA=\"DEL ALL\"\r\n";

PROGMEM const char CCLK[] = "AT+CCLK=";
PROGMEM const char CCLK_REQ[] = "AT+CCLK?\r\n";
PROGMEM const char _CCLK[] = "+CCLK";

PROGMEM const char CGATT[] = "AT+CGATT=1\r\n";
PROGMEM const char _CGATT0[]  = "+CGATT: 0";
PROGMEM const char _CGATT1[]  = "+CGATT: 1";
PROGMEM const char CSTT[] = "AT+CSTT=";
PROGMEM const char _CSTT[] = "AT+CSTT\r\n";

PROGMEM const char CIICR[] = "AT+CIICR\r\n";
PROGMEM const char CIFSR[] = "AT+CIFSR\r\n";
PROGMEM const char CIPSTART[] = "AT+CIPSTART=";
PROGMEM const char CIPSEND[] = "AT+CIPSEND\r\n";
PROGMEM const char CIPSHUT[] = "AT+CIPSHUT\r\n";
PROGMEM const char CIPCLOSE[] = "AT+CIPCLOSE\r\n";
PROGMEM const char _CONN_OK[]  = "CONNECT OK";
PROGMEM const char _SHUT_OK[]  = "SHUT OK";
PROGMEM const char _SEND_OK[]  = "SEND OK";
PROGMEM const char _CLOSE_OK[]  = "CLOSE OK";

PROGMEM const char CIPSTATUS[] = "AT+CIPSTATUS\r\n";
PROGMEM const char _S_INI[]  = "STATE: IP INITIAL";
PROGMEM const char _S_STAR[]  = "STATE: IP START";
PROGMEM const char _S_GPRS[]  = "STATE: IP GPRSACT";
PROGMEM const char _S_STAT[]  = "STATE: IP STATUS";
PROGMEM const char _CO_FAIL[]  = "CONNECT FAIL";
PROGMEM const char _S_PDP[]  = "STATE: PDP DEACT";
PROGMEM const char _S_CONF[]  = "STATE: IP CONFIG";
PROGMEM const char _S_TCLOS[]  = "STATE: TCP CLOSED";

PROGMEM const char _OK[]  = "OK";
PROGMEM const char _CMTI[]  = "+CMTI:";
PROGMEM const char _CMS[]  = "+CMS ERROR:";
PROGMEM const char _CME[]  = "+CME ERROR:";
PROGMEM const char _PR[]  = "> ";





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
    const char *p_string;
    uint8_t event_id;
    uint8_t message_id;
};


struct at_response_code frc_table[] =
{
    _OK, EVENT_GSM_ACK_RESPONSE, GSM_MSG_OK,
    _CMTI, EVENT_GSM_SMS_RECIEVE_URC, 0,
    _CMS, EVENT_GSM_ERROR_RESPONSE, GSM_MSG_ERROR,
    _CME, EVENT_GSM_ERROR_RESPONSE, GSM_MSG_ERROR,
    ERROR, EVENT_GSM_ERROR_RESPONSE, GSM_MSG_ERROR,
    // array terminator !!!
    0, 0, 0
};

struct at_response_code sms_table[] =
{
    _PR, EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_PROMPT,
    _CMGS1, EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_QUEUED,
    _CMGR_UN, EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_REC_UNREAD,
    _CMGR_READ, EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_REC_READ,
    _CMGR1, EVENT_GSM_SMS_RESPONSE,  GSM_MSG_SMS_UNKOWN_MESAGE,
    _CMGL1, EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_LIST,
    // array terminator !!!
    (char *) 0, 0, 0
};

/* To accomodate those which donot have a closing \r\n */
struct at_response_code special_table[] =
{
    _PR, EVENT_GSM_SMS_RESPONSE, GSM_MSG_SMS_PROMPT
};

/* 160  bytes of string */
struct at_response_code gprs_table[] =
{
    _PR, EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_PROMPT,
    _CONN_OK, EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_CONNECT,
    _SHUT_OK, EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_CLOSE,
    _SEND_OK, EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_SEND_ACK,
    _CLOSE_OK, EVENT_GSM_GPRS_RESPONSE, GSM_GPRS_CLOSED,
    _S_INI, EVENT_GSM_GPRS_STATUS_RESPONSE, GSM_GPRS_STATUS_INITIAL,
    _S_STAR, EVENT_GSM_GPRS_STATUS_RESPONSE, GSM_GPRS_STATUS_START,
    _S_GPRS, EVENT_GSM_GPRS_STATUS_RESPONSE, GSM_GPRS_STATUS_GPRSACT,
    _S_STAT, EVENT_GSM_GPRS_STATUS_RESPONSE, GSM_GPRS_STATUS_STATUS,
    _CO_FAIL, EVENT_GSM_GPRS_STATUS_RESPONSE, GSM_GPRS_CONNECT_ERROR,
    _S_PDP, EVENT_GSM_GPRS_STATUS_RESPONSE, GSM_GPRS_CLOSED,
    _S_CONF, EVENT_GSM_GPRS_STATUS_RESPONSE, GSM_GPRS_STATUS_STATUS,
    _S_TCLOS, EVENT_GSM_GPRS_STATUS_RESPONSE, GSM_GPRS_TCP_CLOSED,
    // array terminator !!!
    (char *) 0, 0, 0
};

struct at_response_code acc_table[] =
{
    _CCLK, EVENT_GSM_CLOCK_RESPONSE, GSM_CLOCK_READ,
    // array terminator !!!
    (char *) 0, 0, 0
};

struct at_response_code network_table[] =
{
    _CREG0, EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_CREG_NEWTWORK_SEARCHING_IN_IDLE,
    _CREG1, EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_CREG_NETWORK_READY_LOCAL,
    _CREG2, EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_CREG_SEARCHING_NETWORK,
    _CREG3, EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_CREG_NETWORK_ACCESS_DENIED,
    _CREG5, EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_CREG_NETWORK_READY_ROAMING,
    _CGATT0, EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_NETWORK_NOT_ATTACHED,
    _CGATT1, EVENT_GSM_NETWORK_RESPONSE, GSM_MSG_NETWORK_ATTACHED,
    // array terminator !!!
    (char *) 0, 0, 0
};

const uint8_t *module_init_table[] =
{
    (uint8_t *)ATE_0,
    /* SMS Related */
    (uint8_t *) CNMI,
    (uint8_t *) CMGF,
    (uint8_t *) CPMS,
    (uint8_t *) CMGDA,
    (uint8_t *)""
};


#endif                  /* gsm_settings.h */