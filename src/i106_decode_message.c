/****************************************************************************

 i106_decode_message.c - 

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

#include "i106_stdint.h"

#include "irig106ch10.h"
#include "i106_decode_message.h"

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




/* ======================================================================= */

EnI106Status I106_CALL_DECL 
    enI106_Read_FirstMSGF0(SuI106Ch10Header* psuHeader,
                                           void* pvBuff,
                            SuMessageF0_CurrMsg* psuMsg)
    {

    // Set pointers to the beginning of the message buffer
    psuMsg->psuChanSpec = (SuMessageF0_ChanSpec*)pvBuff;

    // Check for no messages
    psuMsg->uMsgNum = 0;
    if (psuMsg->psuChanSpec->uCounter == 0)
        return I106_NO_MORE_DATA;

    // Figure out the offset to the first  message and
    // make sure it isn't beyond the end of the data buffer
    psuMsg->ulDataLen    = psuHeader->ulDataLen;
    psuMsg->ulCurrOffset = sizeof(SuMessageF0_ChanSpec);
    if (psuMsg->ulCurrOffset >= psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    // Set the pointer to the first message
    psuMsg->psuMSGF0Hdr = (SuMessageF0_Header*)
                             ((char *)(pvBuff) + psuMsg->ulCurrOffset);
    // Check to make sure the data does run beyond the end of the buffer
    if ((psuMsg->ulCurrOffset    + 
         sizeof(SuMessageF0_Header) +
         psuMsg->psuMSGF0Hdr->uMsgLength) > psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    psuMsg->pauData = (uint8_t*)(psuMsg->psuMSGF0Hdr +1);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Read_NextMSGF0(SuMessageF0_CurrMsg* psuMsg)
    {

    // Check for no more messages
    psuMsg->uMsgNum++;
    if (psuMsg->uMsgNum >= psuMsg->psuChanSpec->uCounter)
        return I106_NO_MORE_DATA;

    // Figure out the offset to the next message and
    // make sure it isn't beyond the end of the data buffer
    psuMsg->ulCurrOffset += sizeof(SuMessageF0_Header)      +
                            psuMsg->psuMSGF0Hdr->uMsgLength;

    if (psuMsg->ulCurrOffset >= psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    // Set pointer to the next data buffer
    psuMsg->psuMSGF0Hdr = (SuMessageF0_Header*)
                               ((char *)(psuMsg->psuMSGF0Hdr) +
                                sizeof(SuMessageF0_Header)      +
                                psuMsg->psuMSGF0Hdr->uMsgLength);

    // Check to make sure the data does run beyond the end of the buffer
    if ((psuMsg->ulCurrOffset    + 
         sizeof(SuMessageF0_Header) +
         psuMsg->psuMSGF0Hdr->uMsgLength) > psuMsg->ulDataLen)
        return I106_BUFFER_OVERRUN;

    psuMsg->pauData = (uint8_t*)(psuMsg->psuMSGF0Hdr + 1);

    return I106_OK;
    }


#ifdef __cplusplus
} // end namespace i106
#endif
