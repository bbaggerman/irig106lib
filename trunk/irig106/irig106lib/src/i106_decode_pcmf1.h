/****************************************************************************

 i106_decode_pcmf1.h - 

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
 Expanded by Hans-Gerhard Flohr, Hasotec GmbH, www.hasotec.de

 ****************************************************************************/

#ifndef _I106_DECODE_PCMF1_H
#define _I106_DECODE_PCMF1_H

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
    PCM_PARITY_NONE     = 0,
    PCM_PARITY_ODD      = 1,
    PCM_PARITY_EVEN     = 2,
} PCM_PARITY;

/* ----------------------------------------------------------------------- */
#ifndef d100NANOSECONDS
    #define d100NANOSECONDS     10000000.
#endif

#ifndef IsBitSetL2R
    // Bit 0: Starts at the most left position of the (byte) array (2exp7)
    // Caution: Don't use operators like '<<=', '++' etc for the BitPosition
    #define IsBitSetL2R(Array, BitPosition)   ((Array)[ ((BitPosition) >> 3) ] & 0x80 >> ((BitPosition) & 7) )
#endif

/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Channel specific data word
typedef struct PcmF1_S
    {
    uint32_t    uSyncOffset     : 18;      // Sync offset
    uint32_t    bUnpackedMode   :  1;      // Packed mode flag
    uint32_t    bPackedMode     :  1;      // Unpacked mode flag
    uint32_t    bThruMode       :  1;      // Throughput mode flag
    uint32_t    bAlignment      :  1;      // 16/32 bit alignment flag
    uint32_t    Reserved1       :  2;      // 
    uint32_t    uMajorFrStatus  :  2;      // Major frame lock status
    uint32_t    uMinorFrStatus  :  2;      // Minor frame lock status
    uint32_t    bMinorFrInd     :  1;      // Minor frame indicator
    uint32_t    bMajorFrInd     :  1;      // Major frame indicator
    uint32_t    bIntraPckHdr    :  1;      // Intra-packet header flag
    uint32_t    Reserved2       :  1;      // 
#if !defined(__GNUC__)
    } SuPcmF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuPcmF1_ChanSpec;
#endif

// Intra-message header
typedef struct PcmF1_IntraPktHeader
    {
    uint64_t    suIntraPckTime;            // Reference time
    uint32_t    Reserved1       : 12;      // 
    uint32_t    uMajorFrStatus  :  2;      // Major frame lock status
    uint32_t    uMinorFrStatus  :  2;      // Minor frame lock status
    uint32_t    Reserved2       : 16;      // 
#if !defined(__GNUC__)
    } SuPcmF1_IntraPktHeader;
#else
    } __attribute__ ((packed)) SuPcmF1_IntraPktHeader;
#endif


#if defined(_MSC_VER)
#pragma pack(pop)
#endif

