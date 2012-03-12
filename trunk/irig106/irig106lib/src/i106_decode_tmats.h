/****************************************************************************

 i106_decode_tmats.h - 

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

#ifndef _I106_DECODE_TMATS_H
#define _I106_DECODE_TMATS_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */

/// TMATS signature generating options
#define TMATS_SIGFLAG_NONE          0x0000
#define TMATS_SIGFLAG_INC_COMMENT   0x0001  ///< Include comment fields
#define TMATS_SIGFLAG_INC_VENDOR    0x0002  ///< Include vendor fields
#define TMATS_SIGFLAG_INC_ALL       0x000F  ///< Include all fields

/// TMATS signature version
#define TMATS_SIGVER_1              1
#define TMATS_SIGVER_DEFAULT        TMATS_SIGVER_1
#define TMATS_SIGVER_MAX            TMATS_SIGVER_1

/*
 * Data structures
 * ---------------
 */

// Channel specific data word
// --------------------------

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

typedef PUBLIC struct Tmats_ChanSpec_S
    {
    uint32_t    iCh10Ver        :  8;      // Recorder Ch 10 Version
    uint32_t    bConfigChange   :  1;      // Recorder configuration changed
    uint32_t    iFormat         :  1;      // TMATS / XML Format
    uint32_t    iReserved       : 22;      // Reserved
#if !defined(__GNUC__)
    } SuTmats_ChanSpec;
#else
    } __attribute__ ((packed)) SuTmats_ChanSpec;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

// NEED TO ADD STORAGE FOR REQUIRED DATA FIELDS
// NEED TO ADD SUPPORT OF "OTHER" DATA FIELDS TO PERMIT TMATS WRITE

// P Records
// ---------

typedef PUBLIC struct SuPRecord_S
    {
    int                         iRecordNum;             // P-x
    char                      * szDataLinkName;         // P-x\DLN
    char                      * szPcmCode;              // P-x\D1
    char                      * szBitsPerSec;           // P-x\D2
    char                      * szPolarity;             // P-x\D4
    char                      * szTypeFormat;           // P-x\TF
    char                      * szCommonWordLen;        // P-x\F1
    char                      * szNumMinorFrames;       // P-x\MF\N
    char                      * szWordsInMinorFrame;    // P-x\MF1
    char                      * szBitsInMinorFrame;     // P-x\MF2
    char                      * szMinorFrameSyncType;   // P-x\MF3
    char                      * szMinorFrameSyncPatLen; // P-x\MF4
    char                      * szMinorFrameSyncPat;    // P-x\MF5
    struct SuPRecord_S        * psuNextPRecord;
    } SuPRecord;

// B Records
// ---------

typedef PUBLIC struct SuBRecord_S
    {
    int                         iRecordNum;             // B-x
    char                      * szDataLinkName;         // B-x\DLN
    int                         iNumBuses;              // B-x\NBS\N
    struct SuBRecord_S        * psuNextBRecord;
    } SuBRecord;


// M Records
// ---------

typedef PUBLIC struct SuMRecord_S
    {
    int                         iRecordNum;             // M-x
//    char                      * szRecordNum;            // M-x
    char                      * szDataSourceID;         // M-x\ID
    char                      * szBBDataLinkName;       // M-x\BB\DLN
    char                      * szBasebandSignalType;   // M-x\BSG1
    struct SuPRecord_S        * psuPRecord;             // Corresponding P record
    struct SuBRecord_S        * psuBRecord;             // Corresponding B record
    struct SuMRecord_S        * psuNextMRecord;         // Used to keep track of M records
    } SuMRecord;


// R Records
// ---------

// R record data source
typedef PUBLIC struct SuRDataSource_S
    {
    int                         iDataSourceNum;         // R-x\XXX-n
    char                      * szDataSourceID;         // R-x\DSI-n
    char                      * szChannelDataType;      // R-x\CDT-n
    char                      * szTrackNumber;          // R-x\TK1-n
    int                         iTrackNumber;           // Only valid if szTrackNumber != NULL
    char                      * szEnabled;              // R-x\CHE-n
    int                         bEnabled;               // Only valid if szEnabled != NULL
    char                      * szPcmDataLinkName;      // R-x\PDLN-n (-04, -05)
    char                      * szBusDataLinkName;      // R-x\BDLN-n (-04, -05)
    char                      * szChanDataLinkName;     // R-x\CDLN-n (-07, -09)
    // Video channel attributes
    char                      * szVideoDataType;        // (R-x\VTF-n)
    char                      * szVideoEncodeType;      // (R-x\VXF-n)
    char                      * szVideoSignalType;      // (R-x\VST-n)
    char                      * szVideoSignalFormat;    // (R-x\VSF-n)
    char                      * szVideoConstBitRate;    // (R-x\CBR-n)
    char                      * szVideoVarPeakBitRate;  // (R-x\VBR-n)
    char                      * szVideoEncodingDelay;   // (R-x\VED-n)
    // PCM channel attributes
    char                      * szPcmDataTypeFormat;    // (R-x\PDTF-n)
    char                      * szPcmDataPacking;       // (R-x\PDP-n)
    char                      * szPcmInputClockEdge;    // (R-x\ICE-n)
    char                      * szPcmInputSignalType;   // (R-x\IST-n)
    char                      * szPcmInputThreshold;    // (R-x\ITH-n)
    char                      * szPcmInputTermination;  // (R-x\ITM-n)
    char                      * szPcmVideoTypeFormat;   // (R-x\PTF-n)
    // Analog channel attributes
    char                      * szAnalogChansPerPkt;    // (R-1\ACH\N-n)
    char                      * szAnalogSampleRate;     // (R-1\ASR-n)
    char                      * szAnalogDataPacking;    // (R-1\ADP-n)

    struct SuMRecord_S        * psuMRecord;             // Corresponding M record
    struct SuPRecord_S        * psuPRecord;             // Corresponding P record
    struct SuRDataSource_S    * psuNextRDataSource;
    } SuRDataSource;    

