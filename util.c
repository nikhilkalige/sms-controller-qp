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

uint16_t ascii_to_integer(uint8_t *data)
{
    uint8_t i = 0;
    uint16_t num = 0;
    while (data[i] != '0')
    {
        if ((data[i] > '9') || (data[i] < '0'))
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

void convert_uppercase(char *string)
{
    uint8_t  i = 0;
    while (*(string + i) != '\0')
    {
        if ((*(string + i) > 0x60) && (*(string + i) < 0x7B))
        {
            *(string + i) -= 0x20;
        }
        i++;
    }
#ifdef SOFT_DEBUG
    mySerial.println(string);
#endif
}