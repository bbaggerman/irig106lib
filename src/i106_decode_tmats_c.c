/****************************************************************************

 i106_decode_tmats_c.c - Decode TMATS B fields

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
#include "i106_decode_tmats_c.h"

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
MAKE_GetRecordByIndex(SuCRecord)
MAKE_GetRecordByIndex(SuCBitWeight)
MAKE_GetRecordByIndex(SuCInFlightCalPt)
MAKE_GetRecordByIndex(SuCAmbientConditions)
MAKE_GetRecordByIndex(SuCFilter)
MAKE_GetRecordByIndex(SuCPairSet)
MAKE_GetRecordByIndex(SuCCoefficients)
MAKE_GetRecordByIndex(SuCDMeaureands)
MAKE_GetRecordByIndex(SuCDConstants)
MAKE_GetRecordByIndex(SuCDiscConvData)
MAKE_GetRecordByIndex(SuCDiscParamEvent)

 /* -----------------------------------------------------------------------
 * C Records
 * ----------------------------------------------------------------------- 
 */

// C record specific decode macros
// -------------------------------

// Decode an R record
#define DECODE_C(pattern, field)                                                \
    DECODE(pattern, psuCRec->field)

#define DECODE_C_BIT_WEIGHT(pattern, field)                                     \
    DECODE_1(pattern, field, psuCRec->suTelemValueDef.psuFirstBitWeight, SuCBitWeight)

#define DECODE_C_IN_FLIGHT(pattern, field)                                      \
    DECODE_1(pattern, field, psuCRec->psuFirstInFltCalPt, SuCInFlightCalPt)

#define DECODE_C_AMBIENT(pattern, field)                                        \
    DECODE_1(pattern, field, psuCRec->psuFirstAmbientCondition, SuCAmbientConditions)

#define DECODE_C_FILTER(pattern, field)                                         \
    DECODE_1(pattern, field, psuCRec->suFiltering.psuFirstFilter, SuCFilter)

#define DECODE_C_PAIR_SET(pattern, field)                                       \
    DECODE_1(pattern, field, psuCRec->suEUConversion.suPairSets.psuFirstPairSet, SuCPairSet)

#define DECODE_C_COEFF(pattern, field)                                          \
    DECODE_1(pattern, field, psuCRec->suEUConversion.suCoefficients.psuFirstCoefficient, SuCCoefficients)

#define DECODE_C_COEFF_NEGX(pattern, field)                                     \
    DECODE_1(pattern, field, psuCRec->suEUConversion.suNegXCoefficients.psuFirstCoefficient, SuCCoefficients)

#define DECODE_C_MEASUREAND_N(pattern, field)                                   \
    DECODE_1(pattern, field, psuCRec->suEUConversion.suDerived.psuFirstMeasureand, SuCDMeaureands)

#define DECODE_C_CONSTANT_N(pattern, field)                                     \
    DECODE_1(pattern, field, psuCRec->suEUConversion.suDerived.psuFirstConstant, SuCDConstants)

#define DECODE_C_CONV_DATA(pattern, field)                                      \
    DECODE_1(pattern, field, psuCRec->suEUConversion.suDiscrete.psuFirstConversion, SuCDiscConvData)

#define DECODE_C_EVENT_DEF(pattern, field)                                      \
    DECODE_1(pattern, field, psuCRec->suEUConversion.suDiscrete.psuFirstEvent, SuCDiscParamEvent)


/* ----------------------------------------------------------------------- */

