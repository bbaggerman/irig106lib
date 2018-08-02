/****************************************************************************

 i106_decode_16pp194.h - 

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

#ifndef _I106_DECODE_16PP194_H
#define _I106_DECODE_16PP194_H

#ifdef __cplusplus
namespace Irig106 {
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

// Channel specific header
typedef struct 
    {
    uint32_t    uMsgCnt;                // Message count
#if !defined(__GNUC__)
    } Su16PP194_ChanSpec;
#else
    } __attribute__ ((packed)) Su16PP194_ChanSpec;
#endif

// 16PP194 data words

typedef struct
    {
    uint16_t    uDataWordHigh   : 8;    // Data word bits 24-17
    uint16_t    bParityError    : 1;    // Parity error
    uint16_t    bWordError      : 1;    // Word bit error
    uint16_t    uGap            : 3;    // Gap time
    uint16_t    uBusID          : 3;    // Bus ID
    uint16_t    uDataWordLow;           // Data word bits 1-16
#if !defined(__GNUC__)
    } Su16PP194_DataWord;
#else
    } __attribute__ ((packed)) Su16PP194_DataWord;
#endif
    

// 16PP194 transaction
typedef struct
    {
    Su16PP194_DataWord  suCommand;
    Su16PP194_DataWord  suResponse;
    Su16PP194_DataWord  suCommandEcho;
    Su16PP194_DataWord  suNoGo;
    Su16PP194_DataWord  suNoGoEcho;
    Su16PP194_DataWord  suStatus;
#if !defined(__GNUC__)
    } Su16PP194_Transaction;
#else
    } __attribute__ ((packed)) Su16PP194_Transaction;
#endif

typedef struct 
    {
    // Timestampe
    uint8_t     aubyIntPktTime[8];      // Reference time
    // Intra-message header
    uint16_t    bTransError      : 1;   // Transaction error
    uint16_t    bResetError      : 1;   // Bus master reset
    uint16_t    bMsgTimeoutError : 1;   // Timeout error
    uint16_t    Reserved1        : 6;
    uint16_t    bStatusError     : 1;   // Status error
    uint16_t    Reserved2        : 2;
    uint16_t    bEchoError       : 1;   // Echo error
    uint16_t    Reserved3        : 3;
    uint16_t    uDataLength;            // Data length
    // Data words
    uint16_t    suData[12];
#if !defined(__GNUC__)
    } Su16PP194_Msg;
#else
    } __attribute__ ((packed)) Su16PP194_Msg;
#endif

// Current 16PP194 message
typedef struct
    {
    unsigned int            uMsgNum;
    uint32_t                ulCurrOffset;   // Offset into data buffer
    uint32_t                ulDataLen;
    Su16PP194_ChanSpec    * psuChanSpec;
    Su16PP194_Msg         * psu16PP194Msg;
#if !defined(__GNUC__)
    } Su16PP194_CurrMsg;
#else
    } __attribute__ ((packed)) Su16PP194_CurrMsg;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_First16PP194(SuI106Ch10Header * psuHeader,
                              void              * pvBuff,
                              Su16PP194_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_Next16PP194(Su16PP194_CurrMsg * psuMsg);

#ifdef __cplusplus
}
}
#endif

#endif
