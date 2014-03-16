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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#if defined(_WIN32)
#include <windows.h>        // For FILETIME
#endif

#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_decode_time.h"

#ifdef __cplusplus
namespace Irig106 {
#endif

/*
 * Macros and definitions
 * ----------------------
 */


/*
 * Data structures
 * ---------------
 */

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
// SuTimeRef   m_suCurrRefTime;           // Current value of IRIG reference time


// These structures are used to convert from day of the year format to 
// day and month.  One is for normal years and the other is for leap years.
// The values and index are of the "struct tm" notion.  That is, the day of 
// the year index is number of days since Jan 1st, i.e. Jan 1st = 0.  For
// IRIG time, Jan 1st = 1.  The month value is months since January, i.e. 
// Jan = 0.  Don't get confused!

SuDOY2DM suDoy2DmNormal[] = {
{ 0,  0}, // This is to handle the special case where IRIG DoY is incorrectly set to 000
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
{ 0,  0}, // This is to handle the special case where IRIG DoY is incorrectly set to 000
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



/*
 * Function Declaration
 * --------------------
 */


/* ======================================================================= */

// Take an IRIG F1 time packet and decode it into something we can use

EnI106Status I106_CALL_DECL 
    enI106_Decode_TimeF1(SuI106Ch10Header  * psuHeader,
                         void              * pvBuff,
                         SuIrig106Time     * psuTime)
    {
    SuTimeF1_ChanSpec   * psuChanSpecTime;
    void                * pvTimeBuff;

    psuChanSpecTime = (SuTimeF1_ChanSpec *)pvBuff;
    pvTimeBuff      = (char *)pvBuff + sizeof(SuTimeF1_ChanSpec);

    enI106_Decode_TimeF1_Buff(psuChanSpecTime->uDateFmt, psuChanSpecTime->bLeapYear, pvTimeBuff, psuTime);

    return I106_OK;
    }


/* ---------------------------------------------------------------------- */

void I106_CALL_DECL 
    enI106_Decode_TimeF1_Buff(int                 iDateFmt,
                              int                 bLeapYear,
                              void              * pvTimeBuff,
                              SuIrig106Time     * psuTime)
    {
    struct tm             suTmTime;
    SuTime_MsgDmyFmt    * psuTimeDmy;
    SuTime_MsgDayFmt    * psuTimeDay;

    if (iDateFmt == 0)
        {
        // Make time
        psuTimeDay = (SuTime_MsgDayFmt *)pvTimeBuff;
        suTmTime.tm_sec   = psuTimeDay->uTSn *  10 + psuTimeDay->uSn;
        suTmTime.tm_min   = psuTimeDay->uTMn *  10 + psuTimeDay->uMn;
        suTmTime.tm_hour  = psuTimeDay->uTHn *  10 + psuTimeDay->uHn;

        // Legal IRIG DoY numbers are from 1 to 365 (366 for leap year). Some vendors however
        // will use 000 for DoY.  Not legal but there it is.
        suTmTime.tm_yday  = psuTimeDay->uHDn * 100 + psuTimeDay->uTDn * 10 + psuTimeDay->uDn;

        // Make day
        if (bLeapYear)
            {
            suTmTime.tm_mday  = suDoy2DmLeap[suTmTime.tm_yday].iDay;
            suTmTime.tm_mon   = suDoy2DmLeap[suTmTime.tm_yday].iMonth;
            suTmTime.tm_year  = 72;  // i.e. 1972, a leap year
            }
        else
            {
            suTmTime.tm_mday  = suDoy2DmNormal[suTmTime.tm_yday].iDay;
            suTmTime.tm_mon   = suDoy2DmNormal[suTmTime.tm_yday].iMonth;
            suTmTime.tm_year  = 71;  // i.e. 1971, not a leap year
            }
        suTmTime.tm_isdst = 0;
        psuTime->ulSecs   = mkgmtime(&suTmTime);
        psuTime->ulFrac   = psuTimeDay->uHmn * 1000000L + psuTimeDay->uTmn * 100000L;
        psuTime->enFmt    = I106_DATEFMT_DAY;
        }

    // Time in DMY format
    else
        {
        psuTimeDmy = (SuTime_MsgDmyFmt *)pvTimeBuff;
        suTmTime.tm_sec   = psuTimeDmy->uTSn *   10 + psuTimeDmy->uSn;
        suTmTime.tm_min   = psuTimeDmy->uTMn *   10 + psuTimeDmy->uMn;
        suTmTime.tm_hour  = psuTimeDmy->uTHn *   10 + psuTimeDmy->uHn;
        suTmTime.tm_yday  = 0;
        suTmTime.tm_mday  = psuTimeDmy->uTDn *   10 + psuTimeDmy->uDn;
        suTmTime.tm_mon   = psuTimeDmy->uTOn *   10 + psuTimeDmy->uOn - 1;
        suTmTime.tm_year  = psuTimeDmy->uOYn * 1000 + psuTimeDmy->uHYn * 100 + 
                            psuTimeDmy->uTYn *   10 + psuTimeDmy->uYn - 1900;
        suTmTime.tm_isdst = 0;
        psuTime->ulSecs   = mkgmtime(&suTmTime);
        psuTime->ulFrac   = psuTimeDmy->uHmn * 1000000L + psuTimeDmy->uTmn * 100000L;
        psuTime->enFmt    = I106_DATEFMT_DMY;
        }

    return;
    }


/* ---------------------------------------------------------------------- */
#if 0
// Decode an IRIG F1 time packet with a user supplied year. This is handy
// for DoY packets that don't include year.

void I106_CALL_DECL 
    enI106_Decode_TimeF1_Buff_wYear
        (int                 iDateFmt,
         int                 iYear,
         void              * pvTimeBuff,
         SuIrig106Time     * psuTime)
    {
    struct tm             suTmTime;
    SuTime_MsgDmyFmt    * psuTimeDmy;
    SuTime_MsgDayFmt    * psuTimeDay;

    if (iDateFmt == 0)
        {
        // Make time
        psuTimeDay = (SuTime_MsgDayFmt *)pvTimeBuff;
        suTmTime.tm_sec   = psuTimeDay->uTSn *  10 + psuTimeDay->uSn;
        suTmTime.tm_min   = psuTimeDay->uTMn *  10 + psuTimeDay->uMn;
        suTmTime.tm_hour  = psuTimeDay->uTHn *  10 + psuTimeDay->uHn;

        // Legal IRIG DoY numbers are from 1 to 365 (366 for leap year). Some vendors however
        // will use 000 for DoY.  Not legal but there it is.
        suTmTime.tm_yday  = psuTimeDay->uHDn * 100 + psuTimeDay->uTDn * 10 + psuTimeDay->uDn;

        // Make day
        if (bLeapYear)
            {
            suTmTime.tm_mday  = suDoy2DmLeap[suTmTime.tm_yday].iDay;
            suTmTime.tm_mon   = suDoy2DmLeap[suTmTime.tm_yday].iMonth;
            suTmTime.tm_year  = 72;  // i.e. 1972, a leap year
            }
        else
            {
            suTmTime.tm_mday  = suDoy2DmNormal[suTmTime.tm_yday].iDay;
            suTmTime.tm_mon   = suDoy2DmNormal[suTmTime.tm_yday].iMonth;
            suTmTime.tm_year  = 71;  // i.e. 1971, not a leap year
            }
        suTmTime.tm_isdst = 0;
        psuTime->ulSecs   = mkgmtime(&suTmTime);
        psuTime->ulFrac   = psuTimeDay->uHmn * 1000000L + psuTimeDay->uTmn * 100000L;
        psuTime->enFmt    = I106_DATEFMT_DAY;
        }

    // Time in DMY format
    else
        {
        psuTimeDmy = (SuTime_MsgDmyFmt *)pvTimeBuff;
        suTmTime.tm_sec   = psuTimeDmy->uTSn *   10 + psuTimeDmy->uSn;
        suTmTime.tm_min   = psuTimeDmy->uTMn *   10 + psuTimeDmy->uMn;
        suTmTime.tm_hour  = psuTimeDmy->uTHn *   10 + psuTimeDmy->uHn;
        suTmTime.tm_yday  = 0;
        suTmTime.tm_mday  = psuTimeDmy->uTDn *   10 + psuTimeDmy->uDn;
        suTmTime.tm_mon   = psuTimeDmy->uTOn *   10 + psuTimeDmy->uOn - 1;
        suTmTime.tm_year  = psuTimeDmy->uOYn * 1000 + psuTimeDmy->uHYn * 100 + 
                            psuTimeDmy->uTYn *   10 + psuTimeDmy->uYn - 1900;
        suTmTime.tm_isdst = 0;
        psuTime->ulSecs   = mkgmtime(&suTmTime);
        psuTime->ulFrac   = psuTimeDmy->uHmn * 1000000L + psuTimeDmy->uTmn * 100000L;
        psuTime->enFmt    = I106_DATEFMT_DMY;
        }

    return;
    }
#endif

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


EnI106Status I106_CALL_DECL 
    enI106_Encode_TimeF1(SuI106Ch10Header  * psuHeader,
                         unsigned int        uTimeSrc,
                         unsigned int        uFmtTime,
                         unsigned int        uFmtDate,
                         SuIrig106Time     * psuTime,
                         void              * pvBuffTimeF1)
    {
    // A temporary integer to decimate to get BCD factors
    uint32_t          uIntDec;
    struct tm       * psuTmTime;

    typedef struct
        {
        SuTimeF1_ChanSpec   suChanSpec;
        union
            {
            SuTime_MsgDayFmt    suDayFmt;
            SuTime_MsgDmyFmt    suDmyFmt;
            } suMsg;
        } SuMsgTimeF1;

    SuMsgTimeF1 * psuTimeF1;

    SuTime_MsgDayFmt   * psuDayFmt;
    SuTime_MsgDmyFmt   * psuDmyFmt;

    // Now, after creating this ubertime-structure above, create a 
    // couple of pointers to make the code below simpler to read.
    psuTimeF1 = (SuMsgTimeF1 *)pvBuffTimeF1;
    psuDayFmt = &(psuTimeF1->suMsg.suDayFmt);
    psuDmyFmt = &(psuTimeF1->suMsg.suDmyFmt);

    // Zero out all the time fields
    memset(psuTimeF1, 0, sizeof(SuTimeF1_ChanSpec));

    // Break time down to DMY HMS
    psuTmTime = gmtime((time_t *)&(psuTime->ulSecs));

    // Make channel specific data word
    psuTimeF1->suChanSpec.uTimeSrc    = uTimeSrc;
    psuTimeF1->suChanSpec.uTimeFmt    = uFmtTime;
    psuTimeF1->suChanSpec.uDateFmt    = uFmtDate;
    if (psuTmTime->tm_year % 4 == 0)
        psuTimeF1->suChanSpec.bLeapYear = 1;
    else
        psuTimeF1->suChanSpec.bLeapYear = 0;

    // Fill in day of year format
    if (uFmtDate == 0)
        {
        // Zero out all the time fields
        memset(psuDayFmt, 0, sizeof(SuTime_MsgDayFmt));

        // Set the various time fields
        uIntDec = psuTime->ulFrac / 100000L;
        psuDayFmt->uTmn = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDayFmt->uHmn) / 10;
        psuDayFmt->uHmn = (uint16_t)(uIntDec  % 10);

        uIntDec = psuTmTime->tm_sec;
        psuDayFmt->uSn  = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDayFmt->uSn)  / 10;
        psuDayFmt->uTSn = (uint16_t)(uIntDec  % 10);

        uIntDec = psuTmTime->tm_min;
        psuDayFmt->uMn  = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDayFmt->uMn)  / 10;
        psuDayFmt->uTMn = (uint16_t)(uIntDec  % 10);

        uIntDec = psuTmTime->tm_hour;
        psuDayFmt->uHn  = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDayFmt->uHn)  / 10;
        psuDayFmt->uTHn = (uint16_t)(uIntDec  % 10);

        uIntDec = psuTmTime->tm_yday + 1;
        psuDayFmt->uDn = (uint16_t)(uIntDec   % 10);
        uIntDec = (uIntDec - psuDayFmt->uDn)  / 10;
        psuDayFmt->uTDn = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDayFmt->uTDn) / 10;
        psuDayFmt->uHDn  = (uint16_t)(uIntDec  % 10);

        // Set the data length in the header
        psuHeader->ulDataLen = 
            sizeof(SuTimeF1_ChanSpec) + sizeof(SuTime_MsgDayFmt);
        }

    // Fill in day, month, year format
    else
        {
        // Zero out all the time fields
        memset(psuDmyFmt, 0, sizeof(SuTime_MsgDmyFmt));

        // Set the various time fields
        uIntDec = psuTime->ulFrac / 100000L;
        psuDmyFmt->uTmn = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDmyFmt->uHmn) / 10;
        psuDmyFmt->uHmn = (uint16_t)(uIntDec  % 10);

        uIntDec = psuTmTime->tm_sec;
        psuDmyFmt->uSn  = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDmyFmt->uSn)  / 10;
        psuDmyFmt->uTSn = (uint16_t)(uIntDec  % 10);

        uIntDec = psuTmTime->tm_min;
        psuDmyFmt->uMn  = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDmyFmt->uMn)  / 10;
        psuDmyFmt->uTMn = (uint16_t)(uIntDec  % 10);

        uIntDec = psuTmTime->tm_hour;
        psuDmyFmt->uHn  = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDmyFmt->uHn)  / 10;
        psuDmyFmt->uTHn = (uint16_t)(uIntDec  % 10);

        uIntDec = psuTmTime->tm_mday;
        psuDmyFmt->uDn  = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDmyFmt->uDn)  / 10;
        psuDmyFmt->uTDn = (uint16_t)(uIntDec  % 10);

        uIntDec = psuTmTime->tm_mon + 1;
        psuDmyFmt->uOn  = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDmyFmt->uOn)  / 10;
        psuDmyFmt->uTOn = (uint16_t)(uIntDec  % 10);

        uIntDec = psuTmTime->tm_year + 1900;
        psuDmyFmt->uYn  = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDmyFmt->uYn)  / 10;
        psuDmyFmt->uTYn = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDmyFmt->uTYn) / 10;
        psuDmyFmt->uHYn = (uint16_t)(uIntDec  % 10);
        uIntDec = (uIntDec - psuDmyFmt->uHYn) / 10;
        psuDmyFmt->uOYn = (uint16_t)(uIntDec  % 10);

        // Set the data length in the header
        psuHeader->ulDataLen = 
            sizeof(SuTimeF1_ChanSpec) + sizeof(SuTime_MsgDmyFmt);
        }

    // Make the data buffer checksum and update the header
    uAddDataFillerChecksum(psuHeader, (unsigned char *)pvBuffTimeF1);

    return I106_OK;
    }

#ifdef __cplusplus
}
#endif
