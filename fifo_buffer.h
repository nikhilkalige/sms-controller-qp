/**
 ******************************************************************************
 *
 * @file       fifo_buffer.h
 * @author     Lonewolf
 * @brief      FIFO Ring buffer header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

 #ifndef FIFO_BUFFER_H_
 #define FIFO_BUFFER_H_

#include "settings.h"


#define ARCH 8


#if (ARCH == 8)
 	#define data_type uint8_t
#elif (ARCH == 32)
 	#define data_type uint16_t
#else
 	#error "Please define the Microcontroller Architechture"
#endif



 typedef struct
 {
 	uint8_t *buf_ptr;
 	volatile data_type rd;
 	volatile data_type wr;
 	data_type buf_size;
 }t_fifo_buffer;


data_type fifoBuf_getSize(t_fifo_buffer *buf);

data_type fifoBuf_getUsed(t_fifo_buffer *buf);
data_type fifoBuf_getFree(t_fifo_buffer *buf);

void fifoBuf_clearData(t_fifo_buffer *buf);
void fifoBuf_removeData(t_fifo_buffer *buf, data_type len);

int8_t fifoBuf_getBytePeek(t_fifo_buffer *buf);
int8_t fifoBuf_getByte(t_fifo_buffer *buf);

data_type fifoBuf_getDataPeek(t_fifo_buffer *buf, void *data, data_type len);
data_type fifoBuf_getData(t_fifo_buffer *buf, void *data, data_type len);

data_type fifoBuf_putByte(t_fifo_buffer *buf, const uint8_t b);

data_type fifoBuf_putData(t_fifo_buffer *buf, const void *data, data_type len);
void fifoBuf_init(t_fifo_buffer *buf, const void *buffer, const data_type buffer_size);

#endif							/* fifo_buffer.h */