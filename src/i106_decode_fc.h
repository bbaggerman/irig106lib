/****************************************************************************

 i106_decode_fc.h - Decode Fibre Channel data

 Copyright (c) 2019 Irig106.org

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

#ifndef _I106_DECODE_FC_H
#define _I106_DECODE_FC_H

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

/* Fibre Channel Format 0 */

/// Fibre Channel Format 0 Channel Specific Data Data Header
typedef struct 
    {
    uint32_t    uNumFrames      : 16;      ///< Number of frames
    uint32_t    Reserved        : 12;
    uint32_t    uFormat         :  4;      ///< Frame data format
#if !defined(__GNUC__)
    } SuFibreChanF0_ChanSpec;
#else
    } __attribute__ ((packed)) SuFibreChanF0_ChanSpec;
#endif

/// Fibre Channel Format 0 Intra-packet header
typedef struct 
    {
    uint8_t     aubyIntPktTime[8];      ///< Reference time
    uint32_t    uFrameLen       : 12;   ///< Frame length in bytes
    uint32_t    uStartOfFrame   :  4;   ///< Start of frame delimiter
    uint32_t    uEndOfFrame     :  3;   ///< End of frame delimiter
    uint32_t    Reserved        :  5;
    uint32_t    uTopology       :  2;   ///< Fibre Channel topology
    uint32_t    uContent        :  2;   ///< Fibre Channel frame content
    uint32_t    bNonStrippedMode : 1;   ///< Stripped / Non-stripped mode flag
    uint32_t    bOverrunError   :  1;   ///< Overrun error flag
    uint32_t    bCrcError       :  1;   ///< CRC error flag
    uint32_t    bFramingError   :  1;   ///< Framing error flag
#if !defined(__GNUC__)
    } SuFibreChanF0_Header;
#else
    } __attribute__ ((packed)) SuFibreChanF0_Header;
#endif

/* Fibre Channel Format 1 */

/// Fibre Channel Format 1 Channel Specific Data Data Header
typedef struct 
    {
    uint32_t    uNumFrames      : 16;      ///< Number of frames
    uint32_t    Reserved        : 16;
#if !defined(__GNUC__)
    } SuFibreChanF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuFibreChanF1_ChanSpec;
#endif

/// Fibre Channel Format 1 Intra-packet header
typedef struct 
    {
    uint8_t     aubyIntPktTime[8];      ///< Reference time
    uint32_t    uFrameLen       : 12;   ///< Frame length in bytes
    uint32_t    uStartOfFrame   :  4;   ///< Start of frame delimiter
    uint32_t    uEndOfFrame     :  3;   ///< End of frame delimiter
    uint32_t    Reserved        : 10;
    uint32_t    bOverrunError   :  1;   ///< Overrun error flag
    uint32_t    bCrcError       :  1;   ///< CRC error flag
    uint32_t    bFramingError   :  1;   ///< Framing error flag
#if !defined(__GNUC__)
    } SuFibreChanF1_Header;
#else
    } __attribute__ ((packed)) SuFibreChanF1_Header;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif



/*
 * Function Declaration
 * --------------------
 */


#ifdef __cplusplus
}
}
#endif

#endif
