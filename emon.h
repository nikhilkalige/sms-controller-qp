/**
 ******************************************************************************
 *
 * @file       emon.h
 * @author     Lonewolf
 * @brief      Energy Monitor Header
 * @see        The GNU Public License (GPL) Version 3
 *
 * Based on code from:
 * Created by Trystan Lea, April 27 2010
 *****************************************************************************/

#ifndef EMON_H
#define EMON_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <qpn_port.h>

#define EMON_TIMEOUT    2 sec
#define EMON_CROSSINGS  20

typedef struct emon_tag
{
    QActive super;
    /* Public Members */
    QActive *master;
    double realPower,
           apparentPower,
           powerFactor,
           Vrms,
           Irms;
    //Calibration coeficients
    //These need to be set in order to obtain accurate results
    double VCAL;
    double ICAL;
    double PHASECAL;
    uint8_t current_activity;

} emon;
emon emon_dev;
void emon_ctor(void);
void emon_config(QActive *master);
#endif