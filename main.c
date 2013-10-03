#include <avr/io.h>
#include <util/delay.h>
#include "softserial.h"
#include "bsp.h"
#include "settings.h"
#include "gsm.h"
#include "app.h"
#include "com.h"
#include "emon.h"

//#include "gsm_settings.h"

//extern t_fifo_buffer rx_buffer;

/* Hardware Configuration */
#define GSM_PWR_DDR     DDRB
#define GSM_BAUD        115200
#define GSM_PWR_PORT    PORTB
#define GSM_PWRKEY      1

static QEvt l_GSMQueue[5];
static QEvt l_ComQueue[5];
static QEvt l_AppQueue[5];
static QEvt l_emonQueue[3];
/* QF_active[] array defines all active object control blocks --------------*/

QActiveCB const Q_ROM Q_ROM_VAR QF_active[] =
{
    { (QActive *)0, (QEvt *)0 , 0 },
    { (QActive *) &gsm_dev, l_GSMQueue, Q_DIM(l_GSMQueue)},
    { (QActive *) &app_dev, l_AppQueue, Q_DIM(l_AppQueue)},
    { (QActive *) &com_drv, l_ComQueue, Q_DIM(l_ComQueue)},
    { (QActive *) &emon_dev, l_emonQueue, Q_DIM(l_emonQueue)},
};

/* make sure that the QF_active[] array matches QF_MAX_ACTIVE in qpn_port.h */
Q_ASSERT_COMPILE(QF_MAX_ACTIVE == Q_DIM(QF_active) - 1);

//uint8_t buffer[100];
uint8_t rx_buffer[100];
uint8_t tx_buffer[100];





int main(void)
{
    Softserial_begin(9600);
    DDRD |= (1 << 7);
    PORTD |= (1 << 7);
    GSM_config(&app_dev, &com_drv);
    Com_init((QActive *)&gsm_dev, tx_buffer, 100, rx_buffer, 100);
    emon_config((QActive *)&app_dev);
    //void Com_init(QActive* master, uint8_t *tx_buffer, uint8_t tx_size, uint8_t *rx_buffer, uint8_t rx_size)
    BSP_init();      /* initialize the board */
    App_ctor();
    GSM_ctor();
    Com_ctor();
    emon_ctor();
    Softserial_println("SYSTEMS INITIALIZED");
    return QF_run(); /* transfer control to QF-nano */
    Softserial_println("EXITING MAIN");
    while (1);
}
