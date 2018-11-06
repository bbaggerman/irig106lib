/****************************************************************************

 i106_decode_ethernet.h - 

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

#ifndef _I106_DECODE_ETHERNET_H
#define _I106_DECODE_ETHERNET_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif


/*
 * Macros and definitions
 * ----------------------
 */

typedef enum
    {
    I106_ENET_FMT_PHYSICAL    =  0x00,
    } EnI106EthernetFmt;

typedef enum
    {
    I106_ENET_CONTENT_FULLMAC =  0x00,
    } EnI106EthernetContent;

typedef enum
    {
    I106_ENET_SPEED_AUTO      =  0x00,
    I106_ENET_SPEED_10MBPS    =  0x01,
    I106_ENET_SPEED_100MBPS   =  0x02,
    I106_ENET_SPEED_1GBPS     =  0x03,
    I106_ENET_SPEED_10GBPS    =  0x04,
    } EnI106EthernetSpeed;


/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Ethernet Format 0
// -----------------

// Channel specific data word
typedef struct EthernetF0_ChanSpec_S
    {
    uint32_t    uNumFrames      : 16;      // Number of frames
    uint32_t    Reserved1       :  9;
    uint32_t    uTTB            :  3;      // Time tag bits
    uint32_t    uFormat         :  4;      // Format of frames
#if !defined(__GNUC__)
    } SuEthernetF0_ChanSpec;
#else
    } __attribute__ ((packed)) SuEthernetF0_ChanSpec;
#endif

// Intra-message header
typedef struct EthernetF0_Header_S
    {
    uint8_t     aubyIntPktTime[8];         // Reference time
    uint32_t    uMsgDataLen     : 14;      // Message data length
    uint32_t    bLengthError    :  1;      // Data length error
    uint32_t    bDataCrcError   :  1;      // Data CRC error
    uint32_t    uNetID          :  8;      // Network identifier
    uint32_t    uSpeed          :  4;      // Ethernet speed
    uint32_t    uContent        :  2;      // Captured data content
    uint32_t    bFrameError     :  1;      // Frame error
    uint32_t    bFrameCrcError  :  1;      // Frame CRC error
#if !defined(__GNUC__)
    } SuEthernetF0_Header;
#else
    } __attribute__ ((packed)) SuEthernetF0_Header;
#endif


// Ethernet physical frame
typedef struct
    {
    uint8_t                 abyDestAddr[6]; // Destination address
    uint8_t                 abySrcAddr[6];  // Source address
    uint16_t                uTypeLen;       // Ethernet type / 802.3 length, byte swapped!
    uint8_t                 abyData[1];     // Start of the data
#if !defined(__GNUC__)
    } SuEthernetF0_Physical_FullMAC;
#else
    } __attribute__ ((packed)) SuEthernetF0_Physical_FullMAC;
#endif


// Current Ethernet message
typedef struct
    {
    unsigned int            uFrameNum;
    uint32_t                ulPktDataLen;   // Overall data packet length
    SuEthernetF0_ChanSpec * psuChanSpec;
    SuEthernetF0_Header   * psuEthernetF0Hdr;
    uint8_t               * pauData;
#if !defined(__GNUC__)
    } SuEthernetF0_CurrMsg;
#else
    } __attribute__ ((packed)) SuEthernetF0_CurrMsg;
#endif


// Ethernet Format 1 ARINC-664
// ---------------------------

// Channel specific data word
typedef struct EthernetF1_ChanSpec_S
    {
    uint32_t    uNumFrames      : 16;       // Number of frames
    uint32_t    uIPHLength      : 16;       // Intra-packet header length
#if !defined(__GNUC__)
    } SuEthernetF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuEthernetF0_ChanSpec;
#endif

// Intra-packet data header
typedef struct EthernetF1_Header_S
    {
    uint8_t     aubyIntPktTime[8];          // Reference time
    uint32_t    uFlagBits       :  8;       // Flag bits
    uint32_t    uErrorBits      :  8;       // Error bits
    uint32_t    uMsgDataLen     : 16;       // Message data length
    uint32_t    uVirtualLinkID  : 16;       // Virtual link
    uint32_t    Reserved1       :  8;       // 
    uint8_t     auSrcIP[4];                 // Source IP address
    uint8_t     auDstIP[4];                 // Source IP address
    uint32_t    uDstPort        : 16;       // Destination Port
    uint32_t    uSrcPort        : 16;       // Source Port
#if !defined(__GNUC__)
    } SuEthernetF1_Header;
#else
    } __attribute__ ((packed)) SuEthernetF0_Header;
#endif

// Current ARINC-664 message
typedef struct
    {
    unsigned int            uFrameNum;
    uint32_t                ulPktDataLen;           // Overall data packet length
    SuEthernetF1_ChanSpec * psuChanSpec;
    SuEthernetF1_Header   * psuEthernetF1Hdr;
    uint8_t               * pauData;
    uint8_t               * puSequenceNum;
#if !defined(__GNUC__)
    } SuEthernetF1_CurrMsg;
#else
    } __attribute__ ((packed)) SuEthernetF0_CurrMsg;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif





/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstEthernetF0(SuI106Ch10Header     * psuHeader,
                                  void                 * pvBuff,
                                  SuEthernetF0_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextEthernetF0(SuEthernetF0_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstEthernetF1(SuI106Ch10Header     * psuHeader,
                                  void                 * pvBuff,
                                  SuEthernetF1_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextEthernetF1(SuEthernetF1_CurrMsg * psuMsg);

#ifdef __cplusplus
}
}
#endif

#endif
