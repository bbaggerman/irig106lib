/****************************************************************************

 i106_decode_arinc429.c -

 Copyright (c) 2010 Irig106.org

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

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_decode_arinc429.h"

// #include <windows.h>

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

#if 0
typedef struct
    {
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
    uint32_t msecond;
    uint32_t usecond;
    } currentWordTime;
#endif

/*
 * Module data
 * -----------
 */



/*
 * Function Declaration
 * --------------------
 */



/* ======================================================================= */

#if 0
EnI106Status I106_CALL_DECL
enI106_Decode_Arinc429F0(SuI106Ch10Header       * psuHeader,
                        void                    * pvBuff,
                        SuArinc429F0_CurrMsg    * psuCurrMsg)
    {
    int i;
    int numDataWords;
    SuArinc429F0_Header *tmpPtrH;
    SuArinc429F0_Data   *tmpPtrD;

    // Save pointer to channel specific data
    psuCurrMsg->psuChanSpec = (SuArinc429F0_ChanSpec *)pvBuff;

    numDataWords = psuCurrMsg->psuChanSpec->uMsgCount;

    psuCurrMsg->uMsgNum = numDataWords;

    // Read in the rest of the data, alternating intra packet headers and data words
    psuCurrMsg->ulCurrOffset = sizeof(SuArinc429F0_ChanSpec);

//    psuCurrMsg->psu429Hdr  = new SuArinc429F0_Header[numDataWords];
//    psuCurrMsg->psu429Data = new SuArinc429F0_Data[numDataWords];

    for (i = 0; i < numDataWords; i++) 
        {
        // Check to make sure the data does run beyond the end of the buffer
        if (psuCurrMsg->ulCurrOffset > psuHeader->ulDataLen)
            return I106_BUFFER_OVERRUN;

        tmpPtrH = (SuArinc429F0_Header*)((char*)pvBuff + psuCurrMsg->ulCurrOffset);
        psuCurrMsg->psu429Hdr[i] = *tmpPtrH;
        psuCurrMsg->ulCurrOffset += sizeof(SuArinc429F0_Header);

        // Check to make sure the data does run beyond the end of the buffer
        if (psuCurrMsg->ulCurrOffset > psuHeader->ulDataLen)
            return I106_BUFFER_OVERRUN;

        tmpPtrD = (SuArinc429F0_Data*)((char*)pvBuff + psuCurrMsg->ulCurrOffset);
        psuCurrMsg->psu429Data[i] = *tmpPtrD;
        psuCurrMsg->ulCurrOffset += sizeof(SuArinc429F0_Data);
        } // end for all data words

    return I106_OK;
    }
