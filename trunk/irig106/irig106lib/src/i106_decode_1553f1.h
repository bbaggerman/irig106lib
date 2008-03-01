/****************************************************************************

 i106_decode_1553f1.h - 

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
#pragma pack(push)
#pragma pack(1)
#endif

// 1553 Command Word bit fields
typedef struct 
    {
    uint16_t    uWordCnt    : 5;    // Data Word Count or Mode Code
    uint16_t    uSubAddr    : 5;    // Subaddress Specifier
    uint16_t    bTR         : 1;    // Transmit/Receive Flag
    uint16_t    uRTAddr     : 5;    // RT Address
#if !defined(__GNUC__)
    } SuCmdWord;
#else
    } __attribute__ ((packed)) SuCmdWord;
#endif

// A union to make manipulating the command word easier
typedef union 
    {
    SuCmdWord   suStruct;
    uint16_t    uValue;
    } SuCmdWordU;

/* 1553 Format 1 */

// Channel specific header
typedef struct 
    {
    uint32_t    uMsgCnt      : 24;      // Message count
    uint32_t    Reserved     :  6;
    uint32_t    uTTB         :  2;      // Time tag bits
#if !defined(__GNUC__)
    } Su1553F1_ChanSpec;
#else
    } __attribute__ ((packed)) Su1553F1_ChanSpec;
#endif

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
#if !defined(__GNUC__)
    } Su1553F1_Header;
#else
    } __attribute__ ((packed)) Su1553F1_Header;
#endif

// Current 1553 message
typedef struct
    {
    unsigned int            uMsgNum;
    Su1553F1_ChanSpec     * psuChanSpec;
    Su1553F1_Header       * psu1553Hdr;
    SuCmdWordU            * psuCmdWord1;
    SuCmdWordU            * psuCmdWord2;
    uint16_t              * puStatWord1;
    uint16_t              * puStatWord2;
    uint16_t                uWordCnt;
    uint16_t              * pauData;
#if !defined(__GNUC__)
    } Su1553F1_CurrMsg;
#else
    } __attribute__ ((packed)) Su1553F1_CurrMsg;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_First1553F1(SuI106Ch10Header * psuHeader,
                              void             * pvBuff,
                              Su1553F1_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_Next1553F1(Su1553F1_CurrMsg * psuMsg);

int I106_CALL_DECL 
    i1553WordCnt(const SuCmdWordU * psuCmdWord);

char * szCmdWord(unsigned int iCmdWord);

#ifdef __cplusplus
}
#endif

#endif
