#ifndef bsp_h
#define bsp_h

#include "settings.h"



void BSP_init(void);

static 	uint16_t 	BLED_tickCount		=	0;
static 	uint16_t 	BLED_togglePoint	=	40;
static 	uint8_t		BLED_ledState		=	0;

void Blinky_ctor(void);
extern struct BlinkyTag AO_Blinky;

#endif                                                             /* bsp_h */


