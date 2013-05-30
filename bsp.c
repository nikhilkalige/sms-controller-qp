#include "settings.h"
#include "bsp.h"
#include "serial.h"
#include "gsm.h"

#define LED_ON(num) (PORTD |= 1 << num)
#define LED_OFF(num) (PORTD &= ~ (1 << num))
#define LED_OFF_ALL()      (PORTD = 0xFF)

#define TICK_DIVIDER       ((F_CPU / BSP_TICKS_PER_SEC / 1024) - 1)

#if TICK_DIVIDER > 255
#   error BSP_TICKS_PER_SEC too small
#elif TICK_DIVIDER < 2
#   error BSP_TICKS_PER_SEC too large
#endif


uint8_t commandFlag = 0;

/*..........................................................................*/
ISR(TIMER2_COMPA_vect) 
{
	//PORTD ^= (1 << 4);
	QK_ISR_ENTRY();                       /* inform QK-nano about ISR entry */
	QF_tick();
	// PORTD ^= (1 << 7);
	QActive_postISR((QActive *)&AO_Blinky, TIME_TICK_SIG, 0);
	QK_ISR_EXIT();                         /* inform QK-nano about ISR exit */
	//PORTD ^= (1 << 4);
}
/*..........................................................................*/


void BSP_init(void) 
{
	DDRB  = 0xFF;
	DDRD  = 0xFF;                    /* All PORTD pins are outputs for LEDs */
}
/*..........................................................................*/


void QF_onStartup(void) 
{
	cli();                    /* make sure that all interrupts are disabled */

	TCCR2A = ((1 << WGM21) | (0 << WGM20));
	TCCR2B = (( 1 << CS22 ) | ( 1 << CS21 ) | ( 1 << CS20 ));        // 1/2^10
	ASSR &= ~(1<<AS2);
        TIMSK2 = (1 << OCIE2A);                 // Enable TIMER2 compare Interrupt
        TCNT2 = 0;
    
	/* set the output compare value */
    	OCR2A = TICK_DIVIDER;
    
    	sei();                     /* make sure that all interrupts are enabled */
}
/*..........................................................................*/


void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    	(void)file;                                   /* avoid compiler warning */
    	(void)line;                                   /* avoid compiler warning */
	QF_INT_DISABLE();
//    char int_str[10];
//    itoa(line, int_str, 10);
//   Serial_print_string_flash(file);
//    Serial_print_string(int_str);
    	PORTB |= 1 << 5;                                           /* all LEDs on */
    	for (;;) {       /* NOTE: replace the loop with reset for final version */
		PORTB = 1 << 3; 
	}
}

void QK_onIdle(void) 
{
	PORTD ^= (1 << 5);
    /* toggle the LED number 7 on and then off, see NOTE01 */
	QF_INT_DISABLE();
	PORTB |= 1 << 2;
	PORTB &= ~ (1 << 2);
	QF_INT_ENABLE();

#ifdef NDEBUG
	SMCR = (0 << SM0) | (1 << SE);/*idle sleep mode, adjust to your project */
	__asm__ __volatile__ ("sleep" "\n\t" :: );
    	SMCR = 0;                                           /* clear the SE bit */
#endif
}

/* Define the blinky class */

typedef struct BlinkyTag 
{
	/* protected: */
	QActive super;
}Blinky;

/* Protected */
static QState Blinky_initial(Blinky *me);
static QState Blinky_active(Blinky *me);


/* Global objects ----------------------------------------------------------*/
Blinky AO_Blinky;                 /* the single instance of the Blinky active object */


/* Pelican class definition ------------------------------------------------*/
void Blinky_ctor(void)
{
	QActive_ctor(&AO_Blinky.super, Q_STATE_CAST(&Blinky_initial));
}


static QState Blinky_initial(Blinky *me)
{
	return Q_TRAN(&Blinky_active);
}

static QState Blinky_active(Blinky *me)
{
	QState status;
	switch(Q_SIG(me))
	{
		case Q_INIT_SIG:
		{
			//PORTD ^= (1 << 7);
	    		/* Set the command to send AT and post event to GSM to start processing */
			//commandFlag = 1;
			GSM_buffer.cmd_byte = SEND_SMS;
			QActive_post((QActive *)&AO_GSM, GSM_PROCESS_SIG, 0);
	    		//Serial_SendStringNonBlocking(&AO_Blinky, "We are sending some data \r\n");
			PORTD ^= (1 << 7);
			break;
		}

		case GSM_DONE_SIG:
		{
			Serial_SendStringNonBlocking(&AO_Blinky, "Done signal \r\n");
			break;
		}

		case GSM_SUCCESS_EVENT:
		{
			Serial_SendStringNonBlocking(&AO_Blinky, "Finished sending data\r\n");
			break;
		}

		case GSM_FAILURE_EVENT:
		{
			Serial_SendStringNonBlocking(&AO_Blinky, "Failed sending data\r\n");
			break;
		}
		case SERIAL_TRANSMIT_SIG:
		{
			Serial_SendStringNonBlocking(&AO_Blinky, "I love you Nikhil :|@ \r\n");
			break;
		}

		default:
		status = Q_SUPER(&QHsm_top);
		break;
	}
	return status;
}


