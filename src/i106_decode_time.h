/****************************************************************************

 i106_decode_time.h - 

 Copyright (c) 2005 Irig106.org

 All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are 
 met:

   * Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright 
     notice, this list of conditions and the following disclaimer in the 
     documentation and/or other materials provided with the distribution.

   * Neither the name Irig106.org nor the names of its contributors may 
     be used to endorse or promote products derived from this software 
     without specific prior written permission.

 This software is provided by the copyright holders and contributors 
 "as is" and any express or implied warranties, including, but not 
 limited to, the implied warranties of merchantability and fitness for 
 a particular purpose are disclaimed. In no event shall the copyright 
 owner or contributors be liable for any direct, indirect, incidental, 
 special, exemplary, or consequential damages (including, but not 
 limited to, procurement of substitute goods or services; loss of use, 
 data, or profits; or business interruption) however caused and on any 
 theory of liability, whether in contract, strict liability, or tort 
 (including negligence or otherwise) arising in any way out of the use 
 of this software, even if advised of the possibility of such damage.

 ****************************************************************************/

#ifndef _I106_DECODE_TIME_H
#define _I106_DECODE_TIME_H

//#include "irig106ch10.h"

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */

typedef enum
    {
    I106_TIMEFMT_IRIG_B      =  0x00,
    I106_TIMEFMT_IRIG_A      =  0x01,
    I106_TIMEFMT_IRIG_G      =  0x02,
    I106_TIMEFMT_INT_RTC     =  0x03,
    I106_TIMEFMT_GPS_UTC     =  0x04,
    I106_TIMEFMT_GPS_NATIVE  =  0x05,
    } EnI106TimeFmt;

typedef enum
    {
    I106_TIMESRC_INTERNAL      =  0x00,
    I106_TIMESRC_EXTERNAL      =  0x01,
    I106_TIMESRC_INTERNAL_RMM  =  0x02,
    I106_TIMESRC_NONE          =  0x0F    
    } EnI106TimeSrc;


/*
 * Data structures
 * ---------------
 */

/* Time Format 1 */

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

// Channel specific header
typedef struct 
    {
    uint32_t    uTimeSrc    :  4;      // Time source    
    uint32_t    uTimeFmt    :  4;      // Time format
    uint32_t    bLeapYear   :  1;      // Leap year
    uint32_t    uDateFmt    :  1;      // Date format
    uint32_t    uReserved2  :  2;
    uint32_t    uReserved3  : 20;
#if !defined(__GNUC__)
    } SuTimeF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuTimeF1_ChanSpec;
#endif

// Time message - Day format
typedef struct
    {
    uint16_t    uTmn        :  4;      // Tens of milliseconds
    uint16_t    uHmn        :  4;      // Hundreds of milliseconds
    uint16_t    uSn         :  4;      // Units of seconds
    uint16_t    uTSn        :  3;      // Tens of seconds
    uint16_t    Reserved1   :  1;      // 0

    uint16_t    uMn         :  4;      // Units of minutes
    uint16_t    uTMn        :  3;      // Tens of minutes
    uint16_t    Reserved2   :  1;      // 0
    uint16_t    uHn         :  4;      // Units of hours
    uint16_t    uTHn        :  2;      // Tens of Hours
    uint16_t    Reserved3   :  2;      // 0

    uint16_t    uDn         :  4;      // Units of day number
    uint16_t    uTDn        :  4;      // Tens of day number
    uint16_t    uHDn        :  2;      // Hundreds of day number
    uint16_t    Reserved4   :  6;      // 0
#if !defined(__GNUC__)
    } SuTime_MsgDayFmt;
#else
    } __attribute__ ((packed)) SuTime_MsgDayFmt;
#endif

// Time message - DMY format
typedef struct
    {
    uint16_t    uTmn        :  4;      // Tens of milliseconds
    uint16_t    uHmn        :  4;      // Hundreds of milliseconds
    uint16_t    uSn         :  4;      // Units of seconds
    uint16_t    uTSn        :  3;      // Tens of seconds
    uint16_t    Reserved1   :  1;      // 0

    uint16_t    uMn         :  4;      // Units of minutes
    uint16_t    uTMn        :  3;      // Tens of minutes
    uint16_t    Reserved2   :  1;      // 0
    uint16_t    uHn         :  4;      // Units of hours
    uint16_t    uTHn        :  2;      // Tens of Hours
    uint16_t    Reserved3   :  2;      // 0

    uint16_t    uDn         :  4;      // Units of day number
    uint16_t    uTDn        :  4;      // Tens of day number
    uint16_t    uOn         :  4;      // Units of month number
    uint16_t    uTOn        :  1;      // Tens of month number
    uint16_t    Reserved4   :  3;      // 0

    uint16_t    uYn         :  4;      // Units of year number
    uint16_t    uTYn        :  4;      // Tens of year number
    uint16_t    uHYn        :  4;      // Hundreds of year number
    uint16_t    uOYn        :  2;      // Thousands of year number
    uint16_t    Reserved5   :  2;      // 0
#if !defined(__GNUC__)
    } SuTime_MsgDmyFmt;
#else
    } __attribute__ ((packed)) SuTime_MsgDmyFmt;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_TimeF1(SuI106Ch10Header  * psuHeader,
                         void              * pvBuff,
                         SuIrig106Time     * psuTime);

void I106_CALL_DECL 
    enI106_Decode_TimeF1_Buff(int                 iDateFmt,
                              int                 bLeapYear,
                              void              * pvTimeBuff,
                              SuIrig106Time     * psuTime);

EnI106Status I106_CALL_DECL 
    enI106_Encode_TimeF1(SuI106Ch10Header  * psuHeader,
                         unsigned int        uTimeSrc,
                         unsigned int        uFmtTime,
                         unsigned int        uFmtDate,
                         SuIrig106Time     * psuTime,
                         void              * pvBuffTimeF1);

#ifdef __cplusplus
}
}
#endif

#endif
