/****************************************************************************

 i106_decode_index.h - Computer generated data format 3 recording index

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

#ifndef _I106_DECODE_INDEX_H
#define _I106_DECODE_INDEX_H

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

// Channel specific data word
// --------------------------

typedef struct Index_ChanSpec_S
    {
    uint32_t    uIdxEntCount    : 16;   // Total number of indexes
    uint32_t    uReserved       : 13;
    uint32_t    bIntraPckHdr    :  1;   // Intra-packet header present
    uint32_t    bFileSize       :  1;   // File size present
    uint32_t    uIndexType      :  1;   // Index type
#if !defined(__GNUC__)
    } SuIndex_ChanSpec;
#else
    } __attribute__ ((packed)) SuIndex_ChanSpec;
#endif

// Index time
typedef union Index_Time_S
    {
    SuIntraPacketRtc        suRtcTime;  // RTC format time stamp
    SuI106Ch4_Binary_Time   suCh4Time;  // Ch 4 format time stamp
    SuIEEE1588_Time         su1588Time; // IEEE-1588 format time stamp
    uint64_t                llTime;     // Generic 8 byte time
#if !defined(__GNUC__)
    } SuIndex_Time;
#else
    } __attribute__ ((packed)) SuIndex_Time;
#endif


// Node Index
// ----------

// Node index data
typedef struct Index_NodeData_S
    {
    uint32_t    uChannelID      : 16;
    uint32_t    uDataType       :  8;
    uint32_t    uReserved       :  8;
    uint64_t    uOffset;
#if !defined(__GNUC__)
    } SuIndex_NodeData;
#else
    } __attribute__ ((packed)) SuIndex_NodeData;
#endif


// Node index entry
typedef struct Index_NodeEntry_S
    {
    SuIndex_Time                suTime;
    SuIndex_NodeData            suData;     // Data about the event
#if !defined(__GNUC__)
    } SuIndex_NodeEntry;
#else
    } __attribute__ ((packed)) SuIndex_NodeEntry;
#endif


// Node index packet
typedef struct Index_NodePacket_S
    {
    SuIndex_ChanSpec    IndexCSDW;
    SuIndex_NodeEntry   NodeEntry[1];
#if !defined(__GNUC__)
    } SuIndex_NodePacket;
#else
    } __attribute__ ((packed)) SuIndex_NodePacket;
#endif

// Root Index
// ----------

// Root index entry
typedef struct Index_RootEntry_S
    {
    SuIndex_Time                suTime;
    uint64_t                    uOffset;    // Offset to node packet
#if !defined(__GNUC__)
    } SuIndex_RootEntry;
#else
    } __attribute__ ((packed)) SuIndex_RootEntry;
#endif


typedef struct Index_RootPacket_S
    {
    SuIndex_ChanSpec    IndexCSDW;
    SuIndex_RootEntry   RootEntry[1];
#if !defined(__GNUC__)
    } SuIndex_RootPacket;
#else
    } __attribute__ ((packed)) SuIndex_RootPacket;
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
