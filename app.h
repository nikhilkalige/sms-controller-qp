/**
 ******************************************************************************
 *
 * @file       App.h
 * @author     Lonewolf
 * @brief      Application Header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#ifndef APP_H_
#define APP_H_


#include <qpn_port.h>


void App_ctor(void);

typedef struct App_tag
{
    QActive super;
    /* Public Members */
    uint8_t buffer[100];
} App;

App app_mod;

#endif					/* app.h */