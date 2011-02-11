/****************************************************************************

 i106_time.c - 

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>

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


/*
 * Module data
 * -----------
 */

static SuTimeRef    m_asuTimeRef[MAX_HANDLES]; // Relative / absolute time reference

/*
 * Function Declaration
 * --------------------
 */


/* ----------------------------------------------------------------------- */

// Update the current reference time value
EnI106Status I106_CALL_DECL 
    enI106_SetRelTime(int              iI106Ch10Handle,
                      SuIrig106Time  * psuTime,
                      uint8_t          abyRelTime[])
    {

    // Save the absolute time value
    m_asuTimeRef[iI106Ch10Handle].suIrigTime.ulSecs = psuTime->ulSecs;
    m_asuTimeRef[iI106Ch10Handle].suIrigTime.ulFrac = psuTime->ulFrac;
    m_asuTimeRef[iI106Ch10Handle].suIrigTime.enFmt  = psuTime->enFmt;

    // Save the relative (i.e. the 10MHz counter) value
    m_asuTimeRef[iI106Ch10Handle].uRelTime          = 0;
    memcpy((char *)&(m_asuTimeRef[iI106Ch10Handle].uRelTime), 
           (char *)&abyRelTime[0], 6);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

// Take a 6 byte relative time value (like the one in the IRIG header) and
// turn it into a real time based on the current reference IRIG time.

EnI106Status I106_CALL_DECL 
    enI106_Rel2IrigTime(int              iI106Ch10Handle,
                        uint8_t          abyRelTime[],
                        SuIrig106Time  * psuTime)
    {
    int64_t         llRelTime;
    EnI106Status    enStatus;

    // Convert 6 byte time array to 16 bit int.  This only works for 
    // positive time, but that shouldn't be a problem
    llRelTime = 0L;
    memcpy(&llRelTime, &abyRelTime[0], 6);

    enStatus = enI106_RelInt2IrigTime(iI106Ch10Handle, llRelTime, psuTime);

    return enStatus;
    }




/* ----------------------------------------------------------------------- */

// Take a 64 bit relative time value and turn it into a real time based on 
// the current reference IRIG time.

EnI106Status I106_CALL_DECL 
    enI106_RelInt2IrigTime(int               iI106Ch10Handle,
                           int64_t           llRelTime,
                           SuIrig106Time   * psuTime)
    {
    int64_t         uTimeDiff;
    int64_t         lFracDiff;
    int64_t         lSecDiff;

    int64_t         lSec;
    int64_t         lFrac;


    // Figure out the relative time difference
    uTimeDiff = llRelTime - m_asuTimeRef[iI106Ch10Handle].uRelTime;
    lSecDiff  = uTimeDiff / 10000000;
    lFracDiff = uTimeDiff % 10000000;

    lSec      = m_asuTimeRef[iI106Ch10Handle].suIrigTime.ulSecs + lSecDiff;
    lFrac     = m_asuTimeRef[iI106Ch10Handle].suIrigTime.ulFrac + lFracDiff;

    // This seems a bit extreme but it's defensive programming
    while (lFrac < 0)
        {
        lFrac += 10000000;
        lSec  -= 1;
        }
        
    while (lFrac >= 10000000)
        {
        lFrac -= 10000000;
        lSec  += 1;
        }

    // Now add the time difference to the last IRIG time reference
    psuTime->ulFrac = (unsigned long)lFrac;
    psuTime->ulSecs = (unsigned long)lSec;
    psuTime->enFmt  = m_asuTimeRef[iI106Ch10Handle].suIrigTime.enFmt;

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

// Take a real clock time and turn it into a 6 byte relative time.

EnI106Status I106_CALL_DECL 
    enI106_Irig2RelTime(int              iI106Ch10Handle,
                        SuIrig106Time  * psuTime,
                        uint8_t          abyRelTime[])
    {
    int64_t         llDiff;
    int64_t         llNewRel;

    // Calculate time difference (LSB = 100 nSec) between the passed time 
    // and the time reference
    llDiff = 
         (int64_t)(+ psuTime->ulSecs - m_asuTimeRef[iI106Ch10Handle].suIrigTime.ulSecs) * 10000000 +
         (int64_t)(+ psuTime->ulFrac - m_asuTimeRef[iI106Ch10Handle].suIrigTime.ulFrac);

    // Add this amount to the reference 
    llNewRel = m_asuTimeRef[iI106Ch10Handle].uRelTime + llDiff;

    // Now convert this to a 6 byte relative time
    memcpy((char *)&abyRelTime[0],
           (char *)&(llNewRel), 6);

    return I106_OK;
    }

/* ----------------------------------------------------------------------- */

// Take a Irig Ch4 time value (like the one in a secondary IRIG header) and
// turn it into an Irig106 time

EnI106Status I106_CALL_DECL 
    enI106_Ch4Binary2IrigTime(SuI106Ch4_Binary_Time * psuCh4BinaryTime,
                              SuIrig106Time         * psuIrig106Time)
{
    psuIrig106Time->ulSecs = (unsigned long)
        ( (double)psuCh4BinaryTime->uHighBinTime * CH4BINARYTIME_HIGH_LSB_SEC
        + (unsigned long)psuCh4BinaryTime->uLowBinTime * CH4BINARYTIME_LOW_LSB_SEC );
    psuIrig106Time->ulFrac = (unsigned long)psuCh4BinaryTime->uUSecs * _100_NANO_SEC_IN_MICRO_SEC;

    return I106_OK;
}

/* ----------------------------------------------------------------------- */

// Take a IEEE-1588 time value (like the one in a secondary IRIG header) and
// turn it into an Irig106 time
EnI106Status I106_CALL_DECL 
    enI106_IEEE15882IrigTime(SuIEEE1588_Time * psuIEEE1588Time,
                              SuIrig106Time  * psuIrig106Time)
{
    psuIrig106Time->ulSecs = (unsigned long)psuIEEE1588Time->uSeconds;
    //Convert 'nanoseconds' to '100 nanoseconds'
    psuIrig106Time->ulFrac = (unsigned long)psuIEEE1588Time->uNanoSeconds/100;     

    return I106_OK;
}


/* ------------------------------------------------------------------------ */

// Warning - array to int / int to array functions are little endian only!

// Create a 6 byte array value from a 64 bit int relative time

void I106_CALL_DECL 
    vLLInt2TimeArray(int64_t * pllRelTime,
                     uint8_t   abyRelTime[])
    {
    memcpy((char *)abyRelTime, (char *)pllRelTime, 6);
    return;
    }


/* ------------------------------------------------------------------------ */

// Create a 64 bit int relative time from 6 byte array value

void I106_CALL_DECL 
    vTimeArray2LLInt(uint8_t   abyRelTime[],
                     int64_t * pllRelTime)
    {
    *pllRelTime = 0L;
    memcpy((char *)pllRelTime, (char *)abyRelTime, 6);
    return;
    }



/* ------------------------------------------------------------------------ */

// Read the data file from the current position to try to determine a valid 
// relative time to clock time from a time packet.

EnI106Status I106_CALL_DECL 
    enI106_SyncTime(int     iI106Ch10Handle,
                    int     bRequireSync,   // Require external time source sync
                    int     iTimeLimit)     // Max time to look in seconds
    {
    int64_t             llCurrOffset;
    int64_t             llTimeLimit;
    int64_t             llCurrTime;
    EnI106Status        enStatus;
    EnI106Status        enRetStatus;
    SuI106Ch10Header    suI106Hdr;
    SuIrig106Time       suTime;
    unsigned long       ulBuffSize = 0;
    void              * pvBuff = NULL;
    SuTimeF1_ChanSpec * psuChanSpecTime = NULL;

    // Get and save the current file position
    enStatus = enI106Ch10GetPos(iI106Ch10Handle, &llCurrOffset);
    if (enStatus != I106_OK)
        return enStatus;

    // Set the file to the start
    //enStatus = enI106Ch10SetPos(iI106Ch10Handle, 0);
    //if (enStatus != I106_OK)
    //    return enStatus;

    // Read the next header
    enStatus = enI106Ch10ReadNextHeaderFile(iI106Ch10Handle, &suI106Hdr);
    if (enStatus == I106_EOF)
        return I106_TIME_NOT_FOUND;

    if (enStatus != I106_OK)
        return enStatus;

    // Calculate the time limit if there is one
    if (iTimeLimit > 0)
        {
        vTimeArray2LLInt(suI106Hdr.aubyRefTime, &llTimeLimit);
        llTimeLimit = llTimeLimit + (int64_t)iTimeLimit * (int64_t)10000000;
        }
    else
        llTimeLimit = 0;

    // Loop, looking for appropriate time message
    while (bTRUE)
        {

        // See if we've passed our time limit
        if (llTimeLimit > 0)
            {
            vTimeArray2LLInt(suI106Hdr.aubyRefTime, &llCurrTime);
            if (llTimeLimit < llCurrTime)
                {
                enRetStatus = I106_TIME_NOT_FOUND;
                break;
                }
            } // end if there is a time limit

        // If IRIG time type then process it
        if (suI106Hdr.ubyDataType == I106CH10_DTYPE_IRIG_TIME)
            {

            // Read header OK, make buffer for time message
            if (ulBuffSize < suI106Hdr.ulPacketLen)
                {
                pvBuff          = realloc(pvBuff, suI106Hdr.ulPacketLen);
                psuChanSpecTime = (SuTimeF1_ChanSpec *)pvBuff;
                ulBuffSize      = suI106Hdr.ulPacketLen;
                }

            // Read the data buffer
            enStatus = enI106Ch10ReadData(iI106Ch10Handle, ulBuffSize, pvBuff);
            if (enStatus != I106_OK)
                {
                enRetStatus = I106_TIME_NOT_FOUND;
                break;
                }

            // If external sync OK then decode it and set relative time
            if ((bRequireSync == bFALSE) || (psuChanSpecTime->uTimeSrc == 1))
                {
                enI106_Decode_TimeF1(&suI106Hdr, pvBuff, &suTime);
                enI106_SetRelTime(iI106Ch10Handle, &suTime, suI106Hdr.aubyRefTime);
                enRetStatus = I106_OK;
                break;
                }
            } // end if IRIG time message

        // Read the next header and try again
        enStatus = enI106Ch10ReadNextHeaderFile(iI106Ch10Handle, &suI106Hdr);
        if (enStatus == I106_EOF)
            {
            enRetStatus = I106_TIME_NOT_FOUND;
            break;
            }

        if (enStatus != I106_OK)
            {
            enRetStatus = enStatus;
            break;
            }

        } // end while looping looking for time message

    // Restore file position
    enStatus = enI106Ch10SetPos(iI106Ch10Handle, llCurrOffset);
    if (enStatus != I106_OK)
        {
        enRetStatus = enStatus;
        }

    // Return the malloc'ed memory
    free(pvBuff);

    return enRetStatus;
    }


/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106Ch10SetPosToIrigTime(int iHandle, SuIrig106Time * psuSeekTime)
    {
    uint8_t             abySeekTime[6];
    int64_t             llSeekTime;
    SuInOrderIndex    * psuIndex = &g_suI106Handle[iHandle].suInOrderIndex;
    int                 iUpperLimit;
    int                 iLowerLimit;
    int                 iSearchLoopIdx;

    // If there is no index in memory then barf
    if (psuIndex->enSortStatus != enSorted)
        return I106_NO_INDEX;

    // We have an index so do a binary search for time

    // Convert clock time to 10 MHz count
    enI106_Irig2RelTime(iHandle, psuSeekTime, abySeekTime);
    vTimeArray2LLInt(abySeekTime, &llSeekTime);

    // Check time bounds
    if (llSeekTime < psuIndex->asuIndex[0].llTime)
        {
        enI106Ch10FirstMsg(iHandle);
        return I106_TIME_NOT_FOUND;
        };

    if (llSeekTime > psuIndex->asuIndex[psuIndex->iArrayUsed].llTime)
        {
        enI106Ch10LastMsg(iHandle);
        return I106_TIME_NOT_FOUND;
        };

    // If we don't already have it, figure out how many search steps
    if (psuIndex->iNumSearchSteps == 0)
        {
        iUpperLimit = 1;
        while (iUpperLimit < psuIndex->iArrayUsed)
            {
            iUpperLimit *= 2;
            psuIndex->iNumSearchSteps++;
            }
        } // end if no search steps

    // Loop prescribed number of times
    iLowerLimit = 0;
    iUpperLimit = psuIndex->iArrayUsed-1;
    psuIndex->iArrayCurr = (iUpperLimit - iLowerLimit) / 2;
    for (iSearchLoopIdx = 0; 
         iSearchLoopIdx < psuIndex->iNumSearchSteps; 
         iSearchLoopIdx++)
        {
        if      (psuIndex->asuIndex[psuIndex->iArrayCurr].llTime > llSeekTime)
            iUpperLimit = (iUpperLimit - iLowerLimit) / 2;
        else if (psuIndex->asuIndex[psuIndex->iArrayCurr].llTime < llSeekTime)
            iLowerLimit = (iUpperLimit - iLowerLimit) / 2;
        else
            break;
        }

    return I106_OK;
    }


/* ------------------------------------------------------------------------ */

// General purpose time utilities
// ------------------------------

// Convert IRIG time into an appropriate string

char * IrigTime2String(SuIrig106Time * psuTime)
    {
    static char     szTime[30];
    struct tm     * psuTmTime;

    // Convert IRIG time into it's components
    psuTmTime = gmtime((time_t *)&(psuTime->ulSecs));

    // Make the appropriate string
    switch (psuTime->enFmt)
        {
        // Year / Month / Day format ("2008/02/29 12:34:56.789")
        case I106_DATEFMT_DMY :
            sprintf(szTime, "%4.4i/%2.2i/%2.2i %2.2i:%2.2i:%2.2i.%3.3i",
                psuTmTime->tm_year + 1900,
                psuTmTime->tm_mon + 1,
                psuTmTime->tm_mday,
                psuTmTime->tm_hour,
                psuTmTime->tm_min,
                psuTmTime->tm_sec,
                psuTime->ulFrac / 10000);
            break;

        // Day of the Year format ("001:12:34:56.789")
        case I106_DATEFMT_DAY :
        default :
            sprintf(szTime, "%3.3i:%2.2i:%2.2i:%2.2i.%3.3i",
                psuTmTime->tm_yday+1,
                psuTmTime->tm_hour,
                psuTmTime->tm_min,
                psuTmTime->tm_sec,
                psuTime->ulFrac / 10000);
            break;
        } // end switch on format

    return szTime;
    }



/* ------------------------------------------------------------------------ */

// This function fills in the SuTimeRef structure with the "best" relative 
// and/or absolute time stamp available from the packet header and intra-packet 
// header (if available).

EnI106Status I106_CALL_DECL
    vFillInTimeStruct(SuI106Ch10Header * psuHeader,
                      SuIntraPacketTS  * psuIntraPacketTS, 
                      SuTimeRef        * psuTimeRef)
    {
    int iSecHdrTimeFmt;

    // Get the secondary header time format
    iSecHdrTimeFmt = psuHeader->ubyPacketFlags & I106CH10_PFLAGS_TIMEFMT_MASK;
    psuTimeRef->bRelTimeValid = bFALSE;
    psuTimeRef->bAbsTimeValid = bFALSE;
    
    // Set the relative time from the packet header
    vTimeArray2LLInt(psuHeader->aubyRefTime, &(psuTimeRef->uRelTime));
    psuTimeRef->bRelTimeValid = bTRUE;

    // If secondary header is available, use that time for absolute
    if ((psuHeader->ubyPacketFlags & I106CH10_PFLAGS_SEC_HEADER) != 0)
        {
        switch(iSecHdrTimeFmt)
            {
            case I106CH10_PFLAGS_TIMEFMT_IRIG106:
                enI106_Ch4Binary2IrigTime((SuI106Ch4_Binary_Time *)psuHeader->aulTime, &(psuTimeRef->suIrigTime));              
                psuTimeRef->bAbsTimeValid = bTRUE;
                break;
            case I106CH10_PFLAGS_TIMEFMT_IEEE1588:
                enI106_IEEE15882IrigTime((SuIEEE1588_Time *)psuHeader->aulTime, &(psuTimeRef->suIrigTime));
                psuTimeRef->bAbsTimeValid = bTRUE;
                break;
            default:
                //Currently reserved, should we have a default way to decode?
                break;
            } // end switch on secondary header time format
        } // end if 

    // Now process values from the intra-packet headers if available
    if (psuIntraPacketTS != NULL)
        {
        // If relative time
        if ((psuHeader->ubyPacketFlags & I106CH10_PFLAGS_IPTIMESRC) == 0)
            {
            vTimeArray2LLInt(psuIntraPacketTS->aubyIntPktTime, &(psuTimeRef->uRelTime));
            psuTimeRef->bRelTimeValid = bTRUE;
            }

        // else is absolute time
        else
            {
            switch(iSecHdrTimeFmt)
                {
                case I106CH10_PFLAGS_TIMEFMT_IRIG106:
                    enI106_Ch4Binary2IrigTime((SuI106Ch4_Binary_Time *)psuIntraPacketTS, &(psuTimeRef->suIrigTime));
                    psuTimeRef->bAbsTimeValid = bTRUE;
                    break;
                case I106CH10_PFLAGS_TIMEFMT_IEEE1588:                  
                    enI106_IEEE15882IrigTime((SuIEEE1588_Time *)psuIntraPacketTS, &(psuTimeRef->suIrigTime));
                    psuTimeRef->bAbsTimeValid = bTRUE;
                    break;
                default:
                    //Current reserved, should we have a default way to decode
                    break;
                } // end switch on intra-packet time format
            } // end else absolute time
        } // end if intra-packet time stamp exists
    
    return I106_OK;
    }



/* ------------------------------------------------------------------------ */

/* Return the equivalent in seconds past 12:00:00 a.m. Jan 1, 1970 GMT
   of the Greenwich Mean time and date in the exploded time structure `tm'.

   The standard mktime() has the annoying "feature" of assuming that the 
   time in the tm structure is local time, and that it has to be corrected 
   for local time zone.  In this library time is assumed to be UTC and UTC
   only.  To make sure no timezone correction is applied this time conversion
   routine was lifted from the standard C run time library source.  Interestingly
   enough, this routine was found in the source for mktime().

   This function does always put back normalized values into the `tm' struct,
   parameter, including the calculated numbers for `tm->tm_yday',
   `tm->tm_wday', and `tm->tm_isdst'.

   Returns -1 if the time in the `tm' parameter cannot be represented
   as valid `time_t' number. 
 */

// Number of leap years from 1970 to `y' (not including `y' itself).
#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)

// Nonzero if `y' is a leap year, else zero.
#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

// Additional leapday in February of leap years.
#define leapday(m, y) ((m) == 1 && leap (y))

#define ADJUST_TM(tm_member, tm_carry, modulus) \
  if ((tm_member) < 0) { \
    tm_carry -= (1 - ((tm_member)+1) / (modulus)); \
    tm_member = (modulus-1) + (((tm_member)+1) % (modulus)); \
  } else if ((tm_member) >= (modulus)) { \
    tm_carry += (tm_member) / (modulus); \
    tm_member = (tm_member) % (modulus); \
  }

// Length of month `m' (0 .. 11)
#define monthlen(m, y) (ydays[(m)+1] - ydays[m] + leapday (m, y))


uint32_t I106_CALL_DECL mkgmtime(struct tm * psuTmTime)
    {

    // Accumulated number of days from 01-Jan up to start of current month.
    static short ydays[] =
    {
      0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
    };

    int years, months, days, hours, minutes, seconds;

    years   = psuTmTime->tm_year + 1900;  // year - 1900 -> year
    months  = psuTmTime->tm_mon;          // 0..11
    days    = psuTmTime->tm_mday - 1;     // 1..31 -> 0..30
    hours   = psuTmTime->tm_hour;         // 0..23
    minutes = psuTmTime->tm_min;          // 0..59
    seconds = psuTmTime->tm_sec;          // 0..61 in ANSI C.

    ADJUST_TM(seconds, minutes, 60)
    ADJUST_TM(minutes, hours,   60)
    ADJUST_TM(hours,   days,    24)
    ADJUST_TM(months,  years,   12)

    if (days < 0)
        do 
            {
            if (--months < 0) 
                {
                --years;
                months = 11;
                }
            days += monthlen(months, years);
            } while (days < 0);

    else
        while (days >= monthlen(months, years)) 
            {
            days -= monthlen(months, years);
            if (++months >= 12) 
                {
                ++years;
                months = 0;
                }
            } // end while

    // Restore adjusted values in tm structure
    psuTmTime->tm_year = years - 1900;
    psuTmTime->tm_mon  = months;
    psuTmTime->tm_mday = days + 1;
    psuTmTime->tm_hour = hours;
    psuTmTime->tm_min  = minutes;
    psuTmTime->tm_sec  = seconds;

    // Set `days' to the number of days into the year.
    days += ydays[months] + (months > 1 && leap (years));
    psuTmTime->tm_yday = days;

    // Now calculate `days' to the number of days since Jan 1, 1970.
    days = (unsigned)days + 365 * (unsigned)(years - 1970) +
           (unsigned)(nleap (years));
    psuTmTime->tm_wday = ((unsigned)days + 4) % 7; /* Jan 1, 1970 was Thursday. */
    psuTmTime->tm_isdst = 0;

    if (years < 1970)
        return (uint32_t)-1;

    return (uint32_t)(86400L * days  + 3600L * hours + 60L * minutes + seconds);
    }





#ifdef __cplusplus
} // end namespace i106
#endif

