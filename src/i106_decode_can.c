/****************************************************************************

 i106_decode_can.c -

 Copyright (c) 2014 Irig106.org

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

 Created by: Tommaso Falchi Delitala <volalto86@gmail.com>

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"

#include "i106_decode_can.h"

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

static void vFillInMsgPtrs(SuCan_CurrMsg * psuCurrMsg);

/* ======================================================================= */

EnI106Status I106_CALL_DECL
    enI106_Decode_FirstCan(SuI106Ch10Header         * psuHeader,
                           void                     * pvBuff,
                           SuCan_CurrMsg            * psuCurrMsg)

    {

    psuCurrMsg->uBytesRead = 0;

    // Keep a pointer to the current header
    psuCurrMsg->psuHeader = psuHeader;

    // Set pointers to the beginning of the Message buffer
    psuCurrMsg->psuChanSpec = (SuCan_ChanSpec *)pvBuff;
    psuCurrMsg->uBytesRead += sizeof(SuCan_ChanSpec);

    // Check for no messages
    psuCurrMsg->uMsgNum = 0;
    if (psuCurrMsg->psuChanSpec->uCounter == 0)
        return I106_NO_MORE_DATA;

    // Get the other pointers
    vFillInMsgPtrs(psuCurrMsg);

    vFillInTimeStruct(psuHeader, psuCurrMsg->psuIPTimeStamp, &psuCurrMsg->suTimeRef);

    return I106_OK;

    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL
    enI106_Decode_NextCan(SuCan_CurrMsg *psuCurrMsg)
    {

    // Check for no more messages
       psuCurrMsg->uMsgNum++;
       if (psuCurrMsg->uMsgNum >= psuCurrMsg->psuChanSpec->uCounter)
           return I106_NO_MORE_DATA;

    // Check for packet overrun
        if (psuCurrMsg->psuHeader->ulDataLen <= psuCurrMsg->uBytesRead)
            return I106_NO_MORE_DATA;

    // Get the other pointers
    vFillInMsgPtrs(psuCurrMsg);

    vFillInTimeStruct(psuCurrMsg->psuHeader, psuCurrMsg->psuIPTimeStamp, &psuCurrMsg->suTimeRef);

    return I106_OK;

    }

/* ----------------------------------------------------------------------- */

void vFillInMsgPtrs(SuCan_CurrMsg * psuCurrMsg)
{
     // Set the pointer to the intra-packet time stamp
    psuCurrMsg->psuIPTimeStamp = (SuIntraPacketTS *)
                              ((char *)(psuCurrMsg->psuChanSpec) +
                               psuCurrMsg->uBytesRead);

    // Set the pointer to the intra-packet header
    psuCurrMsg->psuCanHdr = (SuCan_Header *)
                             ((char *)(psuCurrMsg->psuChanSpec) +
                              psuCurrMsg->uBytesRead);
    psuCurrMsg->uBytesRead += sizeof(SuCan_Header);

    // Set pointer to the CAN ID word
    psuCurrMsg->psuCanIdWord = (SuCan_IdWord*)
                             ((char *)(psuCurrMsg->psuChanSpec) +
                              psuCurrMsg->uBytesRead);

    //Do not increment uBytesRead as MsgLength already include IdWord size

    // Set the pointer to the data
    // MsgLength = IdWord (4 bytes) + CAN Payload (0-4 bytes)
    psuCurrMsg->pauData = (uint8_t *)((char *)(psuCurrMsg->psuChanSpec) + psuCurrMsg->uBytesRead);

    // Add the data length, if it is odd, account for the filler byte we will skip
    psuCurrMsg->uBytesRead+=  psuCurrMsg->psuCanHdr->uMsgLength
                            + ( psuCurrMsg->psuCanHdr->uMsgLength % 2);
}

#ifdef __cplusplus
}
#endif

