/****************************************************************************

 i106_time.h - 

 Copyright (c) 2006 Irig106.org

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

#ifndef _I106_TIME_H
#define _I106_TIME_H

//#include "irig106ch10.h"
#include <time.h>

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */

#define CH4BINARYTIME_HIGH_LSB_SEC   655.36
#define CH4BINARYTIME_LOW_LSB_SEC      0.01
#define _100_NANO_SEC_IN_MICRO_SEC    10

typedef PUBLIC enum DateFmt
    {
    I106_DATEFMT_DAY         =  0,
    I106_DATEFMT_DMY         =  1,
    } EnI106DateFmt;


/*
 * Data structures
 * ---------------
 */

// Time has a number of representations in the IRIG 106 spec.
// The structure below is used as a convenient standard way of
// representing time.  The nice thing about standards is that there 
// are so many to choose from, and time is no exception. But none of 
// the various C time representations really fill the bill. So I made 
// a new time representation.  So there.
typedef PUBLIC struct SuIrig106Time_S
    {
    uint32_t        ulSecs;     // This is a time_t
    uint32_t        ulFrac;     // LSB = 100ns
    EnI106DateFmt   enFmt;      // Day or DMY format
    } SuIrig106Time;


// Relative time to absolute time reference
typedef PUBLIC struct SuTimeRef_S
    {
    int64_t         uRelTime;          // Relative time from header
    SuIrig106Time   suIrigTime;        // Clock time from IRIG source
    uint16_t        bRelTimeValid   :  1;
    uint16_t        bAbsTimeValid   :  1;
    uint16_t        uReserved       :  14;
    } SuTimeRef;


/// IRIG 106 secondary header time in Ch 4 BCD format
typedef PUBLIC struct SuI106Ch4_BCD_Time_S
    {
    uint16_t      uMin1     : 4;    // High order time
    uint16_t      uMin10    : 3;
    uint16_t      uHour1    : 4;
    uint16_t      uHour10   : 2;
    uint16_t      uDay1     : 3;
    uint16_t      uSec0_01  : 4;    // Low order time
    uint16_t      uSec0_1   : 4;
    uint16_t      uSec1     : 4;
    uint16_t      uSec10    : 2;
    uint16_t      uReserved : 2;
    uint16_t      uUSecs;           // Microsecond time
#if !defined(__GNUC__)
    } SuI106Ch4_BCD_Time;
#else
    } __attribute__ ((packed)) SuI106Ch4_BCD_Time;
#endif


/// IRIG 106 secondary header time in Ch 4 binary format
typedef PUBLIC struct SuI106Ch4_Binary_Time_S
    {
    uint16_t      uHighBinTime;     // High order time
    uint16_t      uLowBinTime;      // Low order time
    uint16_t      uUSecs;           // Microsecond time
#if !defined(__GNUC__)
    } SuI106Ch4_Binary_Time;
#else
    } __attribute__ ((packed)) SuI106Ch4_Binary_Time;
#endif


/// IRIG 106 secondary header time in IEEE-1588 format
typedef PUBLIC struct SuIEEE1588_Time_S
    {
    uint32_t      uNanoSeconds;     // Nano-seconds
    uint32_t      uSeconds;         // Seconds
#if !defined(__GNUC__)
    } SuIEEE1588_Time;
#else
    } __attribute__ ((packed)) SuIEEE1588_Time;
#endif


/// Intra-packet header relative time counter format
typedef PUBLIC struct SuIntraPacketRtc_S
    {
    uint8_t       aubyRefTime[6];   // Reference time
    uint16_t      uReserved;
#if !defined(__GNUC__)
    } SuIntraPacketRtc;
#else
    } __attribute__ ((packed)) SuIntraPacketRtc;
#endif

/// Intra-packet header time stamp - raw data
typedef PUBLIC struct SuIntraPacketTS_S
    {
    uint8_t       aubyIntPktTime[8];   // Time Stamp    
#if !defined(__GNUC__)
    } SuIntraPacketTS;
#else
    } __attribute__ ((packed)) SuIntraPacketTS;
#endif

/*
 * Global data
 * -----------
 */


/*
 * Function Declaration
 * --------------------
 */

//EnI106Status I106_CALL_DECL 
//    enI106_SetRelTime(int              iI106Ch10Handle,
//                      SuIrig106Time  * psuTime,
//                      uint8_t          abyRelTime[]);
EnI106Status I106_CALL_DECL 
    enI106_SetRelTime(int              iI106Ch10Handle,
                      SuIrig106Time  * psuTime,
                      uint8_t          abyRelTime[]);

EnI106Status I106_CALL_DECL 
    enI106_Rel2IrigTime(int                  iI106Ch10Handle,
                        uint8_t              abyRelTime[],
                        SuIrig106Time      * psuTime);

EnI106Status I106_CALL_DECL 
    enI106_RelInt2IrigTime(int               iI106Ch10Handle,
                           int64_t           llRelTime,
                           SuIrig106Time   * psuTime);

EnI106Status I106_CALL_DECL 
    enI106_Irig2RelTime(int              iI106Ch10Handle,
                        SuIrig106Time  * psuTime,
                        uint8_t          abyRelTime[]);

EnI106Status I106_CALL_DECL 
    enI106_Ch4Binary2IrigTime(SuI106Ch4_Binary_Time * psuCh4BinaryTime,
                              SuIrig106Time         * psuIrig106Time);

EnI106Status I106_CALL_DECL 
    enI106_IEEE15882IrigTime(SuIEEE1588_Time * psuIEEE1588Time,
                              SuIrig106Time  * psuIrig106Time);

EnI106Status I106_CALL_DECL
    vFillInTimeStruct(SuI106Ch10Header * psuHeader,
                       SuIntraPacketTS  * psuIntraPacketTS, 
                       SuTimeRef        * psuTimeRef);

// Warning - array to int / int to array functions are little endian only!

void I106_CALL_DECL 
    vLLInt2TimeArray(int64_t * pllRelTime,
                     uint8_t   abyRelTime[]);


void I106_CALL_DECL 
    vTimeArray2LLInt(uint8_t   abyRelTime[],
                     int64_t * pllRelTime);


EnI106Status I106_CALL_DECL 
    enI106_SyncTime(int     iI106Ch10Handle,
                    int     bRequireSync,       // Require external time sync
                    int     iTimeLimit);        // Max scan ahead time in seconds, 0 = no limit

EnI106Status I106_CALL_DECL 
    enI106Ch10SetPosToIrigTime(int iI106Ch10Handle, SuIrig106Time * psuSeekTime);


// General purpose time utilities
// ------------------------------

// Convert IRIG time into an appropriate string
char * IrigTime2String(SuIrig106Time * psuTime);

// This is handy enough that we'll go ahead and export it to the world
uint32_t I106_CALL_DECL mkgmtime(struct tm * psuTmTime);

#ifdef __cplusplus
} // end extern "C"
} // end namespace i106
#endif

#endif
