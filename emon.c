/**
 ******************************************************************************
 *
 * @file         emon.c
 * @author    Lonewolf
 * @brief      Energy Monitor Library
 * @see        The GNU Public License (GPL) Version 3
 *
 * Based on code from:
 * Created by Trystan Lea, April 27 2010
 *****************************************************************************/

#include "math.h"
#include "emon.h"
#include "adc.h"
#include "settings.h"
#include "softserial.h"

typedef struct emon_temp_tag
{
    int lastSampleV, sampleV;  //sample_ holds the raw analog read value, lastSample_ holds the last sample
    //int lastSampleI, sampleI;
    uint16_t numberOfSamples;
    double lastFilteredV, filteredV;                  //Filtered_ is the raw analog value minus the DC offset
    //double lastFilteredI, filteredI;
    double phaseShiftedV;                             //Holds the calibrated phase shifted voltage.
    double sqV, sumV;
    //double sqI, sumI, instP, sumP;         //sq = squared, sum = Sum, inst = instantaneous
    int startV;                                       //Instantaneous voltage at start of sample window.
    uint8_t lastVCross, checkVCross;                  //Used to measure number of times threshold is crossed.
    int crossCount;
    uint16_t vcc;
    uint8_t pin;
} emon_temp;

emon_temp emon_vars;

static QState init(emon *const me);
static QState idle(emon *const me);
static QState read_entity(emon *const me);
static QState read_vcc(emon *const me);
static QState zero_crossing(emon *const me);
//static QState active(App *const me);

/************************************************************************************************************************************
                                                    ***** Public Funtions *****
************************************************************************************************************************************/
void emon_ctor(void)
{
    QActive_ctor(&emon_dev.super, Q_STATE_CAST(&init));
}

void emon_config(QActive *master)
{
    emon_dev.master = master;
    adc_init((QActive *)&emon_dev);
}

/************************************************************************************************************************************
                                                    ***** Private Funtions *****
************************************************************************************************************************************/

/************************************************************************************************************************************
                                                    ***** State Machines *****
************************************************************************************************************************************/
static QState init(emon *const me)
{
    return Q_TRAN(&idle);
}

static QState idle(emon *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
            return Q_HANDLED();
        case EVENT_EMON_READ_ENTITY:
            emon_vars.pin = (uint8_t)Q_PAR(me);
            return Q_TRAN(&read_entity);
        case EVENT_EMON_READ_CURRENT:
            return Q_HANDLED();
    }
    return Q_SUPER(&QHsm_top);
}

static QState read_entity(emon *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            emon_vars.numberOfSamples = 0;
            emon_vars.crossCount = 0;
            adc_enable();
            adc_enable_interrupt();
            QActive_post((QActive *)me, EVENT_EMON_START, 0);
            return Q_HANDLED();
        }
#if 0
        case Q_INIT_SIG:
        {
            Softserial_println("tran for vcc");
            return Q_TRAN(&read_vcc);
        }