// R record
typedef PUBLIC struct SuRRecord_S
    {
    int                         iRecordNum;             // R-x
    char                      * szDataSourceID;         // R-x\ID
    char                      * szNumDataSources;       // R-x\N
    char                      * szIndexEnabled;         // R-x\IDX\E
    int                         bIndexEnabled;          // Only valid if szIndexEnabled != NULL
    char                      * szEventsEnabled;        // R-x\EVE\E
    int                         bEventsEnabled;         // Only valid if szEventsEnabled != NULL
    SuRDataSource             * psuFirstDataSource;     //
    struct SuRRecord_S        * psuNextRRecord;         // Used to keep track of R records
    } SuRRecord;


// G Records
// ---------

// G record, data source
typedef PUBLIC struct SuGDataSource_S
    {
    int                         iDataSourceNum;         // G\XXX-n
//    char                      * szDataSourceNum;        // G\XXX-n
    char                      * szDataSourceID;         // G\DSI-n
    char                      * szDataSourceType;       // G\DST-n
    struct SuRRecord_S        * psuRRecord;             // Corresponding R record
    struct SuGDataSource_S    * psuNextGDataSource;
    } SuGDataSource;

// G record
typedef PUBLIC struct GRecord_S
    {
    char                      * szProgramName;          // G\PN
    char                      * szIrig106Rev;           // G\106
//    int                         iNumDataSources;        // G\DSI\N
    char                      * szNumDataSources;       // G\DSI\N
    SuGDataSource             * psuFirstGDataSource;
    } SuGRecord;

// Memory linked list
// ------------------

// Linked list that keeps track of malloc'ed memory
typedef PUBLIC struct MemBlock_S
    {
    void                    * pvMemBlock;
    struct MemBlock_S       * psuNextMemBlock;
    } SuMemBlock;

// Decoded TMATS info
// ------------------

typedef PUBLIC struct SuTmatsInfo_S
    {
    int              iCh10Ver;
    int              bConfigChange;
    SuGRecord      * psuFirstGRecord;
    SuRRecord      * psuFirstRRecord;
    SuMRecord      * psuFirstMRecord;
    SuBRecord      * psuFirstBRecord;
    SuPRecord      * psuFirstPRecord;
    void           * psuFirstTRecord;
    void           * psuFirstDRecord;
    void           * psuFirstSRecord;
    void           * psuFirstARecord;
    void           * psuFirstCRecord;
    void           * psuFirstHRecord;
    void           * psuFirstVRecord;
    SuMemBlock     * psuFirstMemBlock;
    } SuTmatsInfo;

/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_Tmats(SuI106Ch10Header  * psuHeader,
                        void              * pvBuff,
                        SuTmatsInfo       * psuTmatsInfo);

EnI106Status I106_CALL_DECL 
    enI106_Decode_Tmats_Text(void         * pvBuff,
                             uint32_t       ulDataLen,
                             SuTmatsInfo  * psuTmatsInfo);
 
void I106_CALL_DECL 
    enI106_Free_TmatsInfo(SuTmatsInfo     * psuTmatsInfo);

I106_CALL_DECL EnI106Status 
    enI106_Encode_Tmats(SuI106Ch10Header  * psuHeader,
                        void              * pvBuff,
                        char              * szTMATS);

I106_CALL_DECL EnI106Status 
    enI106_Tmats_Signature(void         * pvBuff,       ///< TMATS text without CSDW
                           uint32_t       ulDataLen,    ///< Length of TMATS in pvBuff
                           int            iSigVersion,  ///< Request signature version (0 = default)
                           int            iSigFlags,    ///< Additional flags
                           uint8_t      * piOpCode,     ///< Version and flag op code
                           uint32_t     * piSignature); ///< TMATS signature
 
#ifdef __cplusplus
}
}
#endif

#endif
