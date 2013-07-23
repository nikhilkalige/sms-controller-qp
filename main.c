#include "serial.h"
#include "fifo_buffer.h"
#include <avr/io.h>
#include <util/delay.h>
#include "bsp.h"
#include "gsm.h"

//extern t_fifo_buffer rx_buffer;

static QEvt l_blinkyQueue[10];
static QEvt l_GSMQueue[10];
static QEvt l_ComQueue[10];
static QEvt l_AppQueue[10];
/* QF_active[] array defines all active object control blocks --------------*/
QActiveCB const Q_ROM Q_ROM_VAR QF_active[] =
{
    { (QActive *)0, (QEvt *)0 , 0 },
    { (QActive *) &gsm_dev, l_GSMQueue, Q_DIM(l_GSMQueue)},
    { (QActive *) &app_mod, l_AppQueue, Q_DIM(l_AppQueue)},
    { (QActive *) &com_drv, l_ComQueue, Q_DIM(l_ComQueue)},
};

/* make sure that the QF_active[] array matches QF_MAX_ACTIVE in qpn_port.h */
Q_ASSERT_COMPILE(QF_MAX_ACTIVE == Q_DIM(QF_active) - 1);

//uint8_t buffer[100];
uint8_t rx_buffer[100];
uint8_t tx_buffer[100];

int main(void)
{
    DDRB  = 0xFF;
    DDRD  = 0xFF;
    //Serial_init();
    //Serial_SendStringNonBlocking("Hi Lonewolf, we are ready to hunt \r\n");
    //Serial_SendStringNonBlocking("We are waiting for input \r\n");
    //fifoBuf_clearData(&rx_buffer);
    GSM_config(&app_mod, &com_drv, app_mod.buffer);
    Com_init(tx_buffer, 100, rx_buffer, 100);
    BSP_init();      /* initialize the board */
    App_ctor();
    GSM_ctor();
    Com_ctor();

    //Serial_print_string("Hi Lonewolf, we are ready to hunt \r\n");
    //Serial_print_string_flash(test_string);
    //PORTD ^= (1 << 7);
    //PORTD ^= (1 << 2);
    return QF_run(); /* transfer control to QF-nano */
}