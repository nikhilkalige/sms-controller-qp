/**
 ******************************************************************************
 *
 * @file       edb.h
 * @author     Lonewolf
 * @brief      Extended Database Library for AVR Header
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

#ifndef EDB_H
#define EDB_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct edb_header_tag
{
    uint16_t n_recs;
    uint16_t rec_size;
    uint16_t table_size;
} edb_header;

typedef enum edb_status_tag
{
    EDB_OK,
    EDB_OUT_OF_RANGE,
    EDB_TABLE_FULL
} edb_status;

typedef uint8_t *edb_rec;

#define EDB_REC (uint8_t*)(void*)&

typedef void edb_write_handler(uint16_t, const uint8_t);
typedef uint8_t edb_read_handler(uint16_t);
void edb_init(edb_write_handler *w, edb_read_handler *r);
edb_status edb_create(uint16_t, uint16_t, uint16_t);
edb_status edb_open(uint16_t);
edb_status edb_readrec(uint16_t, edb_rec);
edb_status edb_deleterec(uint16_t);
edb_status edb_insertrec(uint16_t, const edb_rec);
edb_status edb_updaterec(uint16_t, const edb_rec);
edb_status edb_appendrec(edb_rec rec);
uint16_t edb_limit();
uint16_t edb_count();
void edb_clear();

//extern EDB edb;

#endif