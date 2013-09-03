/**
 ******************************************************************************
 *
 * @file       adc.h
 * @author     Lonewolf
 * @brief      ADC Header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#ifndef ADC_H
#define ADC_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <qpn_port.h>

typedef struct adc_tag
{
    QActive *master;
} adc_dev_config;

void adc_enable_interrupt();
void adc_disable_interrupt();
void adc_read(uint8_t pin);
void adc_init(QActive *master);
void adc_reference(uint8_t mode);
void adc_interrupt();
void adc_set_ao(QActive *master);
void adc_enable(void);
void adc_disable(void);
#endif