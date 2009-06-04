/****************************************************************************

 i106_decode_1553f1.c - 

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

#include "stdint.h"

#include "irig106ch10.h"
#include "i106_decode_1553f1.h"

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



/*
 * Function Declaration
 * --------------------
 */

static void vFillInMsgPtrs(Su1553F1_CurrMsg * psuCurrMsg);
//static int  i1553WordCnt(const SuCmdWordU * psuCmdWord);


/* ======================================================================= */

EnI106Status I106_CALL_DECL 
    enI106_Decode_First1553F1(SuI106Ch10Header * psuHeader,
                              void             * pvBuff,
                              Su1553F1_CurrMsg * psuMsg)
    {

    // Set pointers to the beginning of the 1553 buffer
    psuMsg->psuChanSpec = (Su1553F1_ChanSpec *)pvBuff;

    // Check for no messages
    psuMsg->uMsgNum = 0;
    if (psuMsg->psuChanSpec->uMsgCnt == 0)
        return I106_NO_MORE_DATA;

    // Check for too many messages. There was a problem with a recorder
    // that produced bad 1553 packets.  These bad packets showed *huge*
    // message counts, and all the data was garbage.  This test catches
    // this case.  Sorry.  8-(
    if (psuMsg->psuChanSpec->uMsgCnt > 100000)
        return I106_BUFFER_OVERRUN;

    // Figure out the offset to the first 1553 message and
    // make sure it isn't beyond the end of the data buffer
    psuMsg->ulDataLen    = psuHeader->ulDataLen;
    psuMsg->ulCurrOffset = sizeof(Su1553F1_ChanSpec);
    if (psuMsg->ulCurrOffset >= psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    // Set the pointer to the first 1553 message
    //psuMsg->psu1553Hdr = (Su1553F1_Header *)
    //                         ((char *)(pvBuff) + 
    //                          sizeof(psuMsg->psuChanSpec));
    psuMsg->psu1553Hdr = (Su1553F1_Header *)
                             ((char *)(pvBuff) + psuMsg->ulCurrOffset);

    // Check to make sure the data does run beyond the end of the buffer
    if ((psuMsg->ulCurrOffset    + 
         sizeof(Su1553F1_Header) +
         psuMsg->psu1553Hdr->uMsgLen) > psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    // Get the other pointers
    vFillInMsgPtrs(psuMsg);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_Next1553F1(Su1553F1_CurrMsg * psuMsg)
    {

    // Check for no more messages
    psuMsg->uMsgNum++;
    if (psuMsg->uMsgNum >= psuMsg->psuChanSpec->uMsgCnt)
        return I106_NO_MORE_DATA;

    // Figure out the offset to the next 1553 message and
    // make sure it isn't beyond the end of the data buffer
    psuMsg->ulCurrOffset += sizeof(Su1553F1_Header)      + 
                            psuMsg->psu1553Hdr->uMsgLen;

    if (psuMsg->ulCurrOffset >= psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    // Set pointer to the next 1553 data buffer
    psuMsg->psu1553Hdr  = (Su1553F1_Header *)
                               ((char *)(psuMsg->psu1553Hdr) + 
                                sizeof(Su1553F1_Header)      + 
                                psuMsg->psu1553Hdr->uMsgLen);

    // Check to make sure the data does run beyond the end of the buffer
    if ((psuMsg->ulCurrOffset    + 
         sizeof(Su1553F1_Header) +
         psuMsg->psu1553Hdr->uMsgLen) > psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;


//psu1553Hdr = (Su1553F1_Header *)((char *)psu1553Hdr + sizeof(Su1553F1_Header) + psu1553Hdr->uMsgLen);

    // Get the other pointers
    vFillInMsgPtrs(psuMsg);

    return I106_OK;
    }




/* ----------------------------------------------------------------------- */

void vFillInMsgPtrs(Su1553F1_CurrMsg * psuCurrMsg)
    {

    psuCurrMsg->psuCmdWord1  = (SuCmdWordU *)
        ((char *)(psuCurrMsg->psu1553Hdr) + sizeof(Su1553F1_Header));

    // Position of data and status response differ between transmit and receive
    // If not RT to RT
    if ((psuCurrMsg->psu1553Hdr)->bRT2RT == 0)
        {
        // Second command and status words not available
        psuCurrMsg->psuCmdWord2  = NULL;
        psuCurrMsg->puStatWord2  = NULL;

        // Figure out the word count
        psuCurrMsg->uWordCnt = i1553WordCnt(psuCurrMsg->psuCmdWord1);

        // Receive
        if (psuCurrMsg->psuCmdWord1->suStruct.bTR == 0)
            {
            psuCurrMsg->pauData     = (uint16_t *)psuCurrMsg->psuCmdWord1 + 1;
            psuCurrMsg->puStatWord1 = psuCurrMsg->pauData + psuCurrMsg->uWordCnt;
            }

        //Transmit
        else
            {
            psuCurrMsg->puStatWord1 = (uint16_t *)psuCurrMsg->psuCmdWord1 + 1;
            psuCurrMsg->pauData     = (uint16_t *)psuCurrMsg->psuCmdWord1 + 2;
            }
        } // end if not RT to RT

    // RT to RT
    else
        {
        psuCurrMsg->psuCmdWord2 = psuCurrMsg->psuCmdWord1 + 1;
        psuCurrMsg->uWordCnt    = i1553WordCnt(psuCurrMsg->psuCmdWord2);
        psuCurrMsg->puStatWord2 = (uint16_t *)psuCurrMsg->psuCmdWord1 + 2;
        psuCurrMsg->pauData     = (uint16_t *)psuCurrMsg->psuCmdWord1 + 3;
        psuCurrMsg->puStatWord1 = (uint16_t *)psuCurrMsg->pauData     + psuCurrMsg->uWordCnt;
        } // end if RT to RT

    return;
    }



/* ----------------------------------------------------------------------- */

char * szCmdWord(unsigned int iCmdWord)
    {
    static char     szCmdWord[16];
    SuCmdWord     * psuCmdWord = (SuCmdWord *)&iCmdWord;

    sprintf(szCmdWord, "%2d-%c-%2d-%2d",
         psuCmdWord->uRTAddr,
         psuCmdWord->bTR ? 'T' : 'R',
         psuCmdWord->uSubAddr,
         psuCmdWord->uWordCnt==0 ? 32 : psuCmdWord->uWordCnt);

    return &szCmdWord[0];
    }



/* ------------------------------------------------------------------------ */

/* Return the number of word in a 1553 message taking into account mode codes */

int I106_CALL_DECL 
    i1553WordCnt(const SuCmdWordU * psuCmdWord)
    {

    // If the subaddress is a mode code then find out number of data words
    if ((psuCmdWord->suStruct.uSubAddr == 0x0000) ||
        (psuCmdWord->suStruct.uSubAddr == 0x001f)) 
        {
        if (psuCmdWord->suStruct.uWordCnt & 0x0010) return 1;
        else                                        return 0;
        } // end if mode code

    // If regular subaddress find out number of data words
    else
        {
        if (psuCmdWord->suStruct.uWordCnt == 0)     return 32;
        else                                        return psuCmdWord->suStruct.uWordCnt;
        } // end if regular subaddress
  }

#ifdef __cplusplus
} // end namespace i106
#endif
