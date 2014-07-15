/****************************************************************************

 i106_decode_can.h - CAN bus decoding, as per IRIG106 ch10, v. 2013

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

#ifndef _I106_DECODE_CAN_H
#define _I106_DECODE_CAN_H

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
#pragma pack(push,1)
#endif

// Channel specific data word
// --------------------------

typedef struct
    {
    uint32_t    uCounter        : 16;      // Message counter
    uint32_t    uReserved       : 16;
#if !defined(__GNUC__)
    } SuCan_ChanSpec;
#else
    } __attribute__ ((packed)) SuCan_ChanSpec;
#endif


// Intra-packed message header
typedef struct
    {
    uint64_t    suIntraPckTime;            // Reference time
    uint32_t    uMsgLength      :  4;      // Message length
    uint32_t    uReserved       : 12;
    uint32_t    uSubChannel     : 14;      // Subchannel number
    uint32_t    bFmtError       :  1;      // Format error flag
    uint32_t    bDataError      :  1;      // Data error flag
#if !defined(__GNUC__)
    } SuCan_Header;
#else
    } __attribute__ ((packed)) SuCan_Header;
#endif

// CAN ID Word
typedef struct
    {
    uint32_t    uCanId          : 29;    // CAN Id
    uint32_t    uReserved       :  1;    // 0 = 11-bit CAN id; 1 = 29-bit CAN Id
    uint32_t    rtr             :  1;    // Remote transfer request bit
    uint32_t    ide             :  1;    // IDE
#if !defined(__GNUC__)
    } SuCan_IdWord;
#else
    } __attribute__ ((packed)) SuCan_IdWord;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

typedef struct
    {
    unsigned int            uMsgNum;
    uint32_t                uBytesRead;     // Offset into data buffer
    SuI106Ch10Header      * psuHeader;      // Pointer to the current header
    SuCan_ChanSpec        * psuChanSpec;    // Pointer to the channel-specific header
    SuCan_Header          * psuCanHdr;      // Pointer to the Intra-Packet header
    SuCan_IdWord          * psuCanIdWord;   // Pointer to the CAN Id Word
    SuIntraPacketTS       * psuIPTimeStamp; // Pointer to the Intra-Packet time stamp
    uint8_t               * pauData;        // Pointer to the data
    SuTimeRef               suTimeRef;
}  SuCan_CurrMsg;

/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL
    enI106_Decode_FirstCan(SuI106Ch10Header         * psuHeader,
                           void                     * pvBuff,
                           SuCan_CurrMsg            * psuCurrMsg);

EnI106Status I106_CALL_DECL
    enI106_Decode_NextCan(SuCan_CurrMsg    * psuCurrMsg);

#ifdef __cplusplus
}
}
#endif

#endif
