/****************************************************************************

 i106_decode_tmats_p.h - Decode TMATS P fields

 Copyright (c) 2018 Irig106.org

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

#ifndef _I106_DECODE_TMATS_P_H
#define _I106_DECODE_TMATS_P_H

#include "i106_decode_tmats_common.h"
//#include "i106_decode_tmats.h"

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

// P Records
// ---------

// Asynchronous embedded format location
typedef PUBLIC struct SuPAsyncEmbeddedLocationDef_S
    {
    int                         iIndex;                 // w
    char                      * szWordLocation;         //LOCATION (P-d\AEF3-n-w)
    char                      * szWordLength;           //WORD LENGTH (P-d\AEF5-n-w)
    char                      * szWordMask;             //MASK (P-d\AEF6-n-w)
    struct SuPAsyncEmbeddedLocationDef_S * psuNext;
    } SuPAsyncEmbeddedLocationDef;

// Asynchronous embedded subcommutated frame location
typedef PUBLIC struct SuPAsyncEmbeddedSubcomLocation_S
    {
    int                         iIndex;                 // m
    char                      * szStartFrame;           //START FRAME (P-d\AEF8-n-w-m)
    char                      * szFrameInterval;        //FRAME INTERVAL (P-d\AEF9-n-w-m)
    char                      * szEndFrame;             //END FRAME (P-d\AEF10-n-w-m)
    struct SuPAsyncEmbeddedSubcomLocation_S * psuNext;
    } SuPAsyncEmbeddedSubcomLocation;

// Asynchronous embedded subcommutate subframes
typedef PUBLIC struct SuPAsyncEmbeddedSubcom_S
    {
    int                         iIndex;                 // w
    char                      * szSubcommutated;        //SUBCOMMUTATED (P-d\AEF7-n-w)
    struct SuPAsyncEmbeddedSubcomLocation * psuFirstSubcomFrameLocation;
    struct SuPAsyncEmbeddedSubcom_S       * psuNext;
    } SuPAsyncEmbeddedSubcom;

// Asynchronous Embedded Streams definitions
typedef PUBLIC struct SuPAsyncEmbedded_S
    {
    int                         iIndex;                 // n
    char                      * szDataLinkName;         // P-x\AEF\DLN-n
    char                      * szSupercomutated;       //SUPERCOM (P-d\AEF1-n)
    char                      * szLocationDefinition;   //LOCATION DEFINITION (P-d\AEF2-n)
    struct SuPAsyncEmbeddedLocationDef_S * psuFirstAsyncLocation;
    char                      * szInterval;             //INTERVAL (P-d\AEF4-n)
    struct SuPRecord_S        * psuPRecord;             // Corresponding P record
    struct SuPAsyncEmbedded_S * psuNext;
    } SuPAsyncEmbedded;



// Subframe ID Counter definitions
// Note: Subframe definitions go away starting in 106-11

typedef PUBLIC struct SuPSubframeLoc_S
    {
    int                         iIndex;                 // n
    char                      * szSubframeLocation;     // P-x\SF4-x-x-n
    struct SuPSubframeLoc_S   * psuNext;
    } SuPSubframeLoc;
    
typedef PUBLIC struct SuPSubframeDef_S
    {
    int                         iIndex;                 // n
    char                      * szSubframeName;         // P-x\SF1-x-n
    char                      * szSuperComPosition;     // P-x\SF2-x-n
    char                      * szSuperComDefined;      // P-x\SF3-x-n
    SuPSubframeLoc            * psuFirstSubframeLoc;    // P-x\SF4-x-x-x
    char                      * szLocationInterval;     // P-x\SF5-x-n
    char                      * szSubframeDepth;        // P-x\SF6-x-n

    struct SuPSubframeDef_S   * psuNext;
    } SuPSubframeDef;
    
typedef PUBLIC struct SuPSubframeId_S
    {
    int                         iIndex;                 // n
    char                      * szCounterName;          // P-x\ISF1-n
    char                      * szCounterType;          // P-x\ISF2-n
    char                      * szWordPosition;         // P-x\IDC1-n
    char                      * szWordLength;           // P-x\IDC2-n
    char                      * szBitLocation;          // P-x\IDC3-n
    char                      * szCounterLen;           // P-x\IDC4-n
    char                      * szEndian;               // P-x\IDC5-n
    char                      * szInitValue;            // P-x\IDC6-n
    char                      * szMFForInitValue;       // P-x\IDC7-n
    char                      * szEndValue;             // P-x\IDC8-n
    char                      * szMFForEndValue;        // P-x\IDC9-n
    char                      * szCountDirection;       // P-x\IDC10-n

    // Only used thru 106-09
    char                      * szNumSubframeDefs;      // P-x\SF\N-n
    SuPSubframeDef            * psuFirstSubframeDef;
    
    struct SuPSubframeId_S    * psuNext;
    } SuPSubframeId;
    

// Chapter 7 segment definitions
typedef PUBLIC struct SuPCh7Segment_S
    {
    int                         iIndex;                 // n
    char                      * szCh7FirstWord;         // P-d\C7FW-n
    char                      * szNumberOfPcmWords;     // (P-d\C7NW-n)
    struct SuPCh7Segment_S    * psuNext;
    } SuPCh7Segment;



// P Record definition

typedef PUBLIC struct SuPRecord_S
    {
    int                         iIndex;                 // x
    char                      * szDataLinkName;         // P-x\DLN
    char                      * szPcmCode;              // P-x\D1
    char                      * szBitsPerSec;           // P-x\D2
    char                      * szPolarity;             // P-x\D4
    char                      * szAutoPolarityCorrection; // P-d\D5
    char                      * szDataDirection;        // P-d\D6
    char                      * szDataRandomized;       // P-d\D7
    char                      * szRandomizerLength;     // P-d\D8
    char                      * szTypeFormat;           // P-x\TF
    char                      * szCommonWordLen;        // P-x\F1
    char                      * szWordTransferOrder;    // P-x\F2 most significant bit "M", least significant bit "L". default: M
    char                      * szParityType;           // P-x\F3 even "EV", odd "OD", or none "NO", default: none
    char                      * szParityTransferOrder;  // P-x\F4 leading "L", default: trailing
    char                      * szCRC;                  // P-d\CRC
    char                      * szCRCCheckWordStartBit; // P-d\CRCCB
    char                      * szCRCDataStartBit;      // P-d\CRCDB
    char                      * szCRCDataNumOfBits;     // P-d\CRCDN
    char                      * szNumMinorFrames;       // P-x\MF\N
    char                      * szWordsInMinorFrame;    // P-x\MF1
    char                      * szBitsInMinorFrame;     // P-x\MF2
    char                      * szMinorFrameSyncType;   // P-x\MF3
    char                      * szMinorFrameSyncPatLen; // P-x\MF4
    char                      * szMinorFrameSyncPat;    // P-x\MF5
    char                      * szInSyncCrit;           // P-x\SYNC1
    char                      * szInSyncErrors;         // P-x\SYNC2
    char                      * szOutSyncCrit;          // P-x\SYNC3
    char                      * szOutSyncErrors;        // P-x\SYNC4
    char                      * szFillBits;             //FILL BITS (P-d\SYNC5)

//NUMBER OF UNIQUE WORD SIZES (P-d\MFW\N)
//WORD NUMBER (P-d\MFW1-n)
//NUMBER OF BITS IN WORD (P-d\MFW2-n)

    char                      * szNumSubframeCounters;  // P-x\ISF\N
    SuPSubframeId             * psuFirstSubframeId;     // Link to Subframe ID Counter defs
    
    char                      * szNumAsyncEmbedded;     // P-x\AEF\N  <-- ADD THIS ONE
    SuPAsyncEmbedded          * psuFirstAsyncEmbedded;  // Link to embedded stream defs

//Frame Format Identifier
//LOCATION (P-d\FFI1)
//MASK (P-d\FFI2)

//Measurement List Change
//NUMBER OF MEASUREMENT LISTS (P-d\MLC\N)
//FFI PATTERN (P-d\MLC1-n)
//MEASUREMENT LIST NAME (P-d\MLC2-n)
//Format Structure Change 
//NUMBER OF FORMATS (P-d\FSC\N)
//FFI PATTERN (P-d\FSC1-n)
//DATA LINK ID (P-d\FSC2-n)

//Alternate Tag And Data
//NUMBER OF TAGS (P-d\ALT\N)
//NUMBER OF BITS IN TAG (P-d\ALT1)
//NUMBER OF BITS IN DATA WORD (P-d\ALT2)
//FIRST TAG LOCATION (P-d\ALT3)
//SEQUENCE (P-d\ALT4)

//Asynchronous Data Merge Format
//NUMBER OF ASYNCHRONOUS DATA MERGE FORMATS (P-d\ADM\N)
//DATA MERGE NAME (P-d\ADM\DMN-n)
//MASK AND PATTERN (P-d\ADM\MP-n)
//OVERHEAD MASK (P-d\ADM\OHM-n)
//FRESH DATA PATTERN (P-d\ADM\FDP-n)
//DATA OVERFLOW PATTERN (P-d\ADM\DOP-n)
//STALE DATA PATTERN (P-d\ADM\SDP-n)
//USER DEFINED PATTERN (P-d\ADM\UDP-n)
//SUPERCOM (P-d\ADM1-n)
//LOCATION DEFINITION (P-d\ADM2-n)
//LOCATION (P-d\ADM3-n-w)
//INTERVAL (P-d\ADM4-n)
//DATA LENGTH (P-d\ADM5-n)
//MSB LOCATION (P-d\ADM6-n)
//PARITY (P-d\ADM7-n)
//SUBCOMMUTATED (P-d\ADM8-n-w)
//START FRAME (P-d\ADM9-n-w-m)
//FRAME INTERVAL (P-d\ADM10-n-w-m)

    //Chapter 7 Format
    char                      * szCh7NumSegments;       // P-x\C7\N
    SuPCh7Segment             * psuFirstCh7Segment;     // Link to Ch 7 segment def

    struct SuPRecord_S        * psuNext;
    } SuPRecord;


/*
 * Function Declaration
 * --------------------
 */

int bDecodePLine(char * szCodeName, char * szDataItem, SuPRecord ** ppsuFirstPRec);

#ifdef __cplusplus
}
}
#endif

#endif
