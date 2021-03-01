/****************************************************************************

 i106_decode_tmats_b.c - Decode TMATS B fields

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <assert.h>

#include "config.h"
#include "i106_stdint.h"

#include "irig106ch10.h"
#include "i106_decode_tmats_b.h"

#ifdef __cplusplus
namespace Irig106 {
#endif

/*
 * Macros and definitions
 * ----------------------
 */


/*
 * Data structures
 * ---------------
 */

/*
 * Module data
 * -----------
 */

/*
 * Function Declaration
 * --------------------
 */

// Make the routines for looking up records by index number
MAKE_GetRecordByIndex(SuBRecord)
MAKE_GetRecordByIndex(SuBBusInfo)
MAKE_GetRecordByIndex(SuBTrackSequence)
MAKE_GetRecordByIndex(SuBMsgContentDef)
MAKE_GetRecordByIndex(SuBMeasurand)
MAKE_GetRecordByIndex(SuBMeasurandLocation)

 /* -----------------------------------------------------------------------
 * B Records
 * ----------------------------------------------------------------------- 
 */

// B record specific decode macros
// -------------------------------

// Decode a B record
#define DECODE_B(pattern, field)                                                \
    DECODE(pattern, psuBRec->field)

#define DECODE_B_BUS(pattern, field)                                            \
    DECODE_1(pattern, field, psuBRec->psuFirstBBusInfo, SuBBusInfo)

#define DECODE_B_MSG_CONTENT(pattern, field)                                    \
    DECODE_2(pattern, field, psuBRec->psuFirstBBusInfo, SuBBusInfo, psuFirstMsgContentDef, SuBMsgContentDef)

#define DECODE_B_MESS_DESCR(pattern, field)                                     \
    DECODE_3(pattern, field, psuBRec->psuFirstBBusInfo, SuBBusInfo,             \
                             psuFirstMsgContentDef,     SuBMsgContentDef,       \
                             psuFirstMeasurand,         SuBMeasurand)

#define DECODE_B_MESS_LOCATION(pattern, field)                                  \
    DECODE_4(pattern, field, psuBRec->psuFirstBBusInfo, SuBBusInfo,             \
                             psuFirstMsgContentDef,     SuBMsgContentDef,       \
                             psuFirstMeasurand,         SuBMeasurand,           \
                             psuFirstLocation,          SuBMeasurandLocation)


/* ----------------------------------------------------------------------- */

