/**
 ******************************************************************************
 *
 * @file       app_settings.h
 * @author     Lonewolf
 * @brief      Application Settings Header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#ifndef APP_SETTINGS_H_
#define APP_SETTINGS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define MAX_NO_USERS            4
#define MAX_PASSWORD_LENGTH     4
#define MAX_NO_LENGTH           14

/******************************************************************
*       All defnitions regarding the EEPROM are defined here
*       EEPROM size is 1024 bytes for ATMEGA328p
*       The EEPROM will be divided into sections as follows:
*               The first 20 bytes will hold the length in the form
*                       Length will be stored as single uint8_t
*                       So we have 20 index.
*                       The length should be always less than 40 bytes
*               The data will be started from 21st location
******************************************************************/

#define FIRST_BOOT_ADD 1023
#define FIRST_BOOT_VALUE 0xAB

/*
    typedef struct user_tag
    {
        uint8_t id;
        unsigned char phone_no[15];
        unsigned char password[5];
        uint8_t pwd_present;
    } user;
    22 bytes per user, 4 users maximum
    22 * 4 = 88 bytes
    10 bytes for the header
    ie. 88 + 10 = 98 bytes
    Provide extra space for 2 more users
    98 + (22 * 2) = 142
*/
#define USER_SIZE           142
#define EEPROM_USER_HEAD    0

/*
    typedef struct
    {
        uint8_t pass_present;
        uint8_t broadcast_mssg;
        uint8_t no_users;
        uint8_t status_freq; // In Minutes
    } settings;
    4 bytes per settings, only 1 table
    1 * 4 = 4 bytes
    10 bytes for the header
    ie. 4 + 10 = 14 bytes
    Head will start at 142
    Add 10 bytes of extra space
    14 + 10 = 24
*/
#define SETTINGS_SIZE           166  /* 142 + 24 = 166 */
#define EEPROM_SETTINGS_HEAD    142


#if 0
/*  GPRS Settings for the device:  "APN","USER","PASS" */
/*  70 bytes for GPRS Settings */
/*  Header Size is 10 bytes */
/*#define GPRS_SETTINGS_SIZE      */       // 270 /*  190 + 80 =270 */
/*#define EEPROM_GRPS_SETTINGS_HEAD               190
typedef struct
{
    char APN_User[70];
} GPRS_Settings;
*/
#endif

/******************************************************************
FLASH Related Storage Information
******************************************************************/
/*
const char Op_1[] PROGMEM = "1.Change PASS\n";
const char Op_2[] PROGMEM = "2.Add NO\n";
const char Op_3[] PROGMEM = "3.Del NO\n";
const char Op_4[] PROGMEM = "En/Dis PASS\n";
const char Op_5[] PROGMEM = "En/Dis Broadcast\n";
const char Op_6[] PROGMEM = "Status Freq\n";
const char Op_7[] PROGMEM = "Set Time\n";
const char Op_8[] PROGMEM = "GPRS\n";
const char Op_9[] PROGMEM = "En/Dis TCP\n";
const char Op_10[] PROGMEM = "COSM\n";
const char Op_11[] PROGMEM = "Ping Freq\n";
const char Op_12[] PROGMEM = "Calibrate\n";
const char Op_13[] PROGMEM = "RESET\n";
const char Resp[] PROGMEM = "<INDEX>";
const char * Menu_Strings[] PROGMEM = { Op_1,Op_2,Op_3,Op_4,Op_5,Op_6,Op_7,Op_8,Op_9,Op_10,Op_11,Op_12,Op_13,Resp};
*/
const char Menu_Strings[] PROGMEM = "1.Change PASS\n2.Add NO\n3.Del NO\n4.En PASS\n5.En Broadcast\n6.Status Freq\n7.Set Time\n8.GPRS\n9.En TCP\n10.COSM\n11.Ping Freq\n12.Calibrate\n13.RESET\nReply <INDEX>";

/*****************************************************************
    CHANGE PASSWORD
******************************************************************/
const char Rep1[] PROGMEM = "<NEW PASSWORD> Should be less than 6 characters";

/*****************************************************************
    ADD NUMBER
******************************************************************/
const char Rep2[] PROGMEM = "<PHONE NUMBER>";

/*****************************************************************
    DELETE NUMBER
******************************************************************/
const char Rep3[] PROGMEM = "<INDEX>";

/*****************************************************************
    ENABLE/DISABLE PASSWORD
******************************************************************/
const char Rep4[] PROGMEM = "<EN> for Enabling Password\n<DI> for Disabling Password";

/*****************************************************************
    ENABLE/DISABLE BROADCAST
******************************************************************/
const char Rep5[] PROGMEM = "<EN> for Enabling Broadcast\n<DI> for Disabling Broadcast";

/*****************************************************************
    STATUS FREQUENCY
******************************************************************/
const char Rep6[] PROGMEM = "<MIN>\n<0> to disable Mssg\nEx <10> for 10 min, should be less than 255";

/*****************************************************************
    SET TIME
******************************************************************/
const char Rep7[] PROGMEM = "Time Synchronization Done";

/*****************************************************************
    GPRS
******************************************************************/
const char Rep8[] PROGMEM = "Experimental";

/*****************************************************************
    ENABLE TCP
******************************************************************/
const char Rep9[] PROGMEM = "Experimental";

/*****************************************************************
    COSM
******************************************************************/
const char Rep10[] PROGMEM = "Experimental";

/*****************************************************************
    PING FREQUENCY
******************************************************************/
const char Rep11[] PROGMEM = "Experimental";

/*****************************************************************
    CALIBRATE
******************************************************************/
const char Rep12[] PROGMEM = "Experimental";

/*****************************************************************
    RESET
******************************************************************/
const char Rep13[] PROGMEM = "Experimental";


#endif