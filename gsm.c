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

#define RX_NOT_STARTED      0
#define RX_ALREADY_STARTED  1


typedef struct GSMTag
{
	QActive super;
}GSM;


static QState GSM_initial(GSM * const me);
static QState GSM_input(GSM * const me);
static QState GSM_transmit(GSM * const me);
static QState GSM_recieve(GSM * const me);
static QState GSM_process(GSM * const me);


GSM AO_GSM;


/* Functions related to GSM */
static QState process_command();
static void trasmit_command();
static void process_data(void *next_state);
static uint8_t compare_string(char const *buffer, char const *string);
static void set_timeouts(GSM_timeouts start_comm_tmout, GSM_timeouts max_interchar_tmout);
void init_statemachine();
//extern uint8_t commandFlag;
//static uint8_t gsmBuffer[GSM_BUFFER_SIZE];

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
	switch(GSM_buffer.cmd_byte)
	{
		case SEND_SMS:
		{
			/* "Phone No""Data to be sent" */
			i = strlen((char*) data);
			strcpy((char*) GSM_buffer.sms.phone_no, (char*) data);
			strcpy((char*) GSM_buffer.buffer, &data[i]);
			break;
		}

		case SET_TIME:
		{
			/* Time is set as string in appropriate form required by GSM Module */
			strcpy((char*) GSM_buffer.buffer, (char*) data);
			break;
		}

		case GET_TIME:
		{
			/* Time is set as string in appropriate form required by GSM Module */
			strcpy((char*) data, (char*) GSM_buffer.buffer);
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
	switch(GSM_buffer.cmd_byte)
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
	switch(GSM_buffer.state)
	{
		case SMS_CMD:
		{
			set_timeouts(START_LONG_COMM_TMOUT, MAX_INTERCHAR_TMOUT);
			Serial_SendStringNonBlocking(&AO_GSM, "AT+CMGS=\"");
			Serial_SendStringNonBlocking(&AO_GSM, GSM_buffer.sms.phone_no);
			Serial_SendStringNonBlocking(&AO_GSM,"\"\r");
			break;
		}

		case SMS_DATA:
		{
			set_timeouts(START_XXLONG_COMM_TMOUT, MAX_INTERCHAR_TMOUT);
			tmp = 0x1A;
			Serial_SendStringNonBlocking(&AO_GSM, (char*) GSM_buffer.buffer);
			Serial_SendBufferNonBlocking(&AO_GSM, (char*) &tmp, 1);
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
	switch(GSM_buffer.state)
	{
		case SMS_CMD: 
		{
			if(compare_string((char*)tmp, ">")) 
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
			if(compare_string((char*)tmp, "+CMGS")) 
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

	if(status == 1)
	{
		QActive_post(GSM_buffer.active_object, GSM_SUCCESS_EVENT, 0);
		next_state = &GSM_input;
	}
	else if(!status)
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
	if(ch != NULL) {
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


static QState GSM_initial(GSM * const me)
{
	//PORTD ^= (1 << 2);
	return Q_TRAN(&GSM_input);
}


static QState GSM_input(GSM * const me)
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


static QState GSM_transmit(GSM * const me)
{
	QState status;
	switch(Q_SIG(me))
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


static QState GSM_recieve(GSM * const me)
{
	QState status;
	switch(Q_SIG(me))
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


static QState GSM_process(GSM * const me)
{
	QState status;
	void *next_state;
	switch(Q_SIG(me))
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