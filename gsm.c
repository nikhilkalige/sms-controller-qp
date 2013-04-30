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

enum GSMTimeouts
{
	/* Various timeouts in ticks */
	START_TINY_COMM_TMOUT = BSP_TICKS_PER_SEC * 2, // 20ms
	START_SHORT_COMM_TMOUT = BSP_TICKS_PER_SEC * 50, // 500ms
	START_LONG_COMM_TMOUT = BSP_TICKS_PER_SEC * 100, // 1s
	START_XLONG_COMM_TMOUT = BSP_TICKS_PER_SEC * 500, // 5s
	START_XXLONG_COMM_TMOUT = BSP_TICKS_PER_SEC * 700, //7s
	MAX_INTERCHAR_TMOUT = BSP_TICKS_PER_SEC * 2, // 20ms
	MAX_MID_INTERCHAR_TMOUT = BSP_TICKS_PER_SEC / 10, // 100ms
	MAX_LONG_INTERCHAR_TMOUT = BSP_TICKS_PER_SEC * 150, // 1.5s
};


#define RX_NOT_STARTED      0
#define RX_ALREADY_STARTED  1

uint8_t rx_state;

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

extern uint8_t commandFlag;
uint8_t gsmBuffer[100];

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
			/* Process the cmd byte, fill the buffer and transit to state: transmit */
			if(commandFlag == 1)
			{
				/* Send AT */
				strcpy(gsmBuffer, "AT\r\n");
			}
			status = Q_TRAN(&GSM_transmit);
			//PORTD ^= (1 << 2);
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
			/* Send the data recieved from the input state */
			uint8_t len = strlen(gsmBuffer);
			Serial_SendBufferNonBlocking(&AO_GSM, gsmBuffer, len);
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
			QActive_arm((QActive *)me, START_LONG_COMM_TMOUT);
			rx_state = RX_NOT_STARTED;
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
			PORTD ^= (1 << 3);
			rx_state = RX_ALREADY_STARTED;
			QActive_arm((QActive *)me, MAX_MID_INTERCHAR_TMOUT);
			status = Q_HANDLED();
			break;
		}

		case Q_TIMEOUT_SIG:
		{
			if(rx_state == RX_NOT_STARTED)
			{
				/* Start of communication timeout */
			}
			else
			{
				/* Intercharacter timeout */
			}
			status = Q_TRAN(&GSM_process);
			PORTD ^= (1 << 4);
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
	switch(Q_SIG(me))
	{
		case Q_ENTRY_SIG:
		{
			Serial_SendStringNonBlocking((QActive *)&AO_GSM, "Done baby \r\n");
			status = Q_HANDLED();
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