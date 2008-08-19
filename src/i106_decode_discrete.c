/****************************************************************************

 i106_decode_discrete.c -

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

#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_decode_discrete.h"

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

static void vFillInMsgPtrs(SuDiscreteF1_CurrMsg * psuCurrMsg);

/* ======================================================================= */

EnI106Status I106_CALL_DECL
    enI106_Decode_FirstDiscreteF1(SuI106Ch10Header     * psuHeader,
                                  void                 * pvBuff,
                                  SuDiscreteF1_CurrMsg * psuCurrMsg,
                                  SuTimeRef            * psuTimeRef)
    {

    psuCurrMsg->uBytesRead = 0;

    // Set pointers to the beginning of the Discrete buffer
    psuCurrMsg->psuChanSpec = (SuDiscreteF1_ChanSpec *)pvBuff;

    psuCurrMsg->uBytesRead+=sizeof(SuDiscreteF1_ChanSpec);

    // Check for no data
    if (psuHeader->ulDataLen <= psuCurrMsg->uBytesRead)
        return I106_NO_MORE_DATA;


    // Get the other pointers
    vFillInMsgPtrs(psuCurrMsg);

    vFillInTimeStruct(psuHeader, psuCurrMsg->psuIPTimeStamp, psuTimeRef);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL
    enI106_Decode_NextDiscreteF1(SuI106Ch10Header     * psuHeader,
                                 SuDiscreteF1_CurrMsg * psuCurrMsg,
                                 SuTimeRef            * psuTimeRef)
    {

   // Check for no more data
    if (psuHeader->ulDataLen <= psuCurrMsg->uBytesRead)
        return I106_NO_MORE_DATA;

    // Get the other pointers
    vFillInMsgPtrs(psuCurrMsg);

    vFillInTimeStruct(psuHeader, psuCurrMsg->psuIPTimeStamp, psuTimeRef);

    return I106_OK;
    }

/* ----------------------------------------------------------------------- */

void vFillInMsgPtrs(SuDiscreteF1_CurrMsg * psuCurrMsg)
{

    psuCurrMsg->psuIPTimeStamp = (SuIntraPacketTS *)
                              ((char *)(psuCurrMsg->psuChanSpec) +
                               psuCurrMsg->uBytesRead);
    psuCurrMsg->uBytesRead+=sizeof(psuCurrMsg->psuIPTimeStamp->aubyIntPktTime);

    psuCurrMsg->uDiscreteData = *( (uint32_t *) ((char *)(psuCurrMsg->psuChanSpec) +
                              psuCurrMsg->uBytesRead));
    psuCurrMsg->uBytesRead+=sizeof(psuCurrMsg->uDiscreteData);

}

#ifdef __cplusplus
} // end namespace Irig106
#endif
