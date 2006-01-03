/****************************************************************************

 i106_decode1553f1.h - 

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

 $RCSfile: i106_decode_1553f1.h,v $
 $Date: 2006-01-03 15:47:06 $
 $Revision: 1.7 $

 ****************************************************************************/

#ifndef _I106_DECODE_1553F1_H
#define _I106_DECODE_1553F1_H

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

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

typedef struct 
    {
    uint16_t    uWC : 5;
    uint16_t    uSA : 5;
    uint16_t    uTR : 1;
    uint16_t    uRT : 5;
    } GCC_PACK SuCmdWord;

/* 1553 Format 1 */

// Channel specific header
typedef struct 
    {
    uint32_t    uMsgCnt      : 24;      // Message count
    uint32_t    Reserved     :  6;
    uint32_t    uTTB         :  2;      // Time tag bits
    } GCC_PACK Su1553F1_ChanSpec;

// Intra-message header
typedef struct 
    {
    uint8_t     aubyIntPktTime[8];      // Reference time
    uint16_t    Reserved1       : 3;    // Reserved
    uint16_t    bWordError      : 1;
    uint16_t    bSyncError      : 1;
    uint16_t    bWordCntError   : 1;
    uint16_t    Reserved2       : 3;
    uint16_t    bRespTimeout    : 1;
    uint16_t    bFormatError    : 1;
    uint16_t    bRT2RT          : 1;
    uint16_t    bMsgError       : 1;
    uint16_t    iBusID          : 1;
    uint16_t    Reserved3       : 2;
    uint8_t     uGapTime1;
    uint8_t     uGapTime2;
    uint16_t    uMsgLen;
    } GCC_PACK Su1553F1_Header;

// Current 1553 message
typedef struct
    {
    unsigned int            uMsgNum;
    Su1553F1_ChanSpec     * psuChanSpec;
    Su1553F1_Header       * psu1553Hdr;
    uint16_t              * puCmdWord1;
    uint16_t              * puCmdWord2;
    uint16_t              * puStatWord1;
    uint16_t              * puStatWord2;
    uint16_t              * pauData;
    } GCC_PACK Su1553F1_CurrMsg;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Function Declaration
 * --------------------
 */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Decode_First1553F1(SuI106Ch10Header * psuHeader,
                              void             * pvBuff,
                              Su1553F1_CurrMsg * psuMsg);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Decode_Next1553F1(Su1553F1_CurrMsg * psuMsg);

char * szCmdWord(unsigned int iCmdWord);

#ifdef __cplusplus
}
#endif

#endif
