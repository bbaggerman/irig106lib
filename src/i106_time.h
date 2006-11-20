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

 Created by Bob Baggerman

 $RCSfile: i106_time.h,v $
 $Date: 2006-11-20 04:36:20 $
 $Revision: 1.2 $

 ****************************************************************************/

#ifndef _I106_TIME_H
#define _I106_TIME_H

// #include "irig106ch10.h"

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

// Time has a number of representations in the IRIG 106 spec.
// The structure below is used as a convenient standard way of
// representing time.  The nice thing about standards is that there 
// are so many to choose from, and time is no exception. But none of 
// the various C time representations really fill the bill. So I made 
// a new time representation.  So there.
typedef struct 
    {
    uint32_t        ulSecs;    // This is a time_t
    uint32_t        ulFrac;    // LSB = 100ns
    } SuIrig106Time;

// Relative time to absolute time reference
typedef struct 
    {
    uint64_t        uRelTime;          // Relative time from header
    SuIrig106Time   suIrigTime;        // Clock time from IRIG source
    } SuTimeRef;


/*
 * Global data
 * -----------
 */


/*
 * Function Declaration
 * --------------------
 */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_SetRelTime(int              iI106Ch10Handle,
                      SuIrig106Time  * psuTime,
                      uint8_t          abyRelTime[]);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Rel2IrigTime(int                  iI106Ch10Handle,
                        uint8_t              abyRelTime[],
                        SuIrig106Time      * psuTime);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Irig2RelTime(int              iI106Ch10Handle,
                        SuIrig106Time  * psuTime,
                        uint8_t          abyRelTime[]);

// Warning - array to int / int to array functions are little endian only!

I106_DLL_DECLSPEC void I106_CALL_DECL 
    vLLInt2TimeArray(int64_t * pllRelTime,
                     uint8_t   abyRelTime[]);


I106_DLL_DECLSPEC void I106_CALL_DECL 
    vTimeArray2LLInt(uint8_t   abyRelTime[],
                     int64_t * pllRelTime);


I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_SyncTime(int     iI106Ch10Handle,
                    int     bRequireSync,
                    int     iTimeLimit);

// IT WOULD BE NICE TO HAVE SOME FUNCTIONS TO COMPARE 6 BYTE
// TIME ARRAY VALUES FOR EQUALITY AND INEQUALITY

// This is handy enough that we'll go ahead and export it to the world
// HMMM... MAYBE A BETTER WAY TO DO THIS IS TO MAKE THE TIME VARIABLES
// AND STRUCTURES THOSE DEFINED IN THIS PACKAGE.
I106_DLL_DECLSPEC time_t I106_CALL_DECL 
    mkgmtime(struct tm * psuTmTime);

#ifdef __cplusplus
}
#endif

#endif
