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

 Created by Bob Baggerman

 $RCSfile: i106_time.c,v $
 $Date: 2006-10-08 16:35:25 $
 $Revision: 1.1 $

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdint.h"
#include "irig106ch10.h"
#include "i106_time.h"

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
I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_SetRelTime(int              iI106Ch10Handle,
                      SuIrig106Time  * psuTime,
                      uint8_t          abyRelTime[])
    {

    // Save the absolute time value
    m_asuTimeRef[iI106Ch10Handle].suIrigTime.ulSecs = psuTime->ulSecs;
    m_asuTimeRef[iI106Ch10Handle].suIrigTime.ulFrac = psuTime->ulFrac;

    // Save the relative (i.e. the 10MHz counter) value
    m_asuTimeRef[iI106Ch10Handle].uRelTime          = 0;
    memcpy((char *)&(m_asuTimeRef[iI106Ch10Handle].uRelTime), 
           (char *)&abyRelTime[0], 6);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

// Take a 6 byte relative time value (like the one in the IRIG header) and
// turn it into a real time based on the current reference IRIG time.

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Rel2IrigTime(int              iI106Ch10Handle,
                        uint8_t          abyRelTime[],
                        SuIrig106Time  * psuTime)
    {
    uint64_t        uRelTime;
    int64_t         uTimeDiff;
    int64_t         lFracDiff;
    int64_t         lSecDiff;
    int64_t         lFrac;

    uRelTime = 0L;
    memcpy(&uRelTime, &abyRelTime[0], 6);

    // Figure out the relative time difference
    uTimeDiff = uRelTime - m_asuTimeRef[iI106Ch10Handle].uRelTime;
//    lFracDiff = m_suCurrRefTime.suIrigTime.ulFrac 
    lSecDiff  = uTimeDiff / 10000000;
    lFracDiff = uTimeDiff % 10000000;
    lFrac     = m_asuTimeRef[iI106Ch10Handle].suIrigTime.ulFrac + lFracDiff;
    if (lFrac < 0)
        {
        lFrac     += 10000000;
        lSecDiff  -= 1;
        }

    // Now add the time difference to the last IRIG time reference
    psuTime->ulFrac = (unsigned long)lFrac;
    psuTime->ulSecs = (unsigned long)(m_asuTimeRef[iI106Ch10Handle].suIrigTime.ulSecs + 
                                      lSecDiff);

    return I106_OK;
    }





/* ----------------------------------------------------------------------- */

// Take a real time and turn it into a 6 byte relative time.

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Irig2RelTime(int              iI106Ch10Handle,
                        SuIrig106Time  * psuTime,
                        uint8_t          abyRelTime[])
    {


    return I106_OK;
    }


