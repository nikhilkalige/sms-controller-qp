/**
 ******************************************************************************
 *
 * @file       adc.c
 * @author     Lonewolf
 * @brief      ADC Driver
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
 
#include "adc.h"
#include "settings.h"
#include "softserial.h"

adc_dev_config adc_dev;

void adc_enable_interrupt()
{
    ADCSRA |=  _BV(ADIE);
}

void adc_disable_interrupt()
{
    ADCSRA &= ~(_BV(ADIE));
}

void adc_read(uint8_t pin)
{
    ADMUX = (ADMUX & 0xF0);
    ADMUX |= pin;
    ADCSRA |= _BV(ADSC);
}

void adc_init(QActive *master)
{
    if (master)
    {
        adc_set_ao(master);
    }
    adc_reference(1);
    /* Set the ADC freq to less than 200KHz */
    //#if (F_CPU == 16000000)
    ADCSRA |= 0x07;
    //#else
    //  #error "hiii"
    //ADCSRA |= 0x06;
    //#endif
}

void adc_enable()
{
    ADCSRA |= _BV(ADEN);
}

void adc_disable(void)
{
    ADCSRA &= ~(_BV(ADEN));
}

void adc_reference(uint8_t mode)
{
    ADMUX |= (mode << 6);
}

ISR(ADC_vect)
{
    uint8_t low, high;
    low = ADCL;
    high = ADCH;
    QActive_post((QActive *)adc_dev.master, EVENT_ADC_COVERSION_COMPLETE, (high << 8) | low);
}

void adc_set_ao(QActive *master)
{
    adc_dev.master = master;
}