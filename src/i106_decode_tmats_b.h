/****************************************************************************

 i106_decode_tmats_b.h - Decode TMATS B fields

 Copyright (c) 2020 Irig106.org

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

#ifndef _I106_DECODE_TMATS_B_H
#define _I106_DECODE_TMATS_B_H

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


// B Records
// =========

// B record data source structures
// -------------------------------


// User defined words
typedef PUBLIC struct SuBUserDefinedWords_S
    {
    char                      * szName;                 // B-x\UMN1-i
    char                      * szParity;               // B-x\U1P-i
    char                      * szParityTransferOrder;  // B-x\U1PT-i
    char                      * szBitMask;              // B-x\U1M-i
    char                      * szTransferOrder;        // B-x\U1T-i
    } SuBUserDefinedWords;

// Recording description track sequence
typedef PUBLIC struct SuBTrackSequence
    {
    int                         iIndex;
    char                      * szTrackSequenceNum;     // B-x\TS-i-k
    struct SuBTrackSequence   * psuNext;
    } SuBTrackSequence;

// Measurement Location
typedef PUBLIC struct SuBMeasurandLocation_S
    {
    int                         iIndex;
    char                      * szWordNumber;           // B-x\MWN-i-n-p-e
    char                      * szBitMask;              // B-x\MBM-i-n-p-e
    char                      * szTransferOrder;        // B-x\MTO-i-n-p-e
    char                      * szFragmentPosition;     // B-x\MFP-i-n-p-e

    struct SuBMeasurandLocation_S * psuNext;
    } SuBMeasurandLocation;

// Measurand Description Set
typedef PUBLIC struct SuBMeasurand_S
    {
    int                         iIndex;
    char                      * szName;                 // B-x\MN-i-n-p
    char                      * szType;                 // B-x\MT-i-n-p
    char                      * szParity;               // B-x\MN1-i-n-p
    char                      * szParityTransferOrder;  // B-x\MN2-i-n-p
    char                      * szNumberOfLocations;    // B-x\NML\N-i-n-p
    SuBMeasurandLocation      * psuFirstLocation;

    struct SuBMeasurand_S     * psuNext;
    } SuBMeasurand;


// Message Content Definition
typedef PUBLIC struct SuBMsgContentDef_S
    {
    int                         iIndex;
    char                      * szMsgNum;               // B-x\MID-i-n
    char                      * szMsgName;              // B-x\MNA-i-n
    char                      * szCmdWordEntry;         // B-x\CWE-i-n
    char                      * szCmdWord;              // B-x\CMD-i-n
    char                      * szRTName;               // B-x\TRN-i-n
    char                      * szRTAddress;            // B-x\TRA-i-n
    char                      * szSubterminalName;      // B-x\STN-i-n
    char                      * szSubterminalAddr;      // B-x\STA-i-n
    char                      * szTRMode;               // B-x\TRM-i-n
    char                      * szWordCntModeCode;      // B-x\DWC-i-n
    char                      * szSpecialProcessing;    // B-x\SPR-i-n

    // ARINC 429
    struct
        {
        char                  * szLabel;                // B-x\LBL-i-n
        char                  * szSdiCode;              // B-x\SDI-i-n
        } suARINC429;

    // RT/RT Receive Command List
    struct
        {
        char                  * szRcvCmdWordEntry;      // B-x\RCWE-i-n
        char                  * szRcvCmdWord;           // B-x\RCMD-i-n
        char                  * szRTName;               // B-x\RTRN-i-n
        char                  * szRTAddress;            // B-x\RTRA-i-n
        char                  * szSubterminalName;      // B-x\RSTN-i-n
        char                  * szSubterminalAddr;      // B-x\RSTA-i-n
        char                  * szWordCnt;              // B-x\RDWC-i-n
        } suRTRTRcvCmdList;

    // Mode Code
    struct
        {
        char                  * szModeCodeDescrip;      // B-x\MCD-i-n
        char                  * szDataWordDescrip;      // B-x\MCW-i-n
        } suModeCode;

    // Measurement Description Set
    char                      * szNumOfMeasurands;      // B-x\MN\N-i-n
    SuBMeasurand              * psuFirstMeasurand;

    struct SuBMsgContentDef_S * psuNext;
    } SuBMsgContentDef;

// Bus definition
typedef PUBLIC struct SuBBusInfo_S
    {
    int                         iIndex;
    char                      * szBusNum;               // B-x\BID-i
    char                      * szBusName;              // B-x\BNA-i
    char                      * szBusType;              // B-x\BT-i

    // User Defined Words
    SuBUserDefinedWords         suUserDefinedMeasurement1;
    SuBUserDefinedWords         suUserDefinedMeasurement2;
    SuBUserDefinedWords         suUserDefinedMeasurement3;

    // Recording Description
    char                      * szNumOfTracks;          // B-x\TK\N-i
    SuBTrackSequence          * psuFirstTrackSequence;

    // Message Content Definition
    char                      * szNumOfMessages;        // B-x\NMS\N-i
    SuBMsgContentDef          * psuFirstMsgContentDef;

    struct SuBBusInfo_S       * psuNext;
    } SuBBusInfo;

// B record
typedef PUBLIC struct SuBRecord_S
    {
    int                         iIndex;                 // B-x
    char                      * szDataLinkName;         // B-x\DLN
    char                      * szTestItem;             // B-x\TA
    char                      * szBusParity;            // B-x\BP
    char                      * szNumBuses;             // B-x\NBS\N

    SuBBusInfo                * psuFirstBBusInfo;

    SuComment                 * psuFirstComment;        // B-x\COM

    struct SuBRecord_S        * psuNext;                // Next record in linked list
    } SuBRecord;


/*
 * Function Declaration
 * --------------------
 */

int bDecodeBLine(char * szCodeName, char * szDataItem, SuBRecord ** ppsuFirstBRec);

#ifdef __cplusplus
}
}
#endif


#endif
