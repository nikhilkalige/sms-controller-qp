/**
 ******************************************************************************
 *
 * @file       fifo_buffer.c
 * @author     Lonewolf
 * @brief      FIFO Ring buffer functions
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#include <string.h>

#include "fifo_buffer.h"


data_type fifoBuf_getSize(t_fifo_buffer *buf)
{
    data_type buf_size = buf->buf_size;
    if(buf_size > 0)
    {
        return buf_size - 1;
    }
    else
        return 0;
}


data_type fifoBuf_getUsed(t_fifo_buffer *buf)
{
    // return the number of bytes available in the rx buffer
    data_type rd = buf->rd;
    data_type wr = buf->wr;
    data_type buf_size = buf->buf_size;

    data_type num_bytes = wr - rd;
    if(wr < rd)
    {
        num_bytes = (buf_size - rd) + wr;
    }

    return num_bytes;
}

data_type fifoBuf_getFree(t_fifo_buffer *buf)
{
    // return the free space size in the buffer
    data_type buf_size = buf->buf_size;

    data_type num_bytes = fifoBuf_getUsed(buf);

    return ((buf_size - num_bytes) - 1);
}

void fifoBuf_clearData(t_fifo_buffer *buf)
{ 
    // remove all data from buffer
    buf->rd = buf->wr;
}

void fifoBuf_removeData(t_fifo_buffer *buf, data_type len)
{
    // remove a number of bytes from the buffer
    data_type rd = buf->rd;
    data_type buf_size = buf->buf_size;

    // get number of bytes available
    data_type num_bytes = fifoBuf_getUsed(buf);

    if(num_bytes > len)
    {
        num_bytes = len;
    }

    if(num_bytes < 1)
    {
        return;             // nothing to remove
    }

    rd += num_bytes;
    if(rd >= buf_size)
    {
        rd -= buf_size;
    }

    buf->rd = rd;
}


int8_t fifoBuf_getBytePeek(t_fifo_buffer *buf)
{
    // get a data byte from the buffer without removing it
    data_type rd = buf->rd;

    // get number of bytes available
    data_type num_bytes = fifoBuf_getUsed(buf);

    if(num_bytes < 1)
    {
        return -1;
    }

    return buf->buf_ptr[rd];
}


int8_t fifoBuf_getByte(t_fifo_buffer *buf)
{
    // get data byte from buffer

    data_type rd = buf->rd;
    data_type buf_size = buf->buf_size;
    uint8_t *buff = buf->buf_ptr;
    // get number of bytes available
    data_type num_bytes = fifoBuf_getUsed(buf);

    if(num_bytes < 1)
    {
        return -1;
    }

    uint8_t b = buff[rd];
    if(++rd >= buf_size)
    {
        rd = 0;
    }
    buf->rd = rd;

    return b;
}


data_type fifoBuf_getDataPeek(t_fifo_buffer *buf, void *data, data_type len)
{       // get data from the buffer without removing it

    data_type rd = buf->rd;
    data_type buf_size = buf->buf_size;
    uint8_t *buff = buf->buf_ptr;

    // get number of bytes available
    data_type num_bytes = fifoBuf_getUsed(buf);

    if (num_bytes > len)
        num_bytes = len;

    if (num_bytes < 1)
        return 0;       // return number of bytes copied

    uint8_t *p = (uint8_t *)data;
    data_type i = 0;

    while (num_bytes > 0)
    {
        data_type j = buf_size - rd;
        if (j > num_bytes)
            j = num_bytes;
        memcpy(p + i, buff + rd, j);
        i += j;
        num_bytes -= j;
        rd += j;
        if (rd >= buf_size)
            rd = 0;
    }

    return i;                   // return number of bytes copied
}

data_type fifoBuf_getData(t_fifo_buffer *buf, void *data, data_type len)
{       // get data from our rx buffer

    data_type rd = buf->rd;
    data_type buf_size = buf->buf_size;
    uint8_t *buff = buf->buf_ptr;

    // get number of bytes available
    data_type num_bytes = fifoBuf_getUsed(buf);

    if (num_bytes > len)
        num_bytes = len;

    if (num_bytes < 1)
        return 0;               // return number of bytes copied

    uint8_t *p = (uint8_t *)data;
    data_type i = 0;

    while (num_bytes > 0)
    {
        data_type j = buf_size - rd;
        if (j > num_bytes)
            j = num_bytes;
        memcpy(p + i, buff + rd, j);
        i += j;
        num_bytes -= j;
        rd += j;
        if (rd >= buf_size)
            rd = 0;
    }

    buf->rd = rd;

    return i;                   // return number of bytes copied
}

data_type fifoBuf_putByte(t_fifo_buffer *buf, const uint8_t b)
{       // add a data byte to the buffer

    data_type wr = buf->wr;
    data_type buf_size = buf->buf_size;
    uint8_t *buff = buf->buf_ptr;

    data_type num_bytes = fifoBuf_getFree(buf);
    if (num_bytes < 1)
        return 0;

    buff[wr] = b;
    if (++wr >= buf_size)
        wr = 0;

    buf->wr = wr;

    return 1;                   // return number of bytes copied
}

data_type fifoBuf_putData(t_fifo_buffer *buf, const void *data, data_type len)
{       // add data to the buffer

    data_type wr = buf->wr;
    data_type buf_size = buf->buf_size;
    uint8_t *buff = buf->buf_ptr;

    data_type num_bytes = fifoBuf_getFree(buf);
    if (num_bytes > len)
        num_bytes = len;

    if (num_bytes < 1)
        return 0;               // return number of bytes copied

    uint8_t *p = (uint8_t *)data;
    data_type i = 0;

    while (num_bytes > 0)
    {
        data_type j = buf_size - wr;
        if (j > num_bytes)
            j = num_bytes;
        memcpy(buff + wr, p + i, j);
        i += j;
        num_bytes -= j;
        wr += j;
        if (wr >= buf_size)
            wr = 0;
    }

    buf->wr = wr;

    return i;                   // return number of bytes copied
}

void fifoBuf_init(t_fifo_buffer *buf, const void *buffer, const data_type buffer_size)
{
    buf->buf_ptr = (uint8_t *)buffer;
    buf->rd = 0;
    buf->wr = 0;
    buf->buf_size = buffer_size;
}


