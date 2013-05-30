/**
 ******************************************************************************
 *
 * @file       serial.c
 * @author     Lonewolf
 * @brief      UART functions
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

/**
* TODO : 
	- The idea is to send an event to the main program loop on completeion of transmitt or recieve.
	- In case of transmit, on completion of the number of bytes added to the loop, an event will be raised.
	- For receieve, one can specify the number of bytes recieved after which the event will be raised.
	- Inorder for this to happen, one must know which active object will recieve the event, so inorder for this to happen we will maintain a 
	  structure to keep track of the AO's.
	- The issue here is that once data has been added to the buffer, some other system should not add data to the same loop as this would make the 
	  AO wait for a longer time to recieve the event. So there should be some form of mechanism to lock the buffer for others, until the current 
	  action is complete.
*/



 #include "serial.h"
 #include "fifo_buffer.h"

// Define the buffer size to be used
 #define SERIAL_BUFFER_SIZE 64

/* Define the Serial BAUD Rate */
 #define BAUDRATE 115200

//void QActive_postISR(QActive * const me, QSignal sig, QParam par);
/* Structure for Serial Device */
struct serial_dev
{
#ifdef Q_LEAPS
	QActive* AO_Trans;
	QActive* AO_Recv;
#endif
	bool tx_busy;
	bool rx_busy;
	/* the no of recieved bytes after which the event is generated */
	uint8_t rx_event_length;
};


struct serial_dev Serial_device;


// Define space for ring buffer for transmit and recieve isr routines
uint8_t rx_storage[SERIAL_BUFFER_SIZE];
uint8_t tx_storage[SERIAL_BUFFER_SIZE];

/* Define the Ring Buffers */
t_fifo_buffer rx_buffer;
t_fifo_buffer tx_buffer;


/* ISR for Serial UART Transmit */
 ISR(USART_UDRE_vect)
 {
 #ifdef Q_LEAPS
 	QK_ISR_ENTRY();
 #endif
 	uint8_t byte;
 	uint8_t bytes_to_send = fifoBuf_getData(&tx_buffer, &byte, 1);
 	if(bytes_to_send > 0)
 	{
 		/* Send the byte recieved */
 		UDR0 = byte;
 	}
 	else
 	{
 		/* No bytes to send, Disable the Tx interrupt */
 		UCSR0B &= ~_BV(UDRIE0);
 		/* Transmission is complete, so raise the event */
 #ifdef Q_LEAPS
 		QActive_postISR((QActive *)Serial_device.AO_Trans, SERIAL_TRANSMIT_SIG, 0);
 		Serial_device.AO_Trans = NULL;
 #endif
 		Serial_device.tx_busy = false;
 	}
 #ifdef Q_LEAPS
 	QK_ISR_EXIT();
 #endif 	
 }


 /* ISR for Serial UART Reciever */
 ISR(USART_RX_vect)
 {
 #ifdef Q_LEAPS
 	QK_ISR_ENTRY();
 #endif
 	uint8_t byte;
 	byte = UDR0;
 	fifoBuf_putByte(&rx_buffer, byte);
 	/* Generate a event to the AO which has subscribed to this */
 	QActive_postISR((QActive *) Serial_device.AO_Recv, SERIAL_RECIEVE_SIG, 0);
 #ifdef Q_LEAPS
 	QK_ISR_EXIT();
 #endif 
 }


/**
* Send the buffer over UART
* return -2 if non-blocking mode activated: buffer is full
*            caller should retry until buffer is free again
* return number of bytes transmitted on success
*/
int8_t Serial_SendBufferNonBlocking(QActive* AO, const uint8_t *buffer, uint16_t len)
{
	/* Register the AO */
	Serial_device.AO_Trans = AO;
	Serial_device.AO_Recv = AO;

	if (len > fifoBuf_getFree(&tx_buffer)) {
		/* Buffer cannot accept all requested bytes (retry) */
		return -2;
	}
	cli();
	uint8_t bytes_into_fifo = fifoBuf_putData(&tx_buffer, buffer, len);
	sei();
	if (bytes_into_fifo > 0) {
		/* More data has been put in the tx buffer, make sure the tx is started */
		UCSR0B |= _BV(UDRIE0);	
	}
	Serial_device.tx_busy = true;
	return (bytes_into_fifo);
}


/**
* Sends a single character over UART
* return 0 on success
* return  buffer is full caller should retry untill buffer is free again
*/
int8_t Serial_SendCharNonBlocking(QActive* AO, char c)
{
	return Serial_SendBufferNonBlocking(AO, (uint8_t *)&c, 1);
}

/**
* Sends a string over UART
* return -1 if port not available
* return -2 buffer is full
*         caller should retry until buffer is free again
* return 0 on success
*/
int8_t Serial_SendStringNonBlocking(QActive* AO, const char *str)
{
	return Serial_SendBufferNonBlocking(AO, (uint8_t *)str, (uint8_t)strlen(str));
}


/**
* Initailize the recieve routing
* return -1 if port is not available
* return 0 on success
*/
#ifdef QP_LEAPS
int8_t Serial_RecieveRegister(QActive* AO)
{
	Serial_device.AO_Recv = AO;
	return 0;
}
#endif

/**
* Initialize the UART
*/
void Serial_init(void)
{
	UCSR0A = _BV(U2X0); //Double speed mode USART0
	UCSR0B = _BV(TXEN0) | _BV(RXEN0); // Transmitter and Reciever enabled
	UCSR0B |= _BV(RXCIE0); // Enable the reciever interrupt
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); // 8 bit data mode,  0 parity bits
	uint16_t baud = (F_CPU/(8 * BAUDRATE)) - 1;
	UBRR0H = baud >> 8;
	UBRR0L = baud;

	Serial_device.rx_busy = false;
	Serial_device.tx_busy = false;
	Serial_device.rx_event_length = 0;
#ifdef Q_LEAPS
	Serial_device.AO_Trans = NULL;
	Serial_device.AO_Recv = NULL;
#endif
	/* Initialze the ring buffer for serial communication */
	fifoBuf_init(&rx_buffer, rx_storage, SERIAL_BUFFER_SIZE);
	fifoBuf_init(&tx_buffer, tx_storage, SERIAL_BUFFER_SIZE);
}
	  
