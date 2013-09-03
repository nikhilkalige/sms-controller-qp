/**
 ******************************************************************************
 *
 * @file       edb.c
 * @author     Lonewolf
 * @brief      Extended Database Library for AVR
 * @see        The GNU Public License (GPL) Version 3
 *
 * Based on code from:
 * http://www.arduino.cc/playground/Code/ExtendedDatabaseLibrary
 *
 * Based on code from:
 * Database library for Arduino
 * Written by Madhusudana das
 * http://www.arduino.cc/playground/Code/DatabaseLibrary
 *
 *****************************************************************************/

#include "settings.h"
#include "softserial.h"
#include "edb.h"
#include <avr/eeprom.h>


uint16_t edb_head_ptr;
uint16_t edb_table_ptr;
edb_header edb_head;

static void edb_write(uint16_t ee, const uint8_t *p, uint16_t);
static void edb_read(uint16_t ee, uint8_t *p, uint16_t);
static void write_head();
static void read_head();
static edb_status writerec(uint16_t, const edb_rec);
static uint8_t read(uint16_t address);
static void write(uint16_t address, uint8_t value);

edb_write_handler *_write_uint8_t = &write;
edb_read_handler *_read_uint8_t = &read;

//_write_uint8_t = &write;
//_read_uint8_t = &read;

/************************************************************************************************************************************
                                                    ***** Public Funtions *****
************************************************************************************************************************************/


void edb_init(edb_write_handler *w, edb_read_handler *r)
{
    _write_uint8_t = &write;
    _read_uint8_t = &read;
}

/* creates a new table and sets header values */
edb_status edb_create(uint16_t head_ptr, uint16_t tablesize, uint16_t recsize)
{
    edb_head_ptr = head_ptr;
    edb_table_ptr = sizeof(edb_header) + edb_head_ptr;
    edb_head.n_recs = 0;
    edb_head.rec_size = recsize;
    edb_head.table_size = tablesize;
    write_head();
    return EDB_OK;
}


/* reads an existing edb header at a given recno and sets header values */
edb_status edb_open(uint16_t head_ptr)
{
    edb_head_ptr = head_ptr;
    edb_table_ptr = sizeof(edb_header) + edb_head_ptr;
    read_head();
    return EDB_OK;
}

/* reads a record from a given recno */
edb_status edb_readrec(uint16_t recno, edb_rec rec)
{
    if (recno < 1 || recno > edb_head.n_recs)
    {
        return EDB_OUT_OF_RANGE;
    }
    edb_read(edb_table_ptr + ((recno - 1) * edb_head.rec_size), rec, edb_head.rec_size);
    return EDB_OK;
}


/* Deletes a record at a given recno
 Becomes more inefficient as you the record set increases and you delete records
 early in the record queue. */
edb_status edb_deleterec(uint16_t recno)
{
    if ((!recno) || (recno > edb_head.n_recs))
    {
        return  EDB_OUT_OF_RANGE;
    }
    edb_rec rec = (uint8_t *)malloc(edb_head.rec_size);
    for (uint16_t i = recno + 1; i <= edb_head.n_recs; i++)
    {
        edb_readrec(i, rec);
        writerec(i - 1, rec);
    }
    free(rec);
    edb_head.n_recs--;
    write_head();
    return EDB_OK;
}

/* Inserts a record at a given recno, increasing all following records' recno by 1.
 This function becomes increasingly inefficient as it's currently implemented and
 is the slowest way to add a record. */
edb_status edb_insertrec(uint16_t recno, edb_rec rec)
{
    if (edb_count() == edb_limit())
    {
        return EDB_TABLE_FULL;
    }
    if (edb_count() > 0 && ((!recno) || recno > edb_head.n_recs))
    {
        return EDB_OUT_OF_RANGE;
    }
    if (edb_count() == 0 && recno == 1)
    {
        return edb_appendrec(rec);
    }
    edb_rec buf = (uint8_t *)malloc(edb_head.rec_size);
    for (uint16_t i = edb_head.n_recs; i >= recno; i--)
    {
        edb_readrec(i, buf);
        writerec(i + 1, buf);
    }
    free(buf);
    writerec(recno, rec);
    edb_head.n_recs++;
    write_head();
    return EDB_OK;
}

/* Updates a record at a given recno */
edb_status edb_updaterec(uint16_t recno, edb_rec rec)
{
    if ((!recno) || recno > edb_head.n_recs)
    {
        return EDB_OUT_OF_RANGE;
    }
    writerec(recno, rec);
    return EDB_OK;
}

/* Adds a record to the end of the record set
 This is the fastest way to add a record */
edb_status edb_appendrec(edb_rec rec)
{
    if (edb_head.n_recs + 1 > edb_limit())
    {
        return EDB_TABLE_FULL;
    }
    edb_head.n_recs++;
    writerec(edb_head.n_recs, rec);
    write_head();
    return EDB_OK;
}

/* returns the number of queued items */
uint16_t edb_count()
{
    return edb_head.n_recs;
}

/* returns the maximum number of items that will fit into the queue */
uint16_t edb_limit()
{
    return (edb_head.table_size - edb_table_ptr) / edb_head.rec_size;
}

/* truncates the queue by resetting the internal pointers */
void edb_clear()
{
    read_head();
    edb_create(edb_head_ptr, edb_head.table_size, edb_head.rec_size);
}

/************************************************************************************************************************************
                                                    ***** Private Funtions *****
************************************************************************************************************************************/
uint8_t read(uint16_t address)
{
    return eeprom_read_byte((unsigned char *) address);
}

void write(uint16_t address, uint8_t value)
{
    eeprom_write_byte((unsigned char *) address, value);
}

/* low level uint8_t write */
void edb_write(uint16_t ee, const uint8_t *p, uint16_t recsize)
{
    for (uint16_t i = 0; i < recsize; i++)
        _write_uint8_t(ee++, *p++);
}

/* low level uint8_t read */
void edb_read(uint16_t ee, uint8_t *p, uint16_t recsize)
{
    for (uint16_t i = 0; i < recsize; i++)
        *p++ = _read_uint8_t(ee++);
}

/* writes EDB_Header */
void write_head()
{
    edb_write(edb_head_ptr, EDB_REC edb_head, (uint16_t)sizeof(edb_header));
}

/* reads EDB_Header */
void read_head()
{
    edb_read(edb_head_ptr, EDB_REC edb_head, (uint16_t)sizeof(edb_header));
}

/* writes a record to a given recno */
edb_status writerec(uint16_t recno, const edb_rec rec)
{
    edb_write(edb_table_ptr + ((recno - 1) * edb_head.rec_size), rec, edb_head.rec_size);
    return EDB_OK;
}