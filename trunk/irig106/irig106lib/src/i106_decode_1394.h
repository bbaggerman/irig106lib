/****************************************************************************

 i106_decode_1394.h - 

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

#ifndef _I106_DECODE_1394_H
#define _I106_DECODE_1394_H

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

/* IEEE 1394 Format 0 */

// Channel specific header
typedef struct 
    {
    uint32_t    uTransCnt    : 16;      // Transaction count
    uint32_t    Reserved     :  9;
    uint32_t    uSyncCode    :  4;      // Synchronization code
    uint32_t    uPacketType  :  3;      // Packet body type
#if !defined(__GNUC__)
    } Su1394F0_ChanSpec;
#else
    } __attribute__ ((packed)) Su1394F0_ChanSpec;
#endif

/* IEEE 1394 Format 1 */

// Channel specific header
typedef struct 
    {
    uint32_t    uPacketCnt   : 16;      // Number of messages
    uint32_t    Reserved     : 16;
#if !defined(__GNUC__)
    } Su1394F1_ChanSpec;
#else
    } __attribute__ ((packed)) Su1394F1_ChanSpec;
#endif

// Intra-message header
typedef struct 
    {
    uint8_t     aubyIntPktTime[8];      // Reference time
    uint32_t    uDataLength  : 16;      // 
    uint32_t    Reserved     :  1;      // 
    uint32_t    bLBO         :  1;      // Local buffer overflow
    uint32_t    uTrfOvf      :  2;      // Transfer overflow
    uint32_t    uSpeed       :  4;      // Transmission speed
    uint32_t    uStatus      :  8;      // Status byte

#if !defined(__GNUC__)
    } Su1394F1_Header;
#else
    } __attribute__ ((packed)) Su1394F1_Header;
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
