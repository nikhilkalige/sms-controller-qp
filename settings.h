/**
 ******************************************************************************
 *
 * @file       settings.h
 * @author     Lonewolf
 * @brief      System Settings and Inlcudes header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <avr/io.h> 
#include <avr/interrupt.h>

/* Sys timer tick per seconds */
#define BSP_TICKS_PER_SEC    100

/* Uncomment inorder to include Quantum Leaps */
#define Q_LEAPS


/* Inlcude Quantum Leaps related files */
#ifdef Q_LEAPS
 #include <qpn_port.h>
#endif


 /* System Peripheral Headers */
 #include "serial.h"

 

#ifdef Q_LEAPS
 enum App_signals
 { 
 	TIME_TICK_SIG = Q_USER_SIG, 	// time tick for all classes that sign up
	SERIAL_TRANSMIT_SIG,		// Serial transmit signal
	SERIAL_RECIEVE_SIG,		// Serial recieve signal
	GSM_PROCESS_SIG,		// Start processing the command
	GSM_DONE_SIG,			// GSM Finished processing
	GSM_SUCCESS_EVENT,
	GSM_FAILURE_EVENT,
    	MAX_PUB_SIG,			// the last published signal
 };
#endif


 #endif 		/* settings.h */