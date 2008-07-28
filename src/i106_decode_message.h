/****************************************************************************

 i106_decode_message.h - 

 Copyright (c) 2008 Irig106.org

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

 ****************************************************************************/

#ifndef _I106_DECODE_MESSAGE_H
#define _I106_DECODE_MESSAGE_H

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

typedef PUBLIC struct MessageF0_ChanSpec_S
    {
    uint32_t    uCounter        : 16;      // Message/segment counter
    uint32_t    uType           :  2;      // Complete/segment type
    uint32_t    uReserved       : 14;
#if !defined(__GNUC__)
    } SuMessageF0_ChanSpec;
#else
    } __attribute__ ((packed)) SuMessageF0_ChanSpec;
#endif


// Intra-message header
typedef struct MessageF0_Header
    {
    uint64_t    suIntraPckTime;            // Reference time
    uint32_t    uMsgLength      : 16;      // Message length
    uint32_t    uSubChannel     : 14;      // Subchannel number
    uint32_t    bFmtError       :  1;      // Format error flag
    uint32_t    bDataError      :  1;      // Data error flag
#if !defined(__GNUC__)
    } SuMessageF0_Header;
#else
    } __attribute__ ((packed)) SuMessageF0_Header;
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
