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
#include "i106_decode_video.h"

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

/// Setup reading multiple Video Format 0 messages

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstVideoF0(SuI106Ch10Header  * psuHeader,
                               void              * pvBuff,
                               SuVideoF0_CurrMsg * psuCurrMsg)
    {

    // Save pointer to channel specific data
    psuCurrMsg->psuChanSpec = (SuVideoF0_ChanSpec *)pvBuff;

    // Set pointers if embedded time used
    if (psuCurrMsg->psuChanSpec->bET == 1)
        {
        psuCurrMsg->psuIPHeader = (SuVideoF0_Header *)
                                  ((char *)pvBuff + sizeof(SuVideoF0_ChanSpec));
        psuCurrMsg->pachTSData  = (uint8_t         *)
                                  ((char *)pvBuff             + 
                                   sizeof(SuVideoF0_ChanSpec) +
                                   sizeof(SuVideoF0_Header));
        }

    // No embedded time
    else
        {
        psuCurrMsg->psuIPHeader = NULL;
        psuCurrMsg->pachTSData  = (uint8_t *)pvBuff + sizeof(SuVideoF0_ChanSpec);
        }

// TAKE CARE OF BYTE SWAPPING BASED ON CH 10 RELEASE AND BA CSDW (NEW IN -09)

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextVideoF0 (SuI106Ch10Header  * psuHeader,
                               SuVideoF0_CurrMsg * psuCurrMsg)
    {
    int     iNextOffset;

    // Calculate the offset to the next video packet
    if (psuCurrMsg->psuChanSpec->bET == 1)
        {
        iNextOffset = 188 + sizeof(SuVideoF0_Header);
        psuCurrMsg->psuIPHeader = (SuVideoF0_Header *)
                                  ((char *)psuCurrMsg->psuIPHeader + iNextOffset);
        psuCurrMsg->pachTSData  = (uint8_t         *)
                                  ((char *)psuCurrMsg->pachTSData  + iNextOffset);
        }
    else
        {
        iNextOffset = 188;
        psuCurrMsg->psuIPHeader = (SuVideoF0_Header *)NULL;
        psuCurrMsg->pachTSData  = (uint8_t         *)
                                  ((char *)psuCurrMsg->pachTSData + iNextOffset);
        }

    // If new data pointer is beyond end of buffer then we're done
    if ((unsigned long)((char *)psuCurrMsg->pachTSData - (char *)psuCurrMsg->psuChanSpec) >= psuHeader->ulDataLen)
        return I106_NO_MORE_DATA;

    return I106_OK;
    }




/* ----------------------------------------------------------------------- */

#ifdef __cplusplus
} // end namespace Irig106
#endif
