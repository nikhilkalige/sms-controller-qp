/**
 ******************************************************************************
 *
 * @file       util.c
 * @author     Lonewolf
 * @brief      Generic Utility Functions
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

#include "util.h"

uint16_t ascii_to_integer(uint8_t * data)
{
    uint8_t i = 0;
    uint16_t num = 0;
    while(data[i] != '0')
    {
        if((data[i] > '9') || (data[i] < '0'))
        {
            return 0;
        }
        else
        {
            num = (num * 10) + (data[i] - 0x30);
        }
    }
    return num;
}