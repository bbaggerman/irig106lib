/****************************************************************************

 i106_decode_tmats_d.c - Decode TMATS D fields

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
#include "i106_decode_tmats_d.h"

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
MAKE_GetRecordByIndex(SuDRecord)
MAKE_GetRecordByIndex(SuDMeasurementList)
MAKE_GetRecordByIndex(SuDMeasurand)
MAKE_GetRecordByIndex(SuDRelativeMeasurement)
MAKE_GetRecordByIndex(SuDTaggedData)
MAKE_GetRecordByIndex(SuDSampleOn)
MAKE_GetRecordByIndex(SuDFragmentLocation)
MAKE_GetRecordByIndex(SuDFragment)

 /* -----------------------------------------------------------------------
 * B Records
 * ----------------------------------------------------------------------- 
 */

// D record specific decode macros
// -------------------------------

#define DECODE_D(pattern, field)                                                \
    DECODE(pattern, psuDRec->field)

#define DECODE_D_MEAS_LIST(pattern, field)                                      \
    DECODE_1(pattern, field, psuDRec->psuFirstMeasurementList, SuDMeasurementList)

#define DECODE_D_MEASURAND(pattern, field)                                      \
    DECODE_2(pattern, field, psuDRec->psuFirstMeasurementList, SuDMeasurementList, psuFirstMeasurand, SuDMeasurand)

#define DECODE_D_FRAGMENT_LOC(pattern, field)                                        \
    DECODE_3(pattern, field, psuDRec->psuFirstMeasurementList,  SuDMeasurementList,  \
                                      psuFirstMeasurand,        SuDMeasurand,        \
                                      psuFirstFragmentLocation, SuDFragmentLocation)

#define DECODE_D_FRAGMENT(pattern, field)                                            \
    DECODE_4(pattern, field, psuDRec->psuFirstMeasurementList,  SuDMeasurementList,  \
                                      psuFirstMeasurand,        SuDMeasurand,        \
                                      psuFirstFragmentLocation, SuDFragmentLocation, \
                                      psuFirstFragment,         SuDFragment)

#define DECODE_D_SAMPLE_ON(pattern, field)                                           \
    DECODE_3(pattern, field, psuDRec->psuFirstMeasurementList,  SuDMeasurementList,  \
                                      psuFirstMeasurand,        SuDMeasurand,        \
                                      psuFirstSampleOn,         SuDSampleOn)

#define DECODE_D_TAG_DATA(pattern, field)                                            \
    DECODE_3(pattern, field, psuDRec->psuFirstMeasurementList,  SuDMeasurementList,  \
                                      psuFirstMeasurand,        SuDMeasurand,        \
                                      psuFirstTaggedData,       SuDTaggedData)

#define DECODE_D_REL_MEAS(pattern, field)                                            \
    DECODE_3(pattern, field, psuDRec->psuFirstMeasurementList,  SuDMeasurementList,  \
                                      psuFirstMeasurand,        SuDMeasurand,        \
                                      psuFirstRelMeasurement,   SuDRelativeMeasurement)



#if 0
#define DECODE_D_MESS_LOCATION(pattern, field)                                  \
    DECODE_4(pattern, field, psuBRec->psuFirstBBusInfo, SuBBusInfo,             \
                             psuFirstMsgContentDef,     SuBMsgContentDef,       \
                             psuFirstMeasurand,         SuBMeasurand,           \
                             psuFirstLocation,          SuBMeasurandLocation)
#endif



/* ----------------------------------------------------------------------- */

