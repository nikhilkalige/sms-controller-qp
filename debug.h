/**
 ******************************************************************************
 *
 * @file       debug.h
 * @author     Lonewolf
 * @brief      System Debug header
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

 #ifndef _DEBUG_H_
 #define _DEBUG_H_

#ifdef DEBUG
#define DEBUG_Assert(test) if (!(test)) DEBUG_Panic(DEBUG_AssertMsg);
#define Assert(test) DEBUG_Assert(test)
#else
#define DEBUG_Assert(test)
#define Assert(test) if (!(test)) while (1);
#endif

 #endif			/* debug.h */