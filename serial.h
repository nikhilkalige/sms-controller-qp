/**
 ******************************************************************************
 *
 * @file       fifo_buffer.c
 * @author     Lonewolf
 * @brief      UART Header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "settings.h"

int8_t Serial_ChangeBaud(uint8_t baud);
int8_t Serial_SendCharNonBlocking(QActive* AO, char c);
int8_t Serial_SendChar(char c);
int8_t Serial_SendBufferNonBlocking(QActive* AO, const uint8_t *buffer, uint16_t len);
int8_t Serial_SendBuffer(const uint8_t *buffer, uint16_t len);
int8_t Serial_SendStringNonBlocking(QActive* AO, const char *str);
int8_t Serial_SendString(const char *str);
int8_t Serial_SendFormattedStringNonBlocking(QActive* AO, const char *format);
int8_t Serial_SendFormattedString(const char *format);
uint16_t Serial_ReceiveBuffer(uint8_t * buf, uint16_t buf_len, uint8_t timeout_ms);
#ifdef QP_LEAPS
int8_t Serial_RecieveRegister(QActive* AO);
#endif
void Serial_init(void);


#endif					/* serial.h */