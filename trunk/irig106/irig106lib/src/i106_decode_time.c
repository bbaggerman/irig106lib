/****************************************************************************

 i106_decode_time.c - 

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

   * Neither the name of the Georgia Tech Applied Research Corporation 
     nor the names of its contributors may be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.

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

 $RCSfile: i106_decode_time.c,v $
 $Date: 2005-12-06 16:31:23 $
 $Revision: 1.2 $

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <windows.h>        // For FILETIME

#include "stdint.h"

#include "irig106ch10.h"
#include "i106_decode_time.h"


/*
 * Macros and definitions
 * ----------------------
 */


/*
 * Data structures
 * ---------------
 */

// Channel specific header
typedef struct 
    {
    uint32_t    uExtTime    :  4;      // External time source
    uint32_t    uTimeFmt    :  4;      // Time format
    uint32_t    uDateFmt    :  4;      // Date format
    } SuTimeF1_ChanSpec;

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
    } SuTime_MsgDayFmt;

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
    } SuTime_MsgDmyFmt;


// Relative time reference
typedef struct 
    {
    uint64_t        uRelTime;          // Relative time from header
    SuIrigTimeF1    suIrigTime;        // Clock time from IRIG source
    } SuTimeRef;


// Day of Year to Day and Month
typedef struct
    {
    int  iMonth;     // Month 0 - 11
    int  iDay;       // Day of month 1-31
    } SuDOY2DM;      

/*
 * Module data
 * -----------
 */

// THIS IS KIND OF A PROBLEM BECAUSE THIS SHOULD BE DONE ON A PER FILE BASIS.
// THAT MEANS THIS REALLY SHOULD BE STORED IN THE HEADER.
SuTimeRef   m_suCurrRefTime;           // Current value of IRIG reference time


// These structures are used to convert from day of the year format to 
// day and month.  One is for normal years and the other is for leap years.
// The values and index are of the "struct tm" notion.  That is, the day of 
// the year index is number of days since Jan 1st, i.e. Jan 1st = 0.  For
// IRIG time, Jan 1st = 1.  The month value is months since January, i.e. 
// Jan = 0.  Don't get confused!

SuDOY2DM suDoy2DmNormal[] = {
{ 0,  1}, { 0,  2}, { 0,  3}, { 0,  4}, { 0,  5}, { 0,  6}, { 0,  7}, { 0,  8},
{ 0,  9}, { 0, 10}, { 0, 11}, { 0, 12}, { 0, 13}, { 0, 14}, { 0, 15}, { 0, 16},
{ 0, 17}, { 0, 18}, { 0, 19}, { 0, 20}, { 0, 21}, { 0, 22}, { 0, 23}, { 0, 24},
{ 0, 25}, { 0, 26}, { 0, 27}, { 0, 28}, { 0, 29}, { 0, 30}, { 0, 31}, { 1,  1},
{ 1,  2}, { 1,  3}, { 1,  4}, { 1,  5}, { 1,  6}, { 1,  7}, { 1,  8}, { 1,  9},
{ 1, 10}, { 1, 11}, { 1, 12}, { 1, 13}, { 1, 14}, { 1, 15}, { 1, 16}, { 1, 17},
{ 1, 18}, { 1, 19}, { 1, 20}, { 1, 21}, { 1, 22}, { 1, 23}, { 1, 24}, { 1, 25},
{ 1, 26}, { 1, 27}, { 1, 28}, { 2,  1}, { 2,  2}, { 2,  3}, { 2,  4}, { 2,  5},
{ 2,  6}, { 2,  7}, { 2,  8}, { 2,  9}, { 2, 10}, { 2, 11}, { 2, 12}, { 2, 13},
{ 2, 14}, { 2, 15}, { 2, 16}, { 2, 17}, { 2, 18}, { 2, 19}, { 2, 20}, { 2, 21},
{ 2, 22}, { 2, 23}, { 2, 24}, { 2, 25}, { 2, 26}, { 2, 27}, { 2, 28}, { 2, 29},
{ 2, 30}, { 2, 31}, { 3,  1}, { 3,  2}, { 3,  3}, { 3,  4}, { 3,  5}, { 3,  6},
{ 3,  7}, { 3,  8}, { 3,  9}, { 3, 10}, { 3, 11}, { 3, 12}, { 3, 13}, { 3, 14},
{ 3, 15}, { 3, 16}, { 3, 17}, { 3, 18}, { 3, 19}, { 3, 20}, { 3, 21}, { 3, 22},
{ 3, 23}, { 3, 24}, { 3, 25}, { 3, 26}, { 3, 27}, { 3, 28}, { 3, 29}, { 3, 30},
{ 4,  1}, { 4,  2}, { 4,  3}, { 4,  4}, { 4,  5}, { 4,  6}, { 4,  7}, { 4,  8},
{ 4,  9}, { 4, 10}, { 4, 11}, { 4, 12}, { 4, 13}, { 4, 14}, { 4, 15}, { 4, 16},
{ 4, 17}, { 4, 18}, { 4, 19}, { 4, 20}, { 4, 21}, { 4, 22}, { 4, 23}, { 4, 24},
{ 4, 25}, { 4, 26}, { 4, 27}, { 4, 28}, { 4, 29}, { 4, 30}, { 4, 31}, { 5,  1},
{ 5,  2}, { 5,  3}, { 5,  4}, { 5,  5}, { 5,  6}, { 5,  7}, { 5,  8}, { 5,  9},
{ 5, 10}, { 5, 11}, { 5, 12}, { 5, 13}, { 5, 14}, { 5, 15}, { 5, 16}, { 5, 17},
{ 5, 18}, { 5, 19}, { 5, 20}, { 5, 21}, { 5, 22}, { 5, 23}, { 5, 24}, { 5, 25},
{ 5, 26}, { 5, 27}, { 5, 28}, { 5, 29}, { 5, 30}, { 6,  1}, { 6,  2}, { 6,  3},
{ 6,  4}, { 6,  5}, { 6,  6}, { 6,  7}, { 6,  8}, { 6,  9}, { 6, 10}, { 6, 11},
{ 6, 12}, { 6, 13}, { 6, 14}, { 6, 15}, { 6, 16}, { 6, 17}, { 6, 18}, { 6, 19},
{ 6, 20}, { 6, 21}, { 6, 22}, { 6, 23}, { 6, 24}, { 6, 25}, { 6, 26}, { 6, 27},
{ 6, 28}, { 6, 29}, { 6, 30}, { 6, 31}, { 7,  1}, { 7,  2}, { 7,  3}, { 7,  4},
{ 7,  5}, { 7,  6}, { 7,  7}, { 7,  8}, { 7,  9}, { 7, 10}, { 7, 11}, { 7, 12},
{ 7, 13}, { 7, 14}, { 7, 15}, { 7, 16}, { 7, 17}, { 7, 18}, { 7, 19}, { 7, 20},
{ 7, 21}, { 7, 22}, { 7, 23}, { 7, 24}, { 7, 25}, { 7, 26}, { 7, 27}, { 7, 28},
{ 7, 29}, { 7, 30}, { 7, 31}, { 8,  1}, { 8,  2}, { 8,  3}, { 8,  4}, { 8,  5},
{ 8,  6}, { 8,  7}, { 8,  8}, { 8,  9}, { 8, 10}, { 8, 11}, { 8, 12}, { 8, 13},
{ 8, 14}, { 8, 15}, { 8, 16}, { 8, 17}, { 8, 18}, { 8, 19}, { 8, 20}, { 8, 21},
{ 8, 22}, { 8, 23}, { 8, 24}, { 8, 25}, { 8, 26}, { 8, 27}, { 8, 28}, { 8, 29},
{ 8, 30}, { 9,  1}, { 9,  2}, { 9,  3}, { 9,  4}, { 9,  5}, { 9,  6}, { 9,  7},
{ 9,  8}, { 9,  9}, { 9, 10}, { 9, 11}, { 9, 12}, { 9, 13}, { 9, 14}, { 9, 15},
{ 9, 16}, { 9, 17}, { 9, 18}, { 9, 19}, { 9, 20}, { 9, 21}, { 9, 22}, { 9, 23},
{ 9, 24}, { 9, 25}, { 9, 26}, { 9, 27}, { 9, 28}, { 9, 29}, { 9, 30}, { 9, 31},
{10,  1}, {10,  2}, {10,  3}, {10,  4}, {10,  5}, {10,  6}, {10,  7}, {10,  8},
{10,  9}, {10, 10}, {10, 11}, {10, 12}, {10, 13}, {10, 14}, {10, 15}, {10, 16},
{10, 17}, {10, 18}, {10, 19}, {10, 20}, {10, 21}, {10, 22}, {10, 23}, {10, 24},
{10, 25}, {10, 26}, {10, 27}, {10, 28}, {10, 29}, {10, 30}, {11,  1}, {11,  2},
{11,  3}, {11,  4}, {11,  5}, {11,  6}, {11,  7}, {11,  8}, {11,  9}, {11, 10},
{11, 11}, {11, 12}, {11, 13}, {11, 14}, {11, 15}, {11, 16}, {11, 17}, {11, 18},
{11, 19}, {11, 20}, {11, 21}, {11, 22}, {11, 23}, {11, 24}, {11, 25}, {11, 26},
{11, 27}, {11, 28}, {11, 29}, {11, 30}, {11, 31} };