#endif
        case EVENT_EMON_START:
        {
            return Q_TRAN(&read_vcc);
        }
        case EVENT_EMON_VCC_READ_DONE:
        {
            return Q_TRAN(&zero_crossing);
        }
        case EVENT_EMON_ZERO_CROSS_FOUND:
        {
            QActive_arm((QActive *)me, EMON_TIMEOUT);
            adc_read(emon_vars.pin); //Make changes
            return Q_HANDLED();
        }
        case EVENT_ADC_COVERSION_COMPLETE:
        {
            emon_vars.numberOfSamples++;
            /* Used for digital high pass filter */
            emon_vars.lastSampleV = emon_vars.sampleV;
            /* Used for offset removal */
            emon_vars.lastFilteredV = emon_vars.filteredV;
            /* Read Raw ADC Sample */
            emon_vars.sampleV = (uint16_t)Q_PAR(me);
#if 0
            Softserial_print_byte(emon_vars.numberOfSamples);
            Softserial_print("--");
            Softserial_print_byte(emon_vars.sampleV);
            Softserial_println("");
#endif
            /* Apply digital high pass filters to remove 2.5V DC offset (centered on 0V) */
            emon_vars.filteredV = 0.996 * (emon_vars.lastFilteredV + (emon_vars.sampleV - emon_vars.lastSampleV));
            /* Root-mean-square method voltage */
            emon_vars.sqV = emon_vars.filteredV * emon_vars.filteredV;
            emon_vars.sumV += emon_vars.sqV;
            /*  Find the number of times the voltage has crossed the initial voltage
                - every 2 crosses we will have sampled 1 wavelength
                - so this method allows us to sample an integer number of half wavelengths which increases accuracy */
            emon_vars.lastVCross = emon_vars.checkVCross;
            if (emon_vars.sampleV > emon_vars.startV)
            {
                emon_vars.checkVCross = true;
            }
            else
            {
                emon_vars.checkVCross = false;
            }
            if (emon_vars.numberOfSamples == 1)
            {
                emon_vars.lastVCross = emon_vars.checkVCross;
            }
            if (emon_vars.lastVCross != emon_vars.checkVCross)
            {
                emon_vars.crossCount++;
            }
            adc_read(emon_vars.pin);
            if (emon_vars.crossCount >= EMON_CROSSINGS)
            {
                double V_RATIO = me->VCAL * ((emon_vars.vcc / 1000.0) / 1023.0);
                me->Vrms = V_RATIO * sqrt(emon_vars.sumV / emon_vars.numberOfSamples);
#ifdef SERIAL_DEBUG
                Softserial_print("Vrms = ");
                Softserial_print_byte(me->Vrms);
                Softserial_println("");
#endif
                QActive_post((QActive *)me->master, EVENT_EMON_MEASUREMENT_DONE, me->Vrms);
                return Q_TRAN(&idle);
            }
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            emon_vars.crossCount = EMON_CROSSINGS;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QActive_disarm((QActive *)me);
            /* Reset accumulators */
            emon_vars.sumV = 0;
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&idle);
}

static QState zero_crossing(emon *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            QActive_arm((QActive *)me, 1 sec);
            adc_read(emon_vars.pin); // Make changes here
            return Q_HANDLED();
        }
        case EVENT_ADC_COVERSION_COMPLETE:
        {
            emon_vars.startV = (uint16_t)Q_PAR(me);
            if ((emon_vars.startV < 550) && (emon_vars.startV > 440))
            {
                QActive_post((QActive *)me, EVENT_EMON_ZERO_CROSS_FOUND, 0);
                return Q_TRAN(read_entity);
            }
            adc_read(emon_vars.pin);
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post(me->master, EVENT_EMON_ERROR, 0);
            return Q_TRAN(&idle);
        }
        case Q_EXIT_SIG:
        {
            QActive_disarm((QActive *)me);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&read_entity);
}

static QState read_vcc(emon *const me)
{
    uint16_t temp;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            /* Read the bandgap voltage of 1.1v */
            ADMUX = (ADMUX & 0xF0);
            ADMUX |= 0x0E;
            QActive_arm((QActive *)me, 1 sec);
            return Q_HANDLED();
        }
        case EVENT_ADC_COVERSION_COMPLETE:
        {
            temp = (uint16_t)Q_PAR(me);
            emon_vars.vcc = 1126400L / temp;
#ifdef SERIAL_DEBUG
            Softserial_print_byte(temp);
            Softserial_println("");
            Softserial_print("VCC = ");
            Softserial_print_byte(emon_vars.vcc);
            Softserial_println("");
#endif
            QActive_post((QActive *)me, EVENT_EMON_VCC_READ_DONE, 0);
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_disarm((QActive *)me);
            ADCSRA |= _BV(ADSC);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&read_entity);
}
