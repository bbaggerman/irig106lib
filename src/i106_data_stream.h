/****************************************************************************

 i106_data_stream.h - A module to decode Chapter 10 UDP data streaming

 Copyright (c) 2011 Irig106.org

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

#ifndef _I106_DATA_STREAMING_H
#define _I106_DATA_STREAMING_H

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

/* UDP Streaming Format 1 */

/// UDP Transfer Header Format 1 - Non-segmented
typedef struct 
    {
    uint32_t    uFormat         :  4;   ///< Streaming version
    uint32_t    uMsgType        :  4;   ///< Type of message
    uint32_t    uUdpSeqNum      : 24;   ///< UDP sequence number
    uint8_t     achData[1];             ///< Start of Ch 10 data packet
#if !defined(__GNUC__)
    } SuUDP_Transfer_Header_F1_NonSeg;
#else
    } __attribute__ ((packed)) SuUDP_Transfer_Header_F1_NonSeg;
#endif

enum { UDP_Transfer_Header_F1_NonSeg_Len = sizeof(SuUDP_Transfer_Header_F1_NonSeg) - 1 };

// UDP Transfer Header Format 1 - Segmented
typedef struct 
    {
    uint32_t    uFormat         :  4;   ///< Streaming version
    uint32_t    uMsgType        :  4;   ///< Type of message
    uint32_t    uUdpSeqNum      : 24;   ///< UDP sequence number
    uint32_t    uChID           : 16;   ///< Channel ID
    uint32_t    uChanSeqNum     :  8;   ///< Channel sequence number
    uint32_t    uReserved       :  8;
    uint32_t    uSegmentOffset;         ///< Segment offset
    uint8_t     achData[1];             ///< Start of Ch 10 data packet
#if !defined(__GNUC__)
    } SuUDP_Transfer_Header_F1_Seg;
#else
    } __attribute__ ((packed)) SuUDP_Transfer_Header_F1_Seg;
#endif

enum { UDP_Transfer_Header_Seg_Len = sizeof(SuUDP_Transfer_Header_F1_Seg) - 1 };

/* UDP Streaming Format 2 */

/// UDP Transfer Header Format 2
typedef struct 
    {
    uint32_t    uFormat         :  4;   ///< Streaming version
    uint32_t    uMsgType        :  4;   ///< Type of message
    uint32_t    uUdpSeqNum      : 24;   ///< UDP sequence number
    uint32_t    uPacketSize     : 24;   ///< Packet size in bytes
    uint32_t    uSegmentOffset1 :  8;
    uint32_t    uChanID         : 16;
    uint32_t    uSegmentOffset2 : 16;
    uint8_t     achData[1];             // Start of Ch 10 data packet
#if !defined(__GNUC__)
    } SuUDP_Transfer_Header_F2;
#else
    } __attribute__ ((packed)) SuUDP_Transfer_Header_F2;
#endif


/* UDP Streaming Format 3 */

/// UDP Transfer Header Format 3
typedef struct 
    {
    uint32_t    uFormat         :  4;   ///< Streaming version
    uint32_t    uSrcIdLen       :  4;   ///< Type of message
    uint32_t    uReserved       :  8;
    uint32_t    uPktStartOffset : 16;   ///< Start of Ch 11 packet
    uint32_t    uUdpSeqNum      : 24;   ///< UDP sequence number
    uint32_t    uSrcId          :  8;   ///< Source identifier
    uint8_t     achData[1];             ///< Start of Ch 10 data packet
#if !defined(__GNUC__)
    } SuUDP_Transfer_Header_F3;
#else
    } __attribute__ ((packed)) SuUDP_Transfer_Header_F3;
#endif


#if defined(_MSC_VER)
#pragma pack(pop)
#endif

/*
 * Function Declaration
 * --------------------
 */


// Open / Close

EnI106Status I106_CALL_DECL
    enI106_OpenNetStreamRead(int                 iHandle,
                             uint16_t            uPort);

EnI106Status I106_CALL_DECL
    enI106_OpenNetStreamWrite(int iHandle, 
                              uint32_t          uIpAddress,
                              uint16_t          uUdpPort);

EnI106Status I106_CALL_DECL
    enI106_CloseNetStream(int                 iHandle);


// Read
// ----

int I106_CALL_DECL
    enI106_ReadNetStream(int            iHandle,
                         void         * pvBuffer,
                         uint32_t       uBuffSize);

// Manipulate receive buffer
// -------------------------

EnI106Status I106_CALL_DECL
    enI106_DumpNetStream(int iHandle);

EnI106Status I106_CALL_DECL
    enI106_MoveReadPointer(int iHandle, long iRelOffset);

// Write
// -----

EnI106Status I106_CALL_DECL
    enI106_WriteNetStream(
            int           iHandle,
            void        * pvBuffer,
            uint32_t      uBuffSize);

EnI106Status I106_CALL_DECL
    enI106_WriteNetNonSegmented(
            int           iHandle,
            void        * pvBuffer,
            uint32_t      uBuffSize);

EnI106Status I106_CALL_DECL
    enI106_WriteNetSegmented(
            int           iHandle,
            void        * pvBuffer,
            uint32_t      uBuffSize);

#ifdef __cplusplus
}
}
#endif

#endif
