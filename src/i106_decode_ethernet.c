/****************************************************************************

 i106_decode_ethernet.c - Decode Ethernet (Format 0) and ARINC-664 (Format 1) data

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

#include "i106_stdint.h"

#include "irig106ch10.h"
#include "i106_decode_ethernet.h"

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

void vFillInMsgPtrs(SuEthernetF0_CurrMsg * psuCurrMsg);

/* ----------------------------------------------------------------------- */
/* Ethernet Format 0                                                       */
/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstEthernetF0(SuI106Ch10Header     * psuHeader,
                                  void                 * pvBuff,
                                  SuEthernetF0_CurrMsg * psuMsg)
    {

    // Set pointers to the beginning of the Ethernet buffer
    psuMsg->psuChanSpec = (SuEthernetF0_ChanSpec *)pvBuff;

    // Check for no messages
    psuMsg->uFrameNum = 0;
    if (psuMsg->psuChanSpec->uNumFrames == 0)
        return I106_NO_MORE_DATA;

    psuMsg->ulPktDataLen = psuHeader->ulDataLen;
 
    // Set the pointer to the first Ethernet message
    psuMsg->psuEthernetF0Hdr = (SuEthernetF0_Header *)
                              ((char *)(pvBuff)        + 
                                sizeof(SuEthernetF0_ChanSpec));

    // Set the pointer to the ethernet message data
    psuMsg->pauData          = (uint8_t *)psuMsg->psuEthernetF0Hdr +
                                sizeof(SuEthernetF0_Header);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextEthernetF0(SuEthernetF0_CurrMsg * psuMsg)
    {

    // Check for no more messages
    psuMsg->uFrameNum++;
    if (psuMsg->uFrameNum >= psuMsg->psuChanSpec->uNumFrames)
        return I106_NO_MORE_DATA;

    // Set pointer to the next ethernet message intrapacket header
    // Note that the next packet header must fall on an even byte boundary
    psuMsg->psuEthernetF0Hdr = (SuEthernetF0_Header *)
                              ((char *)(psuMsg->psuEthernetF0Hdr)  + 
                                sizeof(SuEthernetF0_Header)        + 
                                psuMsg->psuEthernetF0Hdr->uMsgDataLen +
                               (psuMsg->psuEthernetF0Hdr->uMsgDataLen % 2));

    // Set the pointer to the ethernet message data
    psuMsg->pauData          = (uint8_t *)psuMsg->psuEthernetF0Hdr +
                                sizeof(SuEthernetF0_Header);


    return I106_OK;
    }


/* ----------------------------------------------------------------------- */
/* Ethernet Format 1 (ARINC-664)                                           */
/* ----------------------------------------------------------------------- */

EnI106Status enI106_Decode_FirstEthernetF1(SuI106Ch10Header     * psuHeader,
                                           void                 * pvBuff,
                                           SuEthernetF1_CurrMsg * psuMsg)
    {

    // Set pointers to the beginning of the Ethernet buffer
    psuMsg->psuChanSpec = (SuEthernetF1_ChanSpec *)pvBuff;

    // Check for no messages
    psuMsg->uFrameNum = 0;
    if (psuMsg->psuChanSpec->uNumFrames == 0)
        return I106_NO_MORE_DATA;

    psuMsg->ulPktDataLen = psuHeader->ulDataLen;
 
    // Set the pointer to the first Ethernet message
    psuMsg->psuEthernetF1Hdr = (SuEthernetF1_Header *)
                               ((char *)(pvBuff) + sizeof(SuEthernetF1_ChanSpec));

    // Set the pointer to the ethernet message data and sequence number
    psuMsg->pauData       = (uint8_t *)psuMsg->psuEthernetF1Hdr + psuMsg->psuChanSpec->uIPHLength;
    psuMsg->puSequenceNum = (uint8_t *)psuMsg->psuEthernetF1Hdr + psuMsg->psuChanSpec->uIPHLength + psuMsg->psuEthernetF1Hdr->uMsgDataLen - 1;

    return I106_OK;
    }


/* ----------------------------------------------------------------------- */

EnI106Status enI106_Decode_NextEthernetF1(SuEthernetF1_CurrMsg * psuMsg)
    {

    // Check for no more messages
    psuMsg->uFrameNum++;
    if (psuMsg->uFrameNum >= psuMsg->psuChanSpec->uNumFrames)
        return I106_NO_MORE_DATA;

    // Set pointer to the next ethernet message intrapacket header
    // Note that the next packet header must fall on an even byte boundary
    psuMsg->psuEthernetF1Hdr = (SuEthernetF1_Header *)
                              ((char *)(psuMsg->psuEthernetF1Hdr)  + 
                                sizeof(SuEthernetF1_Header)        + 
                                psuMsg->psuEthernetF1Hdr->uMsgDataLen +
                               (psuMsg->psuEthernetF1Hdr->uMsgDataLen % 2));

    // Set the pointer to the ethernet message data
    psuMsg->pauData       = (uint8_t *)psuMsg->psuEthernetF1Hdr + sizeof(SuEthernetF1_Header);
    psuMsg->puSequenceNum = (uint8_t *)psuMsg->psuEthernetF1Hdr + sizeof(SuEthernetF1_Header) + psuMsg->psuEthernetF1Hdr->uMsgDataLen - 1;

    return I106_OK;
    }

#ifdef __cplusplus
} // end namespace i106
#endif