int bDecodeCLine(char * szCodeName, char * szDataItem, SuCRecord ** ppsuFirstCRecord)
    {
    SuCRecord     * psuCRec;

    char            szCodeName2[2000];
    char            szCodeField[2000];
    char          * pcChar;
    int             iCIdx;
    int             iTokens;
    int             iIndex1, iIndex2, iIndex3, iIndex4;

    // Parse to get the C record index number, the C record, and the rest of the line
    iTokens = sscanf(szCodeName, "%*1c-%i\\%s", &iCIdx, szCodeName2);
    if (iTokens == 2)
        {
        psuCRec = psuGetRecordByIndex_SuCRecord(ppsuFirstCRecord, iCIdx, bTRUE);
        assert(psuCRec != NULL);
        }
    else
        return 1;

    // Break the rest of the line apart for further analysis
    for (pcChar=szCodeName2; *pcChar!='\0'; pcChar++)
        if (*pcChar == '-') *pcChar = ' ';
    iTokens = sscanf(szCodeName2, "%s %d %d %d %d", szCodeField, &iIndex1, &iIndex2, &iIndex3, &iIndex4);

    if (bFALSE) {}                              // Keep macro logic happy

    DECODE_C(DCN, szMeasurementName)                    // DCN - Measurement name

    // Transducer Information
    DECODE_C(TRD1, suTransducer.szType)                 // C-d\TRD1
    DECODE_C(TRD2, suTransducer.szModelNum)             // C-d\TRD2
    DECODE_C(TRD3, suTransducer.szSerialNum)            // C-d\TRD3
    DECODE_C(TRD4, suTransducer.szSecurityClass)        // C-d\TRD4
    DECODE_C(TRD5, suTransducer.szOrigDate)             // C-d\TRD5
    DECODE_C(TRD6, suTransducer.szRevNum)               // C-d\TRD6
    DECODE_C(TRD7, suTransducer.szOrientation)          // C-d\TRD7
    DECODE_C(POC1, suTransducer.suPOC.szName)           // C-d\POC1
    DECODE_C(POC2, suTransducer.suPOC.szAgency)         // C-d\POC2
    DECODE_C(POC3, suTransducer.suPOC.szAddress)        // C-d\POC3
    DECODE_C(POC4, suTransducer.suPOC.szTelephone)      // C-d\POC4

    // Measureand
    DECODE_C(MN1, suMeasurand.szDescription)            // C-d\MN1
    DECODE_C(MNA, suMeasurand.szAlias)                  // C-d\MNA
    DECODE_C(MN2, suMeasurand.szExcitationVolts)        // C-d\MN2
    DECODE_C(MN3, suMeasurand.szEngUnits)               // C-d\MN3
    DECODE_C(MN4, suMeasurand.szLinkType)               // C-d\MN4

    // Telemetry Value Definition
    DECODE_C(BFM, suTelemValueDef.szBinaryFormat)       // C-d\BFM
    DECODE_C(FPF, suTelemValueDef.szFloatPtFormat)      // C-d\FPF
    DECODE_C(BWT\\N, suTelemValueDef.szNumBitWeights)   // C-d\BWT\N
    DECODE_C_BIT_WEIGHT(BWTB, szBitNumber)              // C-d\BWTB-n
    DECODE_C_BIT_WEIGHT(BWTV, szBitWeightValue)         // C-d\BWTV-n

    // In-Flight Calibration
    DECODE_C(MC\\N, szNumInFlightCalPts)                // C-d\MC\N
    DECODE_C_IN_FLIGHT(MC1, szStimulus)                 // C-d\MC1-n
    DECODE_C_IN_FLIGHT(MC2, szTelemetryValue)           // C-d\MC2-n
    DECODE_C_IN_FLIGHT(MC3, szDataValue)                // C-d\MC3-n

    // Ambient Value
    DECODE_C(MA\\N, szNumAmbientConditions)             // C-d\MA\N
    DECODE_C_AMBIENT(MA1, szStimulus)                   // C-d\MA1-n
    DECODE_C_AMBIENT(MA2, szTelemetryValue)             // C-d\MA2-n
    DECODE_C_AMBIENT(MA3, szDataValue)                  // C-d\MA3-n

    // Measurement Filtering
    DECODE_C(FEN, suFiltering.szEnabled)                // C-d\FEN
    DECODE_C(FDL, suFiltering.szDelay)                  // C-d\FDL
    DECODE_C(F\\N, suFiltering.szNumFilters)            // C-d\F\N
    DECODE_C_FILTER(FTY, szType)                        // C-d\FTY-n
    DECODE_C_FILTER(FNPS, szNumPolesSamples)            // C-d\FNPS-n

    // Other Information
    DECODE_C(MOT1, szHighMeasurementVal)                // C-d\MOT1
    DECODE_C(MOT2, szLowMeasurementVal)                 // C-d\MOT2
    DECODE_C(MOT3, szHighAlertLimitVal)                 // C-d\MOT3
    DECODE_C(MOT4, szLowAlertLimitVal)                  // C-d\MOT4
    DECODE_C(MOT5, szHighWarnLimitVal)                  // C-d\MOT5
    DECODE_C(MOT6, szLowWarnLimitVal)                   // C-d\MOT6
    DECODE_C(MOT7, szInitialValue)                      // C-d\MOT7
    DECODE_C(SR, szSampleRate)                          // C-d\SR

    // Data Conversion
    DECODE_C(CRT, suEUConversion.szReleaseDateTime)     // C-d\CRT
    DECODE_C(DCT, suEUConversion.szType)                // C-d\DCT

    // Pair Sets
    DECODE_C(PS\\N, suEUConversion.suPairSets.szNumOfSets)  // C-d\PS\N
    DECODE_C(PS1, suEUConversion.suPairSets.szApplication)  // C-d\PS1
    DECODE_C(PS2, suEUConversion.suPairSets.szOrdeOfFit)    // C-d\PS2
    DECODE_C_PAIR_SET(PS3, szTelemetryValue)                // C-d\PS3-n
    DECODE_C_PAIR_SET(PS4, szEngUnitsValue)                 // C-d\PS4-n
  
    // Coefficients
    DECODE_C(CO\\N, suEUConversion.suCoefficients.szOrderOfCurveFit)    // C-d\CO\N
    DECODE_C(CO1, suEUConversion.suCoefficients.szDerivedFromPairSet)   // C-d\CO1
    DECODE_C(CO, suEUConversion.suCoefficients.szCoefficient0)          // C-d\CO
    DECODE_C_COEFF(NPC, szNthCoefficient)                               // C-d\CO-n

    // Coefficients (Negative Powers of X)
    DECODE_C(NPC\\N, suEUConversion.suNegXCoefficients.szOrderOfCurveFit)   // C-d\NPC\N
    DECODE_C(NPC1, suEUConversion.suNegXCoefficients.szDerivedFromPairSet)  // C-d\NPC1
    DECODE_C(NPC, suEUConversion.suNegXCoefficients.szCoefficient0)         // C-d\NPC
    DECODE_C_COEFF_NEGX(NPC, szNthCoefficient)                              // C-d\NPC-n

    // Other
    DECODE_C(OTH, suEUConversion.szOtherConversionDef)   // C-d\OTH

    // Derived Parameter
    DECODE_C(DPAT, suEUConversion.suDerived.szAlgorithmType)            // C-d\DPAT
    DECODE_C(DPA, suEUConversion.suDerived.szAlgorithm)                 // C-d\DPA
    DECODE_C(DPTM, suEUConversion.suDerived.szTriggerMeasurand)         // C-d\DPTM
    DECODE_C(DPNO, suEUConversion.suDerived.szNumOfOccurances)          // C-d\DPNO

    DECODE_C(DP\\N, suEUConversion.suDerived.szNumOfInputMeasurands)    // C-d\DP\N
    DECODE_C_MEASUREAND_N(DP ,szMeasureandN)                            // C-d\DP-n
    DECODE_C(DPC\\N, suEUConversion.suDerived.szNumOfInputConstands)    // C-d\DPC\N
    DECODE_C_CONSTANT_N(DPC, szConstantN)                               // C-d\DPC-n

    // Discrete
    DECODE_C(DIC\\N, suEUConversion.suDiscrete.szNumOfEvents)           // C-d\DIC\N
    DECODE_C_CONV_DATA(DICC, szConversionData)                          // C-d\DICC-n
    DECODE_C(DICI\\N, suEUConversion.suDiscrete.szNumOfIndicators)      // C-d\DICI\N
    DECODE_C_EVENT_DEF(DICP ,szParamEventDef)                           // C-d\DICP-n

    DECODE_C(PTM, suEUConversion.szPCMTimeWordFormat)       // C-d\PTM
    DECODE_C(BTM, suEUConversion.sz1553TimeWordFormat)      // C-d\BTM
    DECODE_C(VOI\\E, suEUConversion.szDigitalVoiceMethod)   // C-d\VOI\E
    DECODE_C(VOI\\D, suEUConversion.szDigitalVoiceDescr)    // C-d\VOI\D
    DECODE_C(VID\\E, suEUConversion.szDigitalVideoMethod)   // C-d\VID\E
    DECODE_C(VID\\D, suEUConversion.szDigitalVideoDescr)    // C-d\VID\D

    return -1;
    } // end bDecodeCLine



#ifdef __cplusplus
} // end namespace i106
#endif
