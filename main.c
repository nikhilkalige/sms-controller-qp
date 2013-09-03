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

static QEvt l_blinkyQueue[10];
static QEvt l_GSMQueue[10];
static QEvt l_ComQueue[10];
static QEvt l_AppQueue[10];
static QEvt l_emonQueue[10];
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

const char test_string[] PROGMEM = "FLASH TEST OK";




int main(void)
{
    uint8_t temp, temp1;
    //temp = MCUCR;
    temp1 = MCUSR;
    temp = MCUSR;
    Softserial_begin(9600);
    Softserial_println("STARTING");
    DDRD |= (1 << 7);
    PORTD |= (1 << 7);
    //DDRB = 0xff;
    /* Softserial_print("MCUCR = ");
     Softserial_print_byte(temp);
     Softserial_println("");
     Softserial_print("MCUSR = ");
     Softserial_print_byte(temp1);
     Softserial_println("");
     */
    //DDRB  = 0xFF;
    // DDRD  = 0xFF;
    //Serial_init();
    //Serial_SendStringNonBlocking("Hi Lonewolf, we are ready to hunt \r\n");
    //Serial_SendStringNonBlocking("We are waiting for input \r\n");
    //fifoBuf_clearData(&rx_buffer);
    GSM_config(&app_dev, &com_drv);
    Com_init((QActive *)&gsm_dev, tx_buffer, 100, rx_buffer, 100);
    emon_config((QActive *)&app_dev);
    //void Com_init(QActive* master, uint8_t *tx_buffer, uint8_t tx_size, uint8_t *rx_buffer, uint8_t rx_size)
    BSP_init();      /* initialize the board */
    App_ctor();
    GSM_ctor();
    Com_ctor();
    emon_ctor();
    Softserial_println("ALL SYSTEMS INITIALIZED");
    //Serial_print_string("Hi Lonewolf, we are ready to hunt \r\n");
    //Serial_print_string_flash(test_string);
    //PORTD ^= (1 << 7);
    //PORTD ^= (1 << 2);
    return QF_run(); /* transfer control to QF-nano */
    Softserial_println("ERROR EXITING MAIN");
    while(1);
}