int bDecodeDLine(char * szCodeName, char * szDataItem, SuDRecord ** ppsuFirstDRecord)
    {
    SuDRecord     * psuDRec;

    char            szCodeName2[2000];
    char            szCodeField[2000];
    char          * pcChar;
    int             iDIdx;
    int             iTokens;
    int             iIndex1, iIndex2, iIndex3, iIndex4;

    // Parse to get the D record index number, the B record, and the rest of the line
    iTokens = sscanf(szCodeName, "%*1c-%i\\%s", &iDIdx, szCodeName2);
    if (iTokens == 2)
        {
        psuDRec = psuGetRecordByIndex_SuDRecord(ppsuFirstDRecord, iDIdx, bTRUE);
        assert(psuDRec != NULL);
        }
    else
        return 1;

    // Break the rest of the line apart for further analysis
    for (pcChar=szCodeName2; *pcChar!='\0'; pcChar++)
        if (*pcChar == '-') *pcChar = ' ';
    iTokens = sscanf(szCodeName2, "%s %d %d %d %d", szCodeField, &iIndex1, &iIndex2, &iIndex3, &iIndex4);

    if (bFALSE) {}                                  // Keep macro logic happy

    DECODE_D(DLN, szDataLinkName)                   // DLN - Data link name

    // Measurement lists
    DECODE_D(ML\\N, szNumMeasurementLists)          // ML\N - Number of measurement lists
    DECODE_D_MEAS_LIST(MLN, szMeasurementListName)  // MLN-y - Measurement list name
    DECODE_D_MEAS_LIST(MN\\N, szNumOfMeasurands)    // MN\N-y - Number of measurands

    // Measurands
    DECODE_D_MEASURAND(MN, szName)                  // MN-y-n - Measurand name
    DECODE_D_MEASURAND(MN1, szParity)               // MN1-y-n - Parity
    DECODE_D_MEASURAND(MN2, szParityTransferOrder)  // MN2-y-n - Parity transfer order
    DECODE_D_MEASURAND(MN3, szMeasTransferOrder)    // MN3-y-n - Measurement transfer order
    DECODE_D_MEASURAND(LT, szMeasLocationType)      // LT-y-n - Measurand location type
    DECODE_D_MEASURAND(IDCN, szSFIDCounterName)     // IDCN-y-n - Subframe ID counter name
    DECODE_D_MEASURAND(MML\\N, szNumOfMeasLocations) // MML\N-y-n - Number of measurand locations
    DECODE_D_FRAGMENT_LOC(MNF\\N, szNumFragments)   // D-x\MNF\N-y-n-m - Number of fragments
    DECODE_D_FRAGMENT(WP, szWordPosition)           // WP-y-n-m-e - Word position
    DECODE_D_FRAGMENT(WI, szWordInterval)           // WI-y-n-m-e - Word interval
    DECODE_D_FRAGMENT(FP, szFramePosition)          // FP-y-n-m-e - Frame position
    DECODE_D_FRAGMENT(FI, szFrameInterval)          // FI-y-n-m-e - Frame interval
    DECODE_D_FRAGMENT(WFM, szBitMask)               // WFM-y-n-m-e - Bit mask
    DECODE_D_FRAGMENT(WFT, szTransferOrder)         // WFT-y-n-m-e - Fragment transfer order
    DECODE_D_FRAGMENT(WFP, szPosition)              // WFP-y-n-m-e - Fragment position

    // 
    DECODE_D_MEASURAND(SS, szSamplingMode)          // SS-y-n - Sampling mode
    DECODE_D_MEASURAND(SON, szSampleOn)             // SON-y-n 
    DECODE_D_MEASURAND(SMN, szSampleOnMeasName)     // SMN-y-n
    DECODE_D_MEASURAND(SS, szNumOfWordFrameSamples) // SS\N-y-n
    DECODE_D_SAMPLE_ON(SS1, szSampleOnWord)         // SS1-y-n-s
    DECODE_D_SAMPLE_ON(SS2, szSampleOnFrame)        // SS2-y-n-s

    DECODE_D_MEASURAND(TD\\N, szNumOfTagDefs)       // TD\N-y-n
    DECODE_D_TAG_DATA(TD2,szTagNumber)              // TD2-y-n-m
    DECODE_D_TAG_DATA(TD3,szBitMask)                // TD3-y-n-m
    DECODE_D_TAG_DATA(TD4,szFragTransferOrder)      // TD4-y-n-m
    DECODE_D_TAG_DATA(TD5,szFragmentPosition)       // TD5-y-n-m

    DECODE_D_MEASURAND(REL\\N, szNumOfParentMeas)   // REL\N-y-n
    DECODE_D_REL_MEAS(REL1 ,szParentMeasurement)    // REL1-y-n-m
    DECODE_D_REL_MEAS(REL2 ,szBitMask)              // REL2-y-n-m
    DECODE_D_REL_MEAS(REL3 ,szFragTransferOrder)    // REL3-y-n-m
    DECODE_D_REL_MEAS(REL4 ,szFragmentPosition)     // REL4-y-n-m

    return -1;
    } // end bDecodeDLine



#ifdef __cplusplus
} // end namespace i106
#endif