#endif



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstArinc429F0(SuI106Ch10Header * psuHeader,
                              void                 * pvBuff,
                              SuArinc429F0_CurrMsg * psuMsg)
    {
    // Save pointer to channel specific data
    psuMsg->psuChanSpec = (SuArinc429F0_ChanSpec *)pvBuff;

    psuMsg->ulCurrOffset = sizeof(SuArinc429F0_ChanSpec);

    // Check for no messages
    psuMsg->uMsgNum = 0;
    if (psuMsg->psuChanSpec->uMsgCount == 0)
        return I106_NO_MORE_DATA;

    // Make the time for the current message
    vTimeArray2LLInt(psuHeader->aubyRefTime, &(psuMsg->llIntPktTime));

    // Set pointers the header and data
    psuMsg->psu429Hdr  = (SuArinc429F0_Header *)((char *)pvBuff + sizeof(SuArinc429F0_ChanSpec));
    psuMsg->psu429Data = (SuArinc429F0_Data *)((char *)(psuMsg->psu429Hdr) + sizeof(SuArinc429F0_Header));

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextArinc429F0(SuArinc429F0_CurrMsg * psuMsg)
    {

    // Check for no more messages
    psuMsg->uMsgNum++;
    if (psuMsg->uMsgNum >= psuMsg->psuChanSpec->uMsgCount)
        return I106_NO_MORE_DATA;

    // Figure out the offset to the next ARINC 429 message and
    // make sure it isn't beyond the end of the data buffer
    psuMsg->ulCurrOffset += sizeof(SuArinc429F0_Header) + 
                            sizeof(SuArinc429F0_Data);

//    if (psuMsg->ulCurrOffset >= psuMsg->ulDataLen)
//        return I106_BUFFER_OVERRUN;

    // Set pointer to the next ARINC 429 header
    psuMsg->psu429Hdr = (SuArinc429F0_Header *)((char *)psuMsg->psu429Hdr           +
                                                        sizeof(SuArinc429F0_Header) + 
                                                        sizeof(SuArinc429F0_Data));

    // Set pointer to the next ARINC 429 data buffer
    psuMsg->psu429Data = (SuArinc429F0_Data *)((char *)(psuMsg->psu429Hdr) + sizeof(SuArinc429F0_Header));

    // Make the time for the current message
    psuMsg->llIntPktTime += psuMsg->psu429Hdr->uGapTime;

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

#if 0
EnI106Status I106_CALL_DECL
enI106_getWordTime_Arinc429F0(int                   m_iI106Handle, 
                              SuI106Ch10Header    * suI106Hdr, 
                              SuArinc429F0_Header * suCurrHdrF0, 
                              currentWordTime     * wordTime, 
                              char                * szTime)
    {
    SuIrig106Time             suTime;
    int                       hr;
    int                       min;
    int                       sec;
    int                       msec;
    int                       usec;
    char                    * pch;

    char                      HH[10]; // hour
    char                      MM[10]; // minute
    char                      SS[10]; // second
    char                      MS[10]; // millisecond
    char                      US[10]; // microsecond


    // Is this this first data word in the packet?
    if (wordTime->hour == -1) 
        {
        EnI106Status enStatus = enI106_Rel2IrigTime(m_iI106Handle, suI106Hdr->aubyRefTime, &suTime);
        if (enStatus != I106_OK) 
            {
            strcpy(szTime,"HH:MM:SS:mmS:uuS");
            return I106_TIME_NOT_FOUND;
            }

        time_t dummyTime = (time_t)suTime.ulSecs;
        char *szTime_tmp = ctime(&dummyTime);
        strcpy(szTime,szTime_tmp);
        szTime[19] = '\0';
        
        // Parse the "szTime" string for hour, minute, and second
        char *justTimeNumbers = &szTime[11];
        pch = strtok(justTimeNumbers,":");
        hr  = atoi(pch);
        pch = strtok(NULL,":");
        min = atoi(pch);
        pch = strtok(NULL,"");
        sec = atoi(pch);

        // Calculate milliseconds and microseconds
        msec = (int)(suTime.ulFrac / 10000.0);
        usec = (int)(1000*((suTime.ulFrac / 10000.0) - (int)(suTime.ulFrac / 10000.0)));
        
        // Update wordTime (in case you want to use the numbers, not the string, outside of this function)
        wordTime->hour    = hr;
        wordTime->minute  = min;
        wordTime->second  = sec;
        wordTime->msecond = msec;
        wordTime->usecond = usec;

        itoa(hr,  HH,10);
        itoa(min, MM,10);
        itoa(sec, SS,10);
        itoa(msec,MS,10);
        itoa(usec,US,10);

        strcpy(szTime,"");
        strcat(szTime,HH);
        strcat(szTime,":");
        strcat(szTime,MM);
        strcat(szTime,":");
        strcat(szTime,SS);
        strcat(szTime,":");
        strcat(szTime,MS);
        strcat(szTime,":");
        strcat(szTime,US);
        } // end if first data word

    // Not first data word
    else 
        {
        char szTime_tmp[80];
        strcpy(szTime_tmp,szTime);
        pch  = strtok(szTime_tmp,":");
        hr   = atoi(pch);
        pch  = strtok(NULL,":");
        min  = atoi(pch);
        pch  = strtok(NULL,":");
        sec  = atoi(pch);
        pch  = strtok(NULL,":");
        msec = atoi(pch);
        pch  = strtok(NULL,"");
        usec = atoi(pch);

        int gapTimeInMicroSecs = (int)(0.1*(float)suCurrHdrF0->uGapTime);
        
        usec += gapTimeInMicroSecs;
        if (usec > 999) 
            {
            usec %= 1000;
            msec++;
            if (msec > 999) 
                {
                msec %= 1000;
                sec++;
                if (sec > 59) 
                    {
                    sec %= 60;
                    min++;
                    if (min > 59) 
                        {
                        min %= 60;
                        hr++;
                        if (hr > 23) 
                            {
                            hr %= 24;
                            }
                        }
                    }
                }
            }

        itoa(hr,  HH,10);
        itoa(min, MM,10);
        itoa(sec, SS,10);
        itoa(msec,MS,10);
        itoa(usec,US,10);

        strcpy(szTime,"");
        strcat(szTime,HH);
        strcat(szTime,":");
        strcat(szTime,MM);
        strcat(szTime,":");
        strcat(szTime,SS);
        strcat(szTime,":");
        strcat(szTime,MS);
        strcat(szTime,":");
        strcat(szTime,US);
        } // end if not first data word

    return I106_OK;
    }
#endif


#ifdef __cplusplus
}
#endif