// Channel attributes
// Note:
// The SuPcmF1_Attributes structure covers most of the information needed to decode raw Pcm data. 
// Only a part of the relevant data is supplied in the message SuPcmF1_ChanSpec.
// Most of the attributes must be imported from TMATS or supplied by another source.
typedef struct PcmF1_Attributes_S
    {
    int         iRecordNum;                 // P-x
    //          szDataLinkName;             // P-x\DLN
    //          szPcmCode;                  // P-x\D1
    uint32_t    uBitsPerSec;                // P-x\D2 number of bits per seconds
    //          szPolarity                  // P-x\D4
    //          szTypeFormat                // P-x\TF

    // Fx
    uint32_t    ulCommonWordLen;            // in bits P-x\F1
    // For throughput data Parity and ByteSwap must be filled externally, no definition in the current package of TMATS
    // Default is no parity, Parity transfer 0 (trailing), no byte swap 
    int32_t     WordTransferOrder;          // Msb (0)/ LSB (1, unsupported) P-x\F2
    int32_t     lParityType;                // Parity (0=none, 1= odd, 2= even) P-x\F3
    int32_t     lParityTransferOrder;       // Trailing (0) Leading (1) P-x\F4
    int32_t     lByteSwap;                  // Byte Swap on the input data

    // MFx
    uint32_t    ulNumMinorFrames;           // P-x\MF\N Number of MinorFrames
    uint32_t    ulWordsInMinorFrame;        // P-x\MF1 Words in Minor Frame
    uint32_t    ulBitsInMinorFrame;         // P-x\MF2 Bits in Minor Frame including syncword
    //          szMinorFrameSyncType;       // P-x\MF3
    uint32_t    ulMinorFrameSyncPatLen;     // P-x\MF4
    uint64_t    uMinorFrameSyncPat;         // P-x\MF5 Wordlen can be up to 64 bits, so let the sync word also 64 bits long
    // SYNCx
    uint32_t    ulMinSyncs;                 // P-x\SYNC1 Minimal number of syncs 0: first sync, 1: second sync etc.;
    // SYNC2 - SYNC4 not implemented

    // ISFx, IDCx, SFx not implemented 

    // Computed values 
    uint64_t    uMinorFrameSyncMask;        // Computed from P-x\MF4 (ulMinorFrameSyncPatLen)
    uint64_t    ullWordMask;                // Computed from P-x\F1
                                                
    double      dDelta100NanoSeconds;       // Computed from P-x\D2, the bits per sec
    int32_t     bFirstRun;                  // First bit flag for a complete decoding run: preload a minor frame sync word to the test word

    // The output buffers must be allocated if bFirstRun is notzero
    int32_t     ulOutBufSize;               // Size of the output buffer in (64-bit) words
    uint64_t    * paullOutBuf;              // Contains the decoded data of a minor frame
    uint8_t     * pauOutBufErr;             // Contains the error flags for a minor frame

    // Variables for bit decoding
    // Must be kept for the whole decoding run because the data 
    // may overlap the CH10 packets (at least in troughput mode)

    uint64_t    ullSyncCount;               // -1: Nothing found, 0: 1 sync found etc. analog to Min Syncs
    uint64_t    ullSyncErrors;              // Counter for statistics 
    uint64_t    ullTestWord;                // Currently collected word resp. syncword
    uint64_t    ullBitsLoaded;              // Bits already loaded (and shifted through) the TestWord. 
    // The amount must be at least the sync word len to check for a sync word
    uint32_t    ulBitPosition;              // Bit position in the current buffer
    uint32_t    ulMinorFrameBitCount;       // Counter for the number of bits in a minor frame (inclusive syncword)
    uint32_t    ulMinorFrameWordCount;      // Counter for the Minor frame words (inclusive syncword)
    uint32_t    ulDataWordBitCount;         // Counter for the bits of a data word
    int32_t     lSaveData;                  // Save the data (0: do nothing, 1 save, 2: save terminated)


#if !defined(__GNUC__)
    } SuPcmF1_Attributes;
#else
    } __attribute__ ((packed)) SuPcmF1_Attributes;
#endif

// Current PCM message
typedef struct
    {
    SuI106Ch10Header    * psuHeader;        // The overall packet header
    SuPcmF1_ChanSpec    * psuChanSpec;      // Header in the data stream
    SuPcmF1_Attributes  * psuAttributes;    // Pointer to the Pcm Format structure, values must be imported from TMATS 
                                            // or another source
    SuPcmF1_IntraPktHeader * psuIntraPktHdr;// Optional intra packet header, consists of the time 
    // suIntraPckTime (like SuIntraPacketTS) and the header itself
    unsigned int        uBytesRead;         // Number of bytes read in this message
    uint32_t            ulDataLen;          // Overall data packet length
    int64_t             llIntPktTime;       // Intrapacket or header time ! Relative Time !
    int64_t             llBaseIntPktTime;   // Intrapacket or header time ! Relative Time !
    uint32_t            ulSubPacketLen;     // MinorFrameLen in Bytes padded, see bAlignment. 
    // In throughput mode it's the length of the whole packet
    uint32_t            ulSubPacketBits;    // MinorFrameLen in Bits
    uint8_t             * pauData;          // Pointer to the start of the data
    SuTimeRef           suTimeRef;

#if !defined(__GNUC__)
    } SuPcmF1_CurrMsg;
#else
    } __attribute__ ((packed)) SuPcmF1_CurrMsg;
#endif

/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstPcmF1(SuI106Ch10Header     * psuHeader,
                                  void            * pvBuff,
                                  SuPcmF1_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextPcmF1(SuPcmF1_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    PcmF1FirstRun(SuPcmF1_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    DecodeMinorFrame_PcmF1(SuPcmF1_CurrMsg * psuMsg);

EnI106Status Set_PcmF1_Attributes(SuRDataSource * psuRDataSrc, SuPcmF1_Attributes * psuAttributes);

// Help functions

EnI106Status PcmCheckParity(uint64_t ullTestWord, int iWordLen, int iParityType, int iParityTransferOrder);

void PrepareNewMinorFrameCollection(SuPcmF1_Attributes * psuAttributes);
void GetNextBit(SuPcmF1_CurrMsg * psuMsg, SuPcmF1_Attributes * psuAttributes);
int IsSyncWordFound(SuPcmF1_Attributes * psuAttributes);
void RenewSyncCounters(SuPcmF1_Attributes * psuAttributes, uint64_t ullSyncCount);
EnI106Status SwapBytes_PcmF1(uint8_t *Buffer, long nBytes);


#ifdef __cplusplus
} // end namespcace
} // end extern c
#endif // __cplusplus
#endif // _I106_DECODE_PCMF1_H