SuDOY2DM suDoy2DmLeap[] = {
{ 0,  1}, { 0,  2}, { 0,  3}, { 0,  4}, { 0,  5}, { 0,  6}, { 0,  7}, { 0,  8},
{ 0,  9}, { 0, 10}, { 0, 11}, { 0, 12}, { 0, 13}, { 0, 14}, { 0, 15}, { 0, 16},
{ 0, 17}, { 0, 18}, { 0, 19}, { 0, 20}, { 0, 21}, { 0, 22}, { 0, 23}, { 0, 24},
{ 0, 25}, { 0, 26}, { 0, 27}, { 0, 28}, { 0, 29}, { 0, 30}, { 0, 31}, { 1,  1},
{ 1,  2}, { 1,  3}, { 1,  4}, { 1,  5}, { 1,  6}, { 1,  7}, { 1,  8}, { 1,  9},
{ 1, 10}, { 1, 11}, { 1, 12}, { 1, 13}, { 1, 14}, { 1, 15}, { 1, 16}, { 1, 17},
{ 1, 18}, { 1, 19}, { 1, 20}, { 1, 21}, { 1, 22}, { 1, 23}, { 1, 24}, { 1, 25},
{ 1, 26}, { 1, 27}, { 1, 28}, { 1, 29}, { 2,  1}, { 2,  2}, { 2,  3}, { 2,  4},
{ 2,  5}, { 2,  6}, { 2,  7}, { 2,  8}, { 2,  9}, { 2, 10}, { 2, 11}, { 2, 12},
{ 2, 13}, { 2, 14}, { 2, 15}, { 2, 16}, { 2, 17}, { 2, 18}, { 2, 19}, { 2, 20},
{ 2, 21}, { 2, 22}, { 2, 23}, { 2, 24}, { 2, 25}, { 2, 26}, { 2, 27}, { 2, 28},
{ 2, 29}, { 2, 30}, { 2, 31}, { 3,  1}, { 3,  2}, { 3,  3}, { 3,  4}, { 3,  5},
{ 3,  6}, { 3,  7}, { 3,  8}, { 3,  9}, { 3, 10}, { 3, 11}, { 3, 12}, { 3, 13},
{ 3, 14}, { 3, 15}, { 3, 16}, { 3, 17}, { 3, 18}, { 3, 19}, { 3, 20}, { 3, 21},
{ 3, 22}, { 3, 23}, { 3, 24}, { 3, 25}, { 3, 26}, { 3, 27}, { 3, 28}, { 3, 29},
{ 3, 30}, { 4,  1}, { 4,  2}, { 4,  3}, { 4,  4}, { 4,  5}, { 4,  6}, { 4,  7},
{ 4,  8}, { 4,  9}, { 4, 10}, { 4, 11}, { 4, 12}, { 4, 13}, { 4, 14}, { 4, 15},
{ 4, 16}, { 4, 17}, { 4, 18}, { 4, 19}, { 4, 20}, { 4, 21}, { 4, 22}, { 4, 23},
{ 4, 24}, { 4, 25}, { 4, 26}, { 4, 27}, { 4, 28}, { 4, 29}, { 4, 30}, { 4, 31},
{ 5,  1}, { 5,  2}, { 5,  3}, { 5,  4}, { 5,  5}, { 5,  6}, { 5,  7}, { 5,  8},
{ 5,  9}, { 5, 10}, { 5, 11}, { 5, 12}, { 5, 13}, { 5, 14}, { 5, 15}, { 5, 16},
{ 5, 17}, { 5, 18}, { 5, 19}, { 5, 20}, { 5, 21}, { 5, 22}, { 5, 23}, { 5, 24},
{ 5, 25}, { 5, 26}, { 5, 27}, { 5, 28}, { 5, 29}, { 5, 30}, { 6,  1}, { 6,  2},
{ 6,  3}, { 6,  4}, { 6,  5}, { 6,  6}, { 6,  7}, { 6,  8}, { 6,  9}, { 6, 10},
{ 6, 11}, { 6, 12}, { 6, 13}, { 6, 14}, { 6, 15}, { 6, 16}, { 6, 17}, { 6, 18},
{ 6, 19}, { 6, 20}, { 6, 21}, { 6, 22}, { 6, 23}, { 6, 24}, { 6, 25}, { 6, 26},
{ 6, 27}, { 6, 28}, { 6, 29}, { 6, 30}, { 6, 31}, { 7,  1}, { 7,  2}, { 7,  3},
{ 7,  4}, { 7,  5}, { 7,  6}, { 7,  7}, { 7,  8}, { 7,  9}, { 7, 10}, { 7, 11},
{ 7, 12}, { 7, 13}, { 7, 14}, { 7, 15}, { 7, 16}, { 7, 17}, { 7, 18}, { 7, 19},
{ 7, 20}, { 7, 21}, { 7, 22}, { 7, 23}, { 7, 24}, { 7, 25}, { 7, 26}, { 7, 27},
{ 7, 28}, { 7, 29}, { 7, 30}, { 7, 31}, { 8,  1}, { 8,  2}, { 8,  3}, { 8,  4},
{ 8,  5}, { 8,  6}, { 8,  7}, { 8,  8}, { 8,  9}, { 8, 10}, { 8, 11}, { 8, 12},
{ 8, 13}, { 8, 14}, { 8, 15}, { 8, 16}, { 8, 17}, { 8, 18}, { 8, 19}, { 8, 20},
{ 8, 21}, { 8, 22}, { 8, 23}, { 8, 24}, { 8, 25}, { 8, 26}, { 8, 27}, { 8, 28},
{ 8, 29}, { 8, 30}, { 9,  1}, { 9,  2}, { 9,  3}, { 9,  4}, { 9,  5}, { 9,  6},
{ 9,  7}, { 9,  8}, { 9,  9}, { 9, 10}, { 9, 11}, { 9, 12}, { 9, 13}, { 9, 14},
{ 9, 15}, { 9, 16}, { 9, 17}, { 9, 18}, { 9, 19}, { 9, 20}, { 9, 21}, { 9, 22},
{ 9, 23}, { 9, 24}, { 9, 25}, { 9, 26}, { 9, 27}, { 9, 28}, { 9, 29}, { 9, 30},
{ 9, 31}, {10,  1}, {10,  2}, {10,  3}, {10,  4}, {10,  5}, {10,  6}, {10,  7},
{10,  8}, {10,  9}, {10, 10}, {10, 11}, {10, 12}, {10, 13}, {10, 14}, {10, 15},
{10, 16}, {10, 17}, {10, 18}, {10, 19}, {10, 20}, {10, 21}, {10, 22}, {10, 23},
{10, 24}, {10, 25}, {10, 26}, {10, 27}, {10, 28}, {10, 29}, {10, 30}, {11,  1},
{11,  2}, {11,  3}, {11,  4}, {11,  5}, {11,  6}, {11,  7}, {11,  8}, {11,  9},
{11, 10}, {11, 11}, {11, 12}, {11, 13}, {11, 14}, {11, 15}, {11, 16}, {11, 17},
{11, 18}, {11, 19}, {11, 20}, {11, 21}, {11, 22}, {11, 23}, {11, 24}, {11, 25},
{11, 26}, {11, 27}, {11, 28}, {11, 29}, {11, 30}, {11, 31} };


// Table of last day number of each month
// static int  aiLastDayOfMonthNormal[14] =
//   { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365, 1000 };

// static int  aiLastDayOfMonthLeapYear[14] =
//   { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366, 1000 };



/*
 * Function Declaration
 * --------------------
 */


/* ======================================================================= */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Decode_TimeF1(SuI106Ch10Header  * psuHeader,
                         void              * pvBuff,
                         SuIrigTimeF1      * psuTime)
    {
//    time_t                lTime;
    struct tm             suTmTime;
//    struct timeval        suTvTime;
    SuTimeF1_ChanSpec   * psuChanSpecTime;
    SuTime_MsgDmyFmt    * psuTimeDmy;
    SuTime_MsgDayFmt    * psuTimeDay;

    psuChanSpecTime = pvBuff;

    // Time in Day format
    if (psuChanSpecTime->uDateFmt == 0)
        {
        // Make 
        psuTimeDay = (SuTime_MsgDayFmt *)((char *)pvBuff + sizeof(SuTimeF1_ChanSpec));
        suTmTime.tm_sec   = psuTimeDay->uTSn *  10 + psuTimeDay->uSn;
        suTmTime.tm_min   = psuTimeDay->uTMn *  10 + psuTimeDay->uMn;
        suTmTime.tm_hour  = psuTimeDay->uTHn *  10 + psuTimeDay->uHn;
        suTmTime.tm_yday  = psuTimeDay->uHDn * 100 + psuTimeDay->uTDn * 10 + psuTimeDay->uDn - 1;
        suTmTime.tm_mday  = suDoy2DmNormal[suTmTime.tm_yday].iDay;
        suTmTime.tm_mon   = suDoy2DmNormal[suTmTime.tm_yday].iMonth;
        suTmTime.tm_year  = 70;  // i.e. 1970
        suTmTime.tm_isdst = 0;
        psuTime->ulSecs   = mktime(&suTmTime);
        psuTime->ulFrac   = psuTimeDay->uHmn * 1000000L + psuTimeDay->uTmn * 100000L;
        }

    // Time in DMY format
    else
        {
        psuTimeDmy = (SuTime_MsgDmyFmt *)((char *)pvBuff + sizeof(SuTimeF1_ChanSpec));
        suTmTime.tm_sec   = psuTimeDmy->uTSn *   10 + psuTimeDmy->uSn;
        suTmTime.tm_min   = psuTimeDmy->uTMn *   10 + psuTimeDmy->uMn;
        suTmTime.tm_hour  = psuTimeDmy->uTHn *   10 + psuTimeDmy->uHn;
        suTmTime.tm_yday  = 0;
        suTmTime.tm_mday  = psuTimeDmy->uTDn *   10 + psuTimeDmy->uDn;
        suTmTime.tm_mon   = psuTimeDmy->uTOn *   10 + psuTimeDmy->uOn;
        suTmTime.tm_year  = psuTimeDmy->uOYn * 1000 + psuTimeDmy->uHYn * 100 + 
                            psuTimeDmy->uTYn *   10 + psuTimeDmy->uYn;
        suTmTime.tm_isdst = 0;
        psuTime->ulSecs   = mktime(&suTmTime);
        psuTime->ulFrac   = psuTimeDmy->uHmn * 1000000L + psuTimeDmy->uTmn * 100000L;
        }

    // Update the current reference time value
    m_suCurrRefTime.suIrigTime.ulSecs = psuTime->ulSecs;
    m_suCurrRefTime.suIrigTime.ulFrac = psuTime->ulFrac;
    m_suCurrRefTime.uRelTime = 0;
    memcpy((char *)&m_suCurrRefTime.uRelTime, (char *)&psuHeader->aubyRefTime[0], 6);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

// Take a 6 byte relative time value (like the one in the IRIG header) and
// turn it into a real time based on the most recent IRIG time message.

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Rel2IrigTime(uint8_t          abyRelTime[],
                        SuIrigTimeF1   * psuTime)
    {
    uint64_t        uRelTime;
    int64_t         uTimeDiff;
    int64_t         lFracDiff;
    int64_t         lSecDiff;
    int64_t         lFrac;

    uRelTime = 0L;
    memcpy(&uRelTime, &abyRelTime[0], 6);

    // Figure out the relative time difference
    uTimeDiff = uRelTime - m_suCurrRefTime.uRelTime;
//    lFracDiff = m_suCurrRefTime.suIrigTime.ulFrac 
    lSecDiff  = uTimeDiff / 10000000;
    lFracDiff = uTimeDiff % 10000000;
    lFrac     = m_suCurrRefTime.suIrigTime.ulFrac + lFracDiff;
    if (lFrac < 0)
        {
        lFrac     += 10000000;
        lSecDiff  -= 1;
        }

    // Now add the time difference to the last IRIG time reference
    psuTime->ulFrac = (unsigned long)lFrac;
    psuTime->ulSecs = (unsigned long)(m_suCurrRefTime.suIrigTime.ulSecs + lSecDiff);

    return I106_OK;
    }



/* --------------------------------------------------------------------------

  This function returns the day of the year that corresponds to the date in
  the input parameter dt.

 ------------------------------------------------------------------------ */
/*
static int iDay_Of_Year(struct date *ptDate)
{
  int        *paiLastDayOfMonth;

  // Figure out leap year
  if ((ptDate->da_year % 4) != 0) paiLastDayOfMonth = aiLastDayOfMonthNormal;
  else                            paiLastDayOfMonth = aiLastDayOfMonthLeapYear;

  return paiLastDayOfMonth[ptDate->da_mon-1] + ptDate->da_day;

}
*/


/* ------------------------------------------------------------------------ */

/* Make the table that maps day of year to month and day for the
 * given year. All values are the 'struct tm' notion.  That is,
 * iMonth = 0-11, iYear is years since 1900, and the index to atDM[]
 * is the day of the year 0-365 (days since 1 Jan).
 */

/*
void iInit_DOY2DMY(int iYear)
  {
  struct tm  tTimetm;
  struct tm *ptTimetm;
  time_t     lStart;
  time_t     lCurrTime;
  int        iDOY;

  // Get the time for noon on Jan 1st.
  tTimetm.tm_year = iYear-1900;
  tTimetm.tm_mon  = 0;
  tTimetm.tm_mday = 1;
  tTimetm.tm_hour = 12;
  tTimetm.tm_min  = 0;
  tTimetm.tm_sec  = 0;
//  tTimetm.tm_gmtoff = 0;
  tTimetm.tm_isdst  = 0;
  lStart = mktime(&tTimetm);

  // Now step through all the days of our lives, er, days of the year
  for (iDOY=0; iDOY<366; iDOY++)
    {
    lCurrTime = lStart + (iDOY * 60*60*24);
    ptTimetm  = gmtime(&lCurrTime);

printf("{%2d, %2d}, ", 
    ptTimetm->tm_mon+1, ptTimetm->tm_mday,  iDOY);

if ((iDOY % 8) == 7) printf("\n");

//    tDOY2DMY.atDM[iDOY].iDay   = ptTimetm->tm_mday;
//    tDOY2DMY.atDM[iDOY].iMonth = ptTimetm->tm_mon;
    }


//  tDOY2DMY.iYear = iYear;

  return;
  }
*/

