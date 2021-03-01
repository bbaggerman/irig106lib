/****************************************************************************

 i106_decode_tmats_c.h - Decode TMATS C fields

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

#ifndef _I106_DECODE_TMATS_C_H
#define _I106_DECODE_TMATS_C_H

#include "i106_decode_tmats_common.h"

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


// C Records
// =========

// C record data source structures
// -------------------------------


typedef PUBLIC struct SuCBitWeight_S
    {
    int                         iIndex;
    char                      * szBitNumber;            // C-d\BWTB-n
    char                      * szBitWeightValue;       // C-d\BWTV-n
    struct SuCBitWeight_S     * psuNext;
    } SuCBitWeight;

typedef PUBLIC struct SuCInFlightCalPt_S
    {
    int                         iIndex;
    char                      * szStimulus;             // C-d\MC1-n
    char                      * szTelemetryValue;       // C-d\MC2-n
    char                      * szDataValue;            // C-d\MC3-n
    struct SuCInFlightCalPt_S * psuNext;
    } SuCInFlightCalPt;

typedef PUBLIC struct SuCAmbientConditions_S
    {
    int                         iIndex;
    char                      * szStimulus;             // C-d\MA1-n
    char                      * szTelemetryValue;       // C-d\MA2-n
    char                      * szDataValue;            // C-d\MA3-n
    struct SuCAmbientConditions_S * psuNext;
    } SuCAmbientConditions;

typedef PUBLIC struct SuCFilter_S
    {
    int                         iIndex;
    char                      * szType;                 // C-d\FTY-n
    char                      * szNumPolesSamples;      // C-d\FNPS-n
    struct SuCFilter_S        * psuNext;
    } SuCFilter;

typedef PUBLIC struct SuCPairSet_S
    {
    int                         iIndex;
    char                      * szTelemetryValue;       // C-d\PS3-n
    char                      * szEngUnitsValue;        // C-d\PS4-n
    struct SuCPairSet_S       * psuNext;
    } SuCPairSet;

typedef PUBLIC struct SuCCoefficients_S
    {
    int                         iIndex;
    char                      * szNthCoefficient;       // C-d\CO-n / C-d\NPC-n
    struct SuCCoefficients_S  * psuNext;
    } SuCCoefficients;

typedef PUBLIC struct SuCDMeaureands_S
    {
    int                         iIndex;
    char                      * szMeasureandN;          // C-d\DP-n
    struct SuCDMeaureands_S   * psuNext;
    } SuCDMeaureands;

typedef PUBLIC struct SuCDConstants_S
    {
    int                         iIndex;
    char                      * szConstantN;            // C-d\DPC-n
    struct SuCDConstants_S    * psuNext;
    } SuCDConstants;

typedef PUBLIC struct SuCDiscConvData_S
    {
    int                         iIndex;
    char                      * szConversionData;       // C-d\DICC-n
    struct SuCDiscConvData_S  * psuNext;
    } SuCDiscConvData;

typedef PUBLIC struct SuCDiscParamEvent_S
    {
    int                         iIndex;
    char                      * szParamEventDef;        // C-d\DICP-n
    struct SuCDiscParamEvent_S* psuNext;
    } SuCDiscParamEvent;


// C record
// --------

typedef PUBLIC struct SuCRecord_S
    {
    int                         iIndex;                 // C-d
    char                      * szMeasurementName;      // C-d\DCN

    // Transducer Information
    struct 
        {
        char                  * szType;                 // C-d\TRD1
        char                  * szModelNum;             // C-d\TRD2
        char                  * szSerialNum;            // C-d\TRD3
        char                  * szSecurityClass;        // C-d\TRD4
        char                  * szOrigDate;             // C-d\TRD5
        char                  * szRevNum;               // C-d\TRD6
        char                  * szOrientation;          // C-d\TRD7
        struct
            {
            char              * szName;                 // C-d\POC1
            char              * szAgency;               // C-d\POC2
            char              * szAddress;              // C-d\POC3
            char              * szTelephone;            // C-d\POC4
            } suPOC;
        } suTransducer;

    // Measurand
    struct
        {
        char                  * szDescription;          // C-d\MN1
        char                  * szAlias;                // C-d\MNA
        char                  * szExcitationVolts;      // C-d\MN2
        char                  * szEngUnits;             // C-d\MN3
        char                  * szLinkType;             // C-d\MN4
        } suMeasurand;

    // Telemetry Value Definition
    struct
        {
        char                  * szBinaryFormat;         // C-d\BFM
        char                  * szFloatPtFormat;        // C-d\FPF
        char                  * szNumBitWeights;        // C-d\BWT\N
        SuCBitWeight          * psuFirstBitWeight;
        } suTelemValueDef;

    // In-Flight Calibration
    char                      * szNumInFlightCalPts;    // C-d\MC\N
    SuCInFlightCalPt          * psuFirstInFltCalPt;

    // Ambient Value
    char                      * szNumAmbientConditions; // C-d\MA\N
    SuCAmbientConditions      * psuFirstAmbientCondition;

    // Measurement Filtering
    struct
        {
        char                  * szEnabled;              // C-d\FEN
        char                  * szDelay;                // C-d\FDL
        char                  * szNumFilters;           // C-d\F\N
        SuCFilter             * psuFirstFilter;
        } suFiltering;

    // Other Information
    char                      * szHighMeasurementVal;   // C-d\MOT1
    char                      * szLowMeasurementVal;    // C-d\MOT2
    char                      * szHighAlertLimitVal;    // C-d\MOT3
    char                      * szLowAlertLimitVal;     // C-d\MOT4
    char                      * szHighWarnLimitVal;     // C-d\MOT5
    char                      * szLowWarnLimitVal;      // C-d\MOT6
    char                      * szInitialValue;         // C-d\MOT7
    char                      * szSampleRate;           // C-d\SR

    // Data Conversion
    struct
        {
        char                  * szReleaseDateTime;      // C-d\CRT
        char                  * szType;                 // C-d\DCT

        // Pair Sets
        struct
            {
            char              * szNumOfSets;            // C-d\PS\N
            char              * szApplication;          // C-d\PS1
            char              * szOrdeOfFit;            // C-d\PS2
            SuCPairSet        * psuFirstPairSet;
            } suPairSets;

        // Coefficients
        struct
            {
            char              * szOrderOfCurveFit;      // C-d\CO\N
            char              * szDerivedFromPairSet;   // C-d\CO1
            char              * szCoefficient0;         // C-d\CO
            SuCCoefficients   * psuFirstCoefficient;
            } suCoefficients;

        // Coefficients (Negative Powers of X)
        struct
            {
            char              * szOrderOfCurveFit;      // C-d\NPC\N
            char              * szDerivedFromPairSet;   // C-d\NPC1
            char              * szCoefficient0;         // C-d\NPC
            SuCCoefficients   * psuFirstCoefficient;
            } suNegXCoefficients;

        // Other
        char                  * szOtherConversionDef;   // C-d\OTH

        // Derived Parameter
        struct
            {
            char              * szAlgorithmType;        // C-d\DPAT
            char              * szAlgorithm;            // C-d\DPA
            char              * szTriggerMeasurand;     // C-d\DPTM
            char              * szNumOfOccurances;      // C-d\DPNO
            char              * szNumOfInputMeasurands; // C-d\DP\N
            SuCDMeaureands    * psuFirstMeasureand;
            char              * szNumOfInputConstants;  // C-d\DP\N
            SuCDConstants     * psuFirstConstant;
            } suDerived;

        // Discrete
        struct
            {
            char              * szNumOfEvents;          // C-d\DIC\N
            SuCDiscParamEvent * psuFirstEvent;
            char              * szNumOfIndicators;      // C-d\DICI\N
            SuCDiscConvData   * psuFirstConversion;
            } suDiscrete;

        char                  * szPCMTimeWordFormat;    // C-d\PTM
        char                  * sz1553TimeWordFormat;   // C-d\BTM
        char                  * szDigitalVoiceMethod;   // C-d\VOI\E
        char                  * szDigitalVoiceDescr;    // C-d\VOI\D
        char                  * szDigitalVideoMethod;   // C-d\VID\E
        char                  * szDigitalVideoDescr;    // C-d\VID\D

        } suEUConversion;

    SuComment                 * psuFirstComment;        // C-d\COM

    struct SuCRecord_S        * psuNext;                // Next record in linked list
    } SuCRecord;


/*
 * Function Declaration
 * --------------------
 */

int bDecodeCLine(char * szCodeName, char * szDataItem, SuCRecord ** ppsuFirstCRec);

#ifdef __cplusplus
}
}
#endif


#endif
