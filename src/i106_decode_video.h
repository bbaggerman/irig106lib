/****************************************************************************

 i106_decode_video.h - Video message decoding routines

 Copyright (c) 2006 Irig106.org

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

#ifndef _I106_DECODE_VIDEO_H
#define _I106_DECODE_VIDEO_H

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

/* Video Format 0 */

/// Video Format 0 channel specific header
typedef struct 
    {
    uint32_t    Reserved     : 24;
    uint32_t    uType        :  4;      ///< Payload type
    uint32_t    bKLV         :  1;      ///< KLV present
    uint32_t    bSRS         :  1;      ///< SCR/RTC Sync
    uint32_t    bIPH         :  1;      ///< Intra-Packet Header
    uint32_t    bET          :  1;      ///< Embedded Time
#if !defined(__GNUC__)
    } SuVideoF0_ChanSpec;
#else
    } __attribute__ ((packed)) SuVideoF0_ChanSpec;
#endif


/// Video Format 0 intra-packet header
typedef struct 
    {
    uint8_t     aubyIntPktTime[8];      ///< Reference time
#if !defined(__GNUC__)
    } SuVideoF0_Header;
#else
    } __attribute__ ((packed)) SuVideoF0_Header;
#endif


/// Current video format 0 data message
typedef struct
    {
    SuVideoF0_ChanSpec    * psuChanSpec;  ///< Pointer to channel specific header
    SuVideoF0_Header      * psuIPHeader;  ///< Pointer to intra-packet header
    uint8_t               * pachTSData;   ///< Pointer to transport stream data
#if !defined(__GNUC__)
    } SuVideoF0_CurrMsg;
#else
    } __attribute__ ((packed)) SuVideoF0_CurrMsg;
#endif


/* Video Format 1 */

/// Video Format 1 channel specific header
typedef struct 
    {
    uint32_t    uPacketCnt   : 12;      ///< Number of packets
    uint32_t    uType        :  1;      ///< TS/PS type
    uint32_t    uMode        :  1;      ///< Const/Var mode
    uint32_t    bET          :  1;      ///< Embedded Time
    uint32_t    uEPL         :  4;      ///< Encoding Profile and Level
    uint32_t    bIPH         :  1;      ///< Intra-Packet Header
    uint32_t    bSRS         :  1;      ///< SCR/RTC Sync
    uint32_t    bKLV         :  1;      ///< KLV present
    uint32_t    uReserved    : 10;
#if !defined(__GNUC__)
    } SuVideoF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuVideoF1_ChanSpec;
#endif


/// Video Format 2 channel specific header
typedef struct 
    {
    uint32_t    uPacketCnt   : 12;      ///< Number of packets
    uint32_t    uType        :  1;      ///< TS/PS type
    uint32_t    uMode        :  1;      ///< Const/Var mode
    uint32_t    bET          :  1;      ///< Embedded Time
    uint32_t    uEP          :  4;      ///< Encoding Profile
    uint32_t    bIPH         :  1;      ///< Intra-Packet Header
    uint32_t    bSRS         :  1;      ///< SCR/RTC Sync
    uint32_t    bKLV         :  1;      ///< KLV present
    uint32_t    uEL          :  4;      ///< Encoding Level
    uint32_t    uAET         :  1;      ///< Audio Encoding Type
    uint32_t    uReserved    :  5;
#if !defined(__GNUC__)
    } SuVideoF2_ChanSpec;
#else
    } __attribute__ ((packed)) SuVideoF2_ChanSpec;
#endif



#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstVideoF0(SuI106Ch10Header  * psuHeader,
                               void              * pvBuff,
                               SuVideoF0_CurrMsg * psuCurrMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextVideoF0 (SuI106Ch10Header  * psuHeader,
                               SuVideoF0_CurrMsg * psuCurrMsg);


#ifdef __cplusplus
}
}
#endif

#endif
