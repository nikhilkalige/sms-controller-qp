#include "serial.h"
#include "fifo_buffer.h"
#include <avr/io.h>
#include <util/delay.h>
#include "bsp.h"
#include "gsm.h"

extern t_fifo_buffer rx_buffer;

static QEvt l_blinkyQueue[10];
static QEvt l_GSMQueue[10];
/* QF_active[] array defines all active object control blocks --------------*/
QActiveCB const Q_ROM Q_ROM_VAR QF_active[] = {
	 { (QActive *)0, (QEvt *)0 ,0 },
     { (QActive *)&AO_Blinky, l_blinkyQueue, Q_DIM(l_blinkyQueue)},
     { (QActive *)&AO_GSM, l_GSMQueue, Q_DIM(l_GSMQueue)},
};

/* make sure that the QF_active[] array matches QF_MAX_ACTIVE in qpn_port.h */
Q_ASSERT_COMPILE(QF_MAX_ACTIVE == Q_DIM(QF_active) - 1);

//uint8_t buffer[100];
int main(void)
{
	DDRB  = 0xFF;
    	DDRD  = 0xFF;
	Serial_init();
	//Serial_SendStringNonBlocking("Hi Lonewolf, we are ready to hunt \r\n");
	//Serial_SendStringNonBlocking("We are waiting for input \r\n");
	//fifoBuf_clearData(&rx_buffer);
	Blinky_ctor();  /* instantiate the Pelican AO */
	GSM_ctor();
    	BSP_init();      /* initialize the board */
	//Serial_print_string("Hi Lonewolf, we are ready to hunt \r\n");
	//Serial_print_string_flash(test_string);
	//PORTD ^= (1 << 7);
	//PORTD ^= (1 << 2);
    	return QF_run(); /* transfer control to QF-nano */
}