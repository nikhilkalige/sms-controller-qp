
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

/* Define the Serial BAUD Rate */
#define BAUDRATE 115200

//void QActive_postISR(QActive * const me, QSignal sig, QParam par);
/* Structure for Serial Device */
typedef struct serial_dev_tag
{
#ifdef Q_LEAPS
    QActive *master;
    QActive *AO_Trans;
    QActive *AO_Recv;
#endif
    uint8_t *p_timeout;
    struct
    {
        uint8_t *p_storage;
        uint8_t buffer_size;
        uint8_t write_ix;
    } rx;
    struct
    {
        uint8_t *p_storage;
        uint8_t buffer_size;
        uint8_t read_ix;
        uint8_t payload_size;
    } tx;
} Serial_device;

Serial_device ser_dev;


/* ISR for Serial UART Transmit */
ISR( USART_UDRE_vect)
{
#ifdef QPK
    QK_ISR_ENTRY();
#endif
    if (ser_dev.tx.read_ix < ser_dev.tx.payload_size)
    {
        UDR0 = ser_dev.tx.p_storage[ser_dev.tx.read_ix];
        ser_dev.tx.read_ix++;
    }
    else
    {
        /* No bytes to send, Disable the Tx interrupt */
        ser_dev.tx.read_ix = 0;
        ser_dev.tx.payload_size = 0;
        UCSR0B &= ~_BV(UDRIE0);
        /* Transmission is complete, so raise the event */
#ifdef Q_LEAPS
        QActive_postISR((QActive *)ser_dev.master, EVENT_SERIAL_SEND_DONE, 0);
#endif
        //Serial_device.tx_busy = false;
    }
#ifdef QPK
    QK_ISR_EXIT();
#endif
}

/* ISR for Serial UART Reciever
 * The Reciever will not generate any event by itself, a timer will used to call the comm_handle at regular intervals
 * TODO : Make provisions for overflow and error handling
 */
ISR( USART_RX_vect)
{
#ifdef QPK
    QK_ISR_ENTRY();
#endif
    uint8_t byte;
    *ser_dev.p_timeout = 0;
    byte = UDR0;
    if (ser_dev.rx.write_ix >= ser_dev.rx.buffer_size)
    {
        /* Raise the overflow flag */

    }
    else
    {
        ser_dev.rx.p_storage[ser_dev.rx.write_ix] = byte;
        ser_dev.rx.write_ix++;
    }
#ifdef QPK
    QK_ISR_EXIT();
#endif
}

#if 0
/**
 * Send the buffer over UART
 * return -2 if non-blocking mode activated: buffer is full
 *            caller should retry until buffer is free again
 * return number of bytes transmitted on success
 */
int8_t Serial_SendBufferNonBlocking(const uint8_t *buffer,
                                    uint16_t len)
{
    if (len > fifoBuf_getFree(&tx_buffer))
    {
        /* Buffer cannot accept all requested bytes (retry) */
        return -2;
    }
    cli();
    uint8_t bytes_into_fifo = fifoBuf_putData(&tx_buffer, buffer, len);
    sei();
    if (bytes_into_fifo > 0)
    {
        /* More data has been put in the tx buffer, make sure the tx is started */
        UCSR0B |= _BV(UDRIE0);
    }
    return (bytes_into_fifo);
}

/**
 * Sends a single character over UART
 * return 0 on success
 * return  buffer is full caller should retry untill buffer is free again
 */
int8_t Serial_SendCharNonBlocking(char c)
{
    return Serial_SendBufferNonBlocking((uint8_t *) &c, 1);
}

/**
 * Sends a string over UART
 * return -1 if port not available
 * return -2 buffer is full
 *         caller should retry until buffer is free again
 * return 0 on success
 */
int8_t Serial_SendStringNonBlocking(const char *str)
{
    return Serial_SendBufferNonBlocking((uint8_t *) str,
                                        (uint8_t) strlen(str));
}
#endif

/**
 * Initialize the UART
 */
void Serial_config(void)
{
    UCSR0A = _BV(U2X0); //Double speed mode USART0
    UCSR0B = _BV(TXEN0) | _BV(RXEN0); // Transmitter and Reciever enabled
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); // 8 bit data mode,  0 parity bits
    UCSR0B |= _BV(RXCIE0); // Enable the reciever interrupt
    uint16_t baud = (F_CPU / (8 * BAUDRATE)) - 1;
    UBRR0H = baud >> 8;
    UBRR0L = baud;
}

void Serial_enable_transmitter()
{
    UCSR0B |= _BV(UDRIE0);
}

void Serial_init(uint8_t *storage_rx, uint8_t size_rx, uint8_t *p_timeout_, uint8_t *storage_tx, uint8_t size_tx, uint8_t *payload_tx, QActive *my_ao)
{
    ser_dev.rx.p_storage = storage_rx;
    ser_dev.rx.buffer_size = size_rx;
    ser_dev.tx.p_storage = storage_tx;
    ser_dev.tx.buffer_size = size_tx;
    ser_dev.p_timeout = p_timeout_;
    ser_dev.tx.payload_size = payload_tx;
    ser_dev.master = my_ao;
}

