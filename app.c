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
    QState status;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
            QActive_arm((QActive*)me, 1);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_TIMEOUT_SIG:
        {
            QActive_post((QActive *)&gsm_dev, EVENT_SYSTEM_GSM_INIT, 0);
            status = Q_HANDLED();
            break;
        }
        case EVENT_GSM_INIT_DONE:
        {
            status = Q_TRAN(&active);
            break;
        }
        case EVENT_GSM_INIT_FAILURE:
        {
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
        	QActive_disarm((QActive*)me);
        	status = Q_HANDLED();
        	break;
        }
        default:
        {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}

static QState active(App *const me)
{
    QState status;
    switch (Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
        	QActive_post((QActive*)&gsm_dev, EVENT_GSM_NETWORK_READ_REQUEST, 0);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case EVENT_GSM_BUSY:
        {
        	QActive_arm((QActive*)me, 1);
        	status = Q_HANDLED();
        	break;
        }
        case Q_TIMEOUT_SIG:
        {

        	status = Q_HANDLED();
        	break;
        }
        case EVNET_GSM_MODULE_FAILURE:
        {
        	// Error
        	// Holy cow
        	status = Q_HANDLED();
        	break;
        }
        case EVENT_GSM_NETWORK_ERROR:
        {
        	status = Q_HANDLED();
        	break;
        }
        case EVENT_GSM_NETWORK_CONNECTED:
        {
        	// Ready to roll my friend
        	status = Q_HANDLED();
        	break;
        }
        case Q_EXIT_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}