int bDecodeBLine(char * szCodeName, char * szDataItem, SuBRecord ** ppsuFirstBRecord)
    {
    SuBRecord     * psuBRec;

    char            szCodeName2[2000];
    char            szCodeField[2000];
    char          * pcChar;
    int             iBIdx;
    int             iTokens;
    int             iIndex1, iIndex2, iIndex3, iIndex4;

    // Parse to get the B record index number, the B record, and the rest of the line
    iTokens = sscanf(szCodeName, "%*1c-%i\\%s", &iBIdx, szCodeName2);
    if (iTokens == 2)
        {
        psuBRec = psuGetRecordByIndex_SuBRecord(ppsuFirstBRecord, iBIdx, bTRUE);
        assert(psuBRec != NULL);
        }
    else
        return 1;

    // Break the rest of the line apart for further analysis
    for (pcChar=szCodeName2; *pcChar!='\0'; pcChar++)
        if (*pcChar == '-') *pcChar = ' ';
    iTokens = sscanf(szCodeName2, "%s %d %d %d %d", szCodeField, &iIndex1, &iIndex2, &iIndex3, &iIndex4);

    if (bFALSE) {}                              // Keep macro logic happy

    DECODE_B(DLN, szDataLinkName)               // DLN - Data link name
    DECODE_B(TA, szTestItem)                    // TA - Test item
    DECODE_B(BP, szBusParity)                   // BP - Bus Parity

    // Buses
    DECODE_B(NBS\\N, szNumBuses)                // NBS\N - Number of buses
    DECODE_B_BUS(BID, szBusNum)                 // BID-i - Bus number
    DECODE_B_BUS(BNA, szBusName)                // BNA-i - Bus name
    DECODE_B_BUS(BT, szBusType)                 // BT-i - Bus type

    // User defined words
    DECODE_B_BUS(UMN1, suUserDefinedMeasurement1.szName)                // UMN1-i - Measurement name
    DECODE_B_BUS(U1P,  suUserDefinedMeasurement1.szParity)              // U1P-i - Parity
    DECODE_B_BUS(U1PT, suUserDefinedMeasurement1.szParityTransferOrder) // U1PT-i - Parity transfer order
    DECODE_B_BUS(U1M,  suUserDefinedMeasurement1.szBitMask)             // U1M-i - Bit mask
    DECODE_B_BUS(U1T,  suUserDefinedMeasurement1.szTransferOrder)       // U1T-i - Transfer order

    DECODE_B_BUS(UMN2, suUserDefinedMeasurement1.szName)                // UMN2-i - Measurement name
    DECODE_B_BUS(U2P,  suUserDefinedMeasurement1.szParity)              // U2P-i - Parity
    DECODE_B_BUS(U2PT, suUserDefinedMeasurement1.szParityTransferOrder) // U2PT-i - Parity transfer order
    DECODE_B_BUS(U2M,  suUserDefinedMeasurement1.szBitMask)             // U2M-i - Bit mask
    DECODE_B_BUS(U2T,  suUserDefinedMeasurement1.szTransferOrder)       // U2T-i - Transfer order

    DECODE_B_BUS(UMN3, suUserDefinedMeasurement1.szName)                // UMN3-i - Measurement name
    DECODE_B_BUS(U3P,  suUserDefinedMeasurement1.szParity)              // U3P-i - Parity
    DECODE_B_BUS(U3PT, suUserDefinedMeasurement1.szParityTransferOrder) // U3PT-i - Parity transfer order
    DECODE_B_BUS(U3M,  suUserDefinedMeasurement1.szBitMask)             // U3M-i - Bit mask
    DECODE_B_BUS(U3T,  suUserDefinedMeasurement1.szTransferOrder)       // U3T-i - Transfer order

    // Recording description
    DECODE_B_BUS(TK\\N, szNumOfTracks)          // TK\N - Number of tracks
    DECODE_2(TS, szTrackSequenceNum, psuBRec->psuFirstBBusInfo, SuBBusInfo, psuFirstTrackSequence, SuBTrackSequence) // TS - Track sequence number

    // Message content definition
    DECODE_B_BUS(NMS\\N, szNumOfMessages)                               // NMS\N-i - Number of messages
    DECODE_B_MSG_CONTENT(MID, szMsgNum)                                 // MID-i-n - Message number
    DECODE_B_MSG_CONTENT(MNA, szMsgName)                                // MNA-i-n - Message name
    DECODE_B_MSG_CONTENT(CWE, szCmdWordEntry)                           // CWE-i-n - Command word entry
    DECODE_B_MSG_CONTENT(CMD, szCmdWord)                                // CMD-i-n - Command word
    DECODE_B_MSG_CONTENT(TRN, szRTName)                                 // TRN-i-n - Remote Terminal name
    DECODE_B_MSG_CONTENT(TRA, szRTAddress)                              // TRA-i-n - Remote Terminal address
    DECODE_B_MSG_CONTENT(STN, szSubterminalName)                        // STN-i-n - Subterminal name
    DECODE_B_MSG_CONTENT(STA, szSubterminalAddr)                        // STA-i-n - Subterminal address
    DECODE_B_MSG_CONTENT(TRM, szTRMode)                                 // TRM-i-n - T/R mode
    DECODE_B_MSG_CONTENT(DWC, szWordCntModeCode)                        // DWC-i-n - Data Words / Mode Code
    DECODE_B_MSG_CONTENT(SPR, szSpecialProcessing)                      // SPR-i-n - Special processing

    // ARINC 429
    DECODE_B_MSG_CONTENT(LBL, suARINC429.szLabel)                       // LBL-i-n - 
    DECODE_B_MSG_CONTENT(SDI, suARINC429.szSdiCode)                     // SDI-i-n - 

    // RT/RT Receive Command List
    DECODE_B_MSG_CONTENT(RCWE, suRTRTRcvCmdList.szRcvCmdWordEntry)      // RCWE-i-n - 
    DECODE_B_MSG_CONTENT(RCMD, suRTRTRcvCmdList.szRcvCmdWord)           // RCMD-i-n - 
    DECODE_B_MSG_CONTENT(RTRN, suRTRTRcvCmdList.szRTName)               // RTRN-i-n - 
    DECODE_B_MSG_CONTENT(RTRA, suRTRTRcvCmdList.szRTAddress)            // RTRA-i-n - 
    DECODE_B_MSG_CONTENT(RSTN, suRTRTRcvCmdList.szSubterminalName)      // RSTN-i-n - 
    DECODE_B_MSG_CONTENT(RSTA, suRTRTRcvCmdList.szSubterminalAddr)      // RSTA-i-n - 
    DECODE_B_MSG_CONTENT(RDWC, suRTRTRcvCmdList.szWordCnt)              // RDWC-i-n - 

    // Mode Code
    DECODE_B_MSG_CONTENT(MCD, suModeCode.szModeCodeDescrip)             // MCD-i-n - 
    DECODE_B_MSG_CONTENT(MCW, suModeCode.szDataWordDescrip)             // MCW-i-n - 

    // Measurement Description Set
    DECODE_B_MSG_CONTENT(MN\\N, szNumOfMeasurands)                      // MN\N-i-n - 
    DECODE_B_MESS_DESCR(MN, szName)                                     // MN-i-n-p - 
    DECODE_B_MESS_DESCR(MT, szType)                                     // MT-i-n-p - 
    DECODE_B_MESS_DESCR(MN1, szParity)                                  // MN1-i-n-p - 
    DECODE_B_MESS_DESCR(MN2, szParityTransferOrder)                     // MN2-i-n-p - 

    // Measurement location
    DECODE_B_MESS_DESCR(NML\\N, szNumberOfLocations)                    // NML\N-i-n-p - 
    DECODE_B_MESS_LOCATION(MWN, szWordNumber)                           // MWN-i-n-p-e - 
    DECODE_B_MESS_LOCATION(MBM, szBitMask)                              // MBM-i-n-p-e - 
    DECODE_B_MESS_LOCATION(MTO, szTransferOrder)                        // MTO-i-n-p-e - 
    DECODE_B_MESS_LOCATION(MFP, szFragmentPosition)                     // MFP-i-n-p-e - 

    return -1;
    } // end bDecodeBLine



#ifdef __cplusplus
} // end namespace i106
#endif
