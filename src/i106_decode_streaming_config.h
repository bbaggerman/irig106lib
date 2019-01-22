/****************************************************************************

 i106_decode_streaming_config.h - Decode computer Generated Data Packet
   Format 4 Streaming Configuration Records

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

#ifndef _I106_DECODE_STREAMING_CONFIG_H
#define _I106_DECODE_STREAMING_CONFIG_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */

// Streaming config data payload formats
#define STREAM_CONFIG_FORMAT_ASCII_TMATS        0X00
#define STREAM_CONFIG_FORMAT_XML_TMATS          0X01
#define STREAM_CONFIG_FORMAT_ASCII_TMATS_SEG    0X02
#define STREAM_CONFIG_FORMAT_XML_TMATS_SEG      0X03
#define STREAM_CONFIG_FORMAT_CHECKSUM           0X04
#define STREAM_CONFIG_FORMAT_CHANNELS           0X05

/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

/// Streaming Config Channel Specific Data Word
typedef struct 
    {
    uint32_t    iCh10Ver        :  8;      // Recorder Ch 10 Version
    uint32_t    iFormat         :  7;      // Config contents
    uint32_t    bLast           :  1;      // Last packet flag
    uint32_t    iReserved       : 16;      // Reserved
#if !defined(__GNUC__)
    } SuStreamingConfig_ChanSpec;
#else
    } __attribute__ ((packed)) SuStreamingConfig_ChanSpec;
#endif

// Steaming config checksum packet layout

typedef struct
    {
    uint32_t    auChecksum[8];              // Checksum bits 0 to 255
#if !defined(__GNUC__)
    } SuStreamingConfig_Checksum;
#else
    } __attribute__ ((packed)) SuStreamingConfig_Checksum;
#endif
    

// Steaming config selected channels packet layout

typedef struct
    {
    uint16_t    iChannels;                  // Number of channels to follow
    uint16_t    auChannelId[1];             // Array of Channel IDs
#if !defined(__GNUC__)
    } SuStreamingConfig_Channels;
#else
    } __attribute__ ((packed)) SuStreamingConfig_Channels;
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
