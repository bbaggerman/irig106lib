/****************************************************************************

 i106_decode_tmats_d.h - Decode TMATS D fields

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

#ifndef _I106_DECODE_TMATS_D_H
#define _I106_DECODE_TMATS_D_H

#include "i106_decode_tmats_common.h"
#include "i106_decode_tmats_c.h"

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

// D Records
// =========

// Location Fragment
typedef PUBLIC struct SuDFragment_S
    {
    int                         iIndex;
    char                      * szWordPosition;     // D-x\WP-y-n-m-e
    char                      * szWordInterval;     // D-x\WI-y-n-m-e
    char                      * szFramePosition;    // D-x\FP-y-n-m-e
    char                      * szFrameInterval;    // D-x\FI-y-n-m-e
    char                      * szBitMask;          // D-x\WFM-y-n-m-e
    char                      * szTransferOrder;    // D-x\WFT-y-n-m-e
    char                      * szPosition;         // D-x\WFP-y-n-m-e

    struct SuDFragment_S      * psuNext;
    } SuDFragment;

// Measureand Sample Location
typedef PUBLIC struct SuDSampleLocation_S
    {
    int                         iIndex;
    char                      * szNumFragments;     // D-x\MNF\N-y-n-m
    SuDFragment               * psuFirstFragment;

    struct SuDSampleLocation_S  * psuNext;
    } SuDFragmentLocation;


// Simultaneous Sampling
typedef PUBLIC struct SuDSampleOn_S
    {
    int                         iIndex;
    char                      * szSampleOnWord;         // D-x\SS1-y-n-s
    char                      * szSampleOnFrame;        // D-x\SS2-y-n-s
    struct SuDSampleOn_S      * psuNext;
    } SuDSampleOn;

// Tagged Data
typedef PUBLIC struct SuDTaggedData_S
    {
    int                         iIndex;
    char                      * szTagNumber;            // D-x\TD2-y-n-m
    char                      * szBitMask;              // D-x\TD3-y-n-m
    char                      * szFragTransferOrder;    // D-x\TD4-y-n-m
    char                      * szFragmentPosition;     // D-x\TD5-y-n-m
    struct SuDTaggedData_S    * psuNext;
    } SuDTaggedData;

// Relative
typedef PUBLIC struct SuDRelativeMeasurement_S
    {
    int                         iIndex;
    char                      * szParentMeasurement;    // D-x\REL1-y-n-m
    char                      * szBitMask;              // D-x\REL2-y-n-m
    char                      * szFragTransferOrder;    // D-x\REL3-y-n-m
    char                      * szFragmentPosition;     // D-x\REL4-y-n-m
    struct SuDRelativeMeasurement_S  * psuNext;
    } SuDRelativeMeasurement;

// Measurand Description Set
typedef PUBLIC struct SuDMeasurand_S
    {
    int                         iIndex;
    char                      * szName;                 // D-x\MN-y-n
    SuCRecord                 * psuCRec;                // Linked conversion C record
    char                      * szParity;               // D-x\MN1-y-n
    char                      * szParityTransferOrder;  // D-x\MN2-y-n
    char                      * szMeasTransferOrder;    // D-x\MN3-y-n
    char                      * szMeasLocationType;     // D-x\LT-y-n
    char                      * szSFIDCounterName;      // D-x\IDCN-y-n
    char                      * szNumOfMeasLocations;   // D-x\MML\N-y-n
    SuDFragmentLocation       * psuFirstFragmentLocation;

    char                      * szSamplingMode;         // D-x\SS-y-n
    char                      * szSampleOn;             // D-x\SON-y-n
    char                      * szSampleOnMeasName;     // D-x\SMN-y-n
    char                      * szNumOfWordFrameSamples; // D-x\SS\N-y-n
    SuDSampleOn               * psuFirstSampleOn;

    char                      * szNumOfTagDefs;         // D-x\TD\N-y-n
    SuDTaggedData             * psuFirstTaggedData;

    char                      * szNumOfParentMeas;      // D-x\REL\N-y-n
    SuDRelativeMeasurement    * psuFirstRelMeasurement;

    struct SuDMeasurand_S     * psuNext;
    } SuDMeasurand;


typedef PUBLIC struct SuDMeasurementList_S
    {
    int                         iIndex;
    char                      * szMeasurementListName;  // D-x\MLN-y
    char                      * szNumOfMeasurands;      // D-x\MN\N-y
    SuDMeasurand              * psuFirstMeasurand;

    struct SuDMeasurementList_S * psuNext;
    } SuDMeasurementList;

// D record
// --------

typedef PUBLIC struct SuDRecord_S
    {
    int                         iIndex;                 // D-x
    char                      * szDataLinkName;         // D-x\DLN
    char                      * szNumMeasurementLists;  // D-x\ML\N

    SuDMeasurementList        * psuFirstMeasurementList;

    SuComment                 * psuFirstComment;        // D-x\COM

    struct SuDRecord_S        * psuNext;                // Next record in linked list
    } SuDRecord;


/*
 * Function Declaration
 * --------------------
 */

int bDecodeDLine(char * szCodeName, char * szDataItem, SuDRecord ** ppsuFirstDRec);

#ifdef __cplusplus
}
}
#endif


#endif
