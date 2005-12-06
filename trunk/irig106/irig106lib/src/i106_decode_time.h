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

 Created by Bob Baggerman

 $RCSfile: i106_decode_time.h,v $
 $Date: 2005-12-06 16:36:00 $
 $Revision: 1.3 $

 ****************************************************************************/

#ifndef _I106_DECODE_TIME_H
#define _I106_DECODE_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)


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


/*
 * Data structures
 * ---------------
 */

/* Time Format 1 */

/* Time */
/*
typedef struct
    {
    uint16_t     bCarrier    : 1;        // IRIG time carrier present
    uint16_t     uUnused1    : 3;        // Reserved
    uint16_t     uFormat     : 2;        // IRIG time format
    uint16_t     uUnused2    : 2;        // Reserved
    uint16_t     bLeapYear   : 1;        // Leap year
    uint16_t     uUnused3    : 7;        // Reserved
    uint16_t     uUnused4;
    uint16_t     uUnused5;
    } SuIrigTime;
*/


// Decoded time

// The nice thing about standards is that there are so many to
// choose from, and time is no exception. But none of the various
// C time representations really fill the bill. So I made a new
// time representation.  So there.
typedef struct 
    {
    unsigned long        ulSecs;    // This is a time_t
    unsigned long        ulFrac;    // LSB = 100ns
    } SuIrigTimeF1;

/*
 * Function Declaration
 * --------------------
 */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Decode_TimeF1(SuI106Ch10Header  * psuHeader,
                         void              * pvBuff,
                         SuIrigTimeF1      * psuTime);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Rel2IrigTime(uint8_t            abyRelTime[],
                        SuIrigTimeF1     * psuTime);


//void iInit_DOY2DMY(int iYear);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif