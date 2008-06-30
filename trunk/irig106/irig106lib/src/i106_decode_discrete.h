/****************************************************************************

 i106_decode_discrete.h - 

 Copyright (c) 2008 Irig106.org

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

 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_DISCRETE_H
#define _I106_DECODE_DISCRETE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */


/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Channel specific data word
// --------------------------

typedef PUBLIC struct DiscreteF1_ChanSpec_S
    {
    uint32_t    uRecordState    :  1;      // Record on state/time
    uint32_t    uAlignment      :  1;      // Data alignment
    uint32_t    uReserved1      :  1;      // 
    uint32_t    uLength         :  5;      // Number of bits
    uint32_t    uReserved2      : 24;      // 
#if !defined(__GNUC__)
    } SuDiscreteF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuDiscreteF1_ChanSpec;
#endif

// Data with various forms of intra-packet headers
// -----------------------------------------------

// Index message, with RTC format time, without optional data
typedef PUBLIC struct DiscreteF1_RTC_S
    {
    SuIntraPacketRtc        suRtcTime;  // RTC format time stamp
    uint32_t                suData;     // Data about the event
#if !defined(__GNUC__)
    } SuDiscreteF1_RTC;
#else
    } __attribute__ ((packed)) SuDiscreteF1_RTC;
#endif

// Index message, with Ch 4 format time, without optional data
typedef PUBLIC struct DiscreteF1_Ch4Time_S
    {
    SuI106Ch4_Binary_Time   suCh4Time;  // Ch 4 format time stamp
    uint32_t                suData;     // Data about the event
#if !defined(__GNUC__)
    } SuDiscreteF1_Ch4Time;
#else
    } __attribute__ ((packed)) SuDiscreteF1_Ch4Time;
#endif

// Index message, with IEEE-1588 format time, without optional data
typedef PUBLIC struct DiscreteF1_1588Time_S
    {
    SuIEEE1588_Time         su1588Time; // IEEE-1588 format time stamp
    uint32_t                suData;     // Data about the event
#if !defined(__GNUC__)
    } SuDiscreteF1_1588Time;
#else
    } __attribute__ ((packed)) SuDiscreteF1_1588Time;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

/*
 * Function Declaration
 * --------------------
 */



#ifdef __cplusplus
}
#endif

#endif
