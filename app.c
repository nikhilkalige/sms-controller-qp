/**
 ******************************************************************************
 *
 * @file       app.c
 * @author     Lonewolf
 * @brief      Application functions
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#include "app.h"
#include "gsm.h"
#include "softserial.h"
//App app_mod;

static QState initiate_app(App *const me);
static QState init(App *const me);
static QState active(App *const me);


/************************************************************************************************************************************
                                                    ***** Public Funtions *****
************************************************************************************************************************************/

void App_ctor(void)
{
    QActive_ctor(&app_mod.super, Q_STATE_CAST(&initiate_app));
}

/************************************************************************************************************************************
                                                    ***** Private Funtions *****
************************************************************************************************************************************/

/************************************************************************************************************************************
                                                    ***** State Machines *****
************************************************************************************************************************************/

static QState initiate_app(App *const me)
{
    return Q_TRAN(&init);
}

static QState init(App *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            QActive_post((QActive *)&gsm_dev, EVENT_SYSTEM_START_AO, 0);
            Softserial_println("INIT APP TIME");
            QActive_arm((QActive *)me, 5 sec);
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            Softserial_println("INIT APP T RECIEVED");
            QActive_post((QActive *)&gsm_dev, EVENT_SYSTEM_GSM_INIT, 0);
            //QActive_arm((QActive *)me, 1 sec);
            return Q_HANDLED();
        }
        case EVENT_GSM_INIT_DONE:
        {
            Softserial_println("GSM INIT DONE");
            return Q_TRAN(&active);
        }
        case EVENT_GSM_INIT_FAILURE:
        {
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QActive_disarm((QActive *)me);
            return Q_HANDLED();
        }
        default:
        {
            return Q_SUPER(&QHsm_top);
        }
    }
}

static QState active(App *const me)
{
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            Softserial_println("Posting GSM init ");
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_NETWORK_READ_REQUEST, 0);
            return Q_HANDLED();
        }
        case Q_INIT_SIG:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_BUSY:
        {
            QActive_arm((QActive *)me, 1);
            return Q_HANDLED();
        }
        case Q_TIMEOUT_SIG:
        {
            return Q_HANDLED();
        }
        case EVNET_GSM_MODULE_FAILURE:
        {
            // Error
            // Holy cow
            return Q_HANDLED();
        }
        case EVENT_GSM_NETWORK_ERROR:
        {
            return Q_HANDLED();
        }
        case EVENT_GSM_NETWORK_CONNECTED:
        {
            QActive_post((QActive *)&gsm_dev, EVENT_GSM_CLOCK_READ, 0);
            // Ready to roll my friend
            return Q_HANDLED();
        }
        case EVENT_GSM_CLOCK_READ_DONE:
        {
            Softserial_println("time read success");
            Softserial_println((char *)me->buffer);
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        default:
        {
            return Q_SUPER(&QHsm_top);
        }
    }
}
