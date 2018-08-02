/****************************************************************************

 i106_decode_16pp194.c - 

 Copyright (c) 2018 Irig106.org

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
#include "i106_decode_16pp194.h"

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

static void vFillInMsgPtrs(Su16PP194_CurrMsg * psuCurrMsg);

/* ======================================================================= */

EnI106Status I106_CALL_DECL 
    enI106_Decode_First16PP194(SuI106Ch10Header * psuHeader,
                              void              * pvBuff,
                              Su16PP194_CurrMsg * psuMsg)
    {

    // Set pointers to the beginning of the 16PP194 buffer
    psuMsg->psuChanSpec = (Su16PP194_ChanSpec *)pvBuff;

    // Check for no messages
    psuMsg->uMsgNum = 0;
    if (psuMsg->psuChanSpec->uMsgCnt == 0)
        return I106_NO_MORE_DATA;

    // Figure out the offset to the first 16PP194 message and
    // make sure it isn't beyond the end of the data buffer
    psuMsg->ulDataLen    = psuHeader->ulDataLen;
    psuMsg->ulCurrOffset = sizeof(Su16PP194_ChanSpec);
    if (psuMsg->ulCurrOffset >= psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    // Set the pointer to the first 16PP194 message
    psuMsg->psu16PP194Msg = (Su16PP194_Msg *)((char *)(pvBuff) + psuMsg->ulCurrOffset);

    // Check to make sure the data does run beyond the end of the buffer
    if ((psuMsg->ulCurrOffset + sizeof(Su16PP194_Msg)) > psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    // Get the other pointers
//    vFillInMsgPtrs(psuMsg);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_Next16PP194(Su16PP194_CurrMsg * psuMsg)
    {

    // Check for no more messages
    psuMsg->uMsgNum++;
    if (psuMsg->uMsgNum >= psuMsg->psuChanSpec->uMsgCnt)
        return I106_NO_MORE_DATA;

    // Figure out the offset to the next 16PP194 message and
    // make sure it isn't beyond the end of the data buffer
    psuMsg->ulCurrOffset += sizeof(Su16PP194_Msg);

    if (psuMsg->ulCurrOffset >= psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    // Set pointer to the next 16PP194 data buffer
    psuMsg->psu16PP194Msg  = (Su16PP194_Msg *)((char *)(psuMsg->psu16PP194Msg) + sizeof(Su16PP194_Msg));

    // Check to make sure the data does run beyond the end of the buffer
    if ((psuMsg->ulCurrOffset + sizeof(Su16PP194_Msg)) > psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    // Get the other pointers
//    vFillInMsgPtrs(psuMsg);

    return I106_OK;
    }




/* ----------------------------------------------------------------------- */

// Nothing to do for now.
void vFillInMsgPtrs(Su16PP194_CurrMsg * psuCurrMsg)
    {

    return;
    }



#ifdef __cplusplus
} // end namespace i106
#endif
