/****************************************************************************

 i106_decode_tmats_p.c - Decode TMATS P fields

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <assert.h>

#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
//#include "i106_decode_tmats_common.h"
//#include "i106_decode_tmats.h"
#include "i106_decode_tmats_p.h"

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
MAKE_GetRecordByIndex(SuPRecord)
MAKE_GetRecordByIndex(SuPSubframeId)
MAKE_GetRecordByIndex(SuPSubframeDef)
MAKE_GetRecordByIndex(SuPSubframeLoc)
MAKE_GetRecordByIndex(SuPAsyncEmbedded)
MAKE_GetRecordByIndex(SuPAsyncEmbeddedLocationDef)
MAKE_GetRecordByIndex(SuPCh7Segment)

 /* -----------------------------------------------------------------------
 * P Records
 * ----------------------------------------------------------------------- 
 */

// P record specific decode macros
// -------------------------------

// Decode an R record
#define DECODE_P(pattern, field)                                                \
    DECODE(pattern, psuPRec->field)

// Subframe ID linked list items
#define DECODE_P_SF(pattern, field)                                             \
    DECODE_1(pattern, field, psuPRec->psuFirstSubframeId, SuPSubframeId)

// Async Embedded linked list items
#define DECODE_P_ASYNC(pattern, field)                                          \
    DECODE_1(pattern, field, psuPRec->psuFirstAsyncEmbedded, SuPAsyncEmbedded)

#define DECODE_P_ASYNC_LOCATION(pattern, field)                                 \
    DECODE_2(pattern, field, psuPRec->psuFirstAsyncEmbedded, SuPAsyncEmbedded, psuFirstAsyncLocation, SuPAsyncEmbeddedLocationDef)



// Chapter 7 segment definitions
#define DECODE_P_CH7(pattern, field)                                          \
    DECODE_1(pattern, field, psuPRec->psuFirstCh7Segment, SuPCh7Segment)



//TODO!!!
#define DECODE_P_SFDEF(pattern, field)                                           
#define DECODE_P_SFDEFLOC(pattern, field)                                          

/* -----------------------------------------------------------------------
 * P Records
 * ----------------------------------------------------------------------- 
 */

int bDecodePLine(char * szCodeName, char * szDataItem, SuPRecord ** ppsuFirstPRecord)
    {
    SuPRecord         * psuPRec;

    char            szCodeName2[2000];
    char            szCodeField[2000];
    char          * pcChar;
    int             iPIdx;
    int             iTokens;
    int             iIndex1, iIndex2, iIndex3, iIndex4;

    // Parse to get the R record index number, the R record, and the rest of the line
    iTokens = sscanf(szCodeName, "%*1c-%i\\%s", &iPIdx, szCodeName2);
    if (iTokens == 2)
        {
        psuPRec = psuGetRecordByIndex_SuPRecord(ppsuFirstPRecord, iPIdx, bTRUE);
        assert(psuPRec != NULL);
        }
    else
        return 1;

    // Break the rest of the line apart for further analysis
    for (pcChar=szCodeName2; *pcChar!='\0'; pcChar++)
        if (*pcChar == '-') *pcChar = ' ';
    iTokens = sscanf(szCodeName2, "%s %d %d %d %d", szCodeField, &iIndex1, &iIndex2, &iIndex3, &iIndex4);

    if (bFALSE) {}                              // Keep macro logic happy
    
    DECODE_P(DLN, szDataLinkName)               // DLN - Data link name
    DECODE_P(D1, szPcmCode)                     // D1 - PCM Code
    DECODE_P(D2, szBitsPerSec)                  // D2 - Bit Rate
    DECODE_P(D4, szPolarity)                    // D4 - Polarity
    DECODE_P(D5, szAutoPolarityCorrection)      // D5 = Auto-polarity correction
    DECODE_P(D6, szDataDirection)               // D6 - Data direction
    DECODE_P(D7, szDataRandomized)              // D7 - Data randomized
    DECODE_P(D8, szRandomizerLength)            // D8 - Randomizer length

    DECODE_P(TF, szTypeFormat)                  // TF - Type Format
    DECODE_P(F1, szCommonWordLen)               // F1 - Common World Length
    DECODE_P(F2, szWordTransferOrder)           // F2 - MSB / LSB first
    DECODE_P(F3, szParityType)                  // F3 - Even, odd, none
    DECODE_P(F4, szParityTransferOrder)         // F4 - Leading / Trailing

    DECODE_P(CRC, szCRC)
    DECODE_P(CRCCB, szCRCCheckWordStartBit)
    DECODE_P(CRCDB, szCRCDataStartBit)
    DECODE_P(CRCDN, szCRCDataNumOfBits)

    // MF
    DECODE_P(MF\\N, szNumMinorFrames)           // ISF\N - Number of minor frames
    DECODE_P(MF1, szWordsInMinorFrame)          // MF1 - Number of word in minor frame
    DECODE_P(MF2, szBitsInMinorFrame)           // MF2 - Number of bits in minor frame
    DECODE_P(MF3, szMinorFrameSyncType)         // MF3 - Minor Frame Sync Type
    DECODE_P(MF4, szMinorFrameSyncPatLen)       // MF4 - Minor frame sync pattern length
    DECODE_P(MF5, szMinorFrameSyncPat)          // MF5 - Minor frame sync pattern
    DECODE_P(SYNC1, szInSyncCrit)               // SYNC1 - In-sync criteria
    DECODE_P(SYNC2, szInSyncErrors)             // SYNC2 - In-sync errors allowed
    DECODE_P(SYNC3, szOutSyncCrit)              // SYNC3 - Out-of-sync criteria
    DECODE_P(SYNC4, szOutSyncErrors)            // SYNC4 - Out-of-sync errors allowed
    DECODE_P(SYNC5, szFillBits)                 // SYNC5 - Fill bits

    // ISF - Subframe sync
    DECODE_P(ISF\\N, szNumSubframeCounters)     // ISF\N - Number of subframe ID counters
    DECODE_P_SF(ISF1, szCounterName)            // ISF1-n - Subframe ID counter name
    DECODE_P_SF(ISF2, szCounterType)            // ISF2-n - Subframe sync type
    DECODE_P_SF(IDC1, szWordPosition)           // IDC1-n - Minor frame word position
    DECODE_P_SF(IDC2, szWordLength)             // IDC2-n - Minor frame word length
    DECODE_P_SF(IDC3, szBitLocation)            // IDC3-n - Bit location
    DECODE_P_SF(IDC4, szCounterLen)             // IDC4-n - Counter bit length
    DECODE_P_SF(IDC5, szEndian)                 // IDC5-n - Counter endian
    DECODE_P_SF(IDC6, szInitValue)              // IDC6-n - Initial value
    DECODE_P_SF(IDC7, szMFForInitValue)         // IDC7-n - Initial count minor frame
    DECODE_P_SF(IDC8, szEndValue)               // IDC8-n - End value
    DECODE_P_SF(IDC9, szMFForEndValue)          // IDC9-n - End value minor frame
    DECODE_P_SF(IDC10, szCountDirection)        // IDC10-n - Count direction

    // AEF - Asynchronous embedded format
    DECODE_P(AEF\\N, szNumAsyncEmbedded)        // AEF\N - Number of subframe ID counters
    DECODE_P_ASYNC(AEF\\DLN, szDataLinkName)    // AEF\DLN-n - Data link name
    DECODE_P_ASYNC(AEF-1, szSupercomutated)     // SUPERCOM (P-d\AEF1-n)
    DECODE_P_ASYNC(AEF-2, szLocationDefinition) // LOCATION DEFINITION (P-d\AEF2-n)
    DECODE_P_ASYNC_LOCATION(AEF-3, szWordLocation)  //(P-d\AEF3-n-w)
    DECODE_P_ASYNC(AEF-4, szInterval)           // INTERVAL (P-d\AEF4-n)
    DECODE_P_ASYNC_LOCATION(AEF-5, szWordLength)    //WORD LENGTH (P-d\AEF5-n-w)
    DECODE_P_ASYNC_LOCATION(AEF-6, szWordMask)      //MASK (P-d\AEF6-n-w)

/*
#if 0
    else if (strcasecmp(szCodeField, "SF") == 0)
        {
        szCodeField = strtok(NULL, "\\");
        if (bFALSE) {}                          // Keep macro logic happy
        DECODE_P_SF(N, szNumSubframeDefs)       // SF\N-n - Number of subframes
        } // end if SF
#else
    DECODE_P(SF\\N, szNumSubframeCounters)     // ISF\N - Number of subframe ID counters
#endif

    DECODE_P_SFDEF(SF1, szSubframeName)         // SF1-n-x - Subframe name
    DECODE_P_SFDEF(SF2, szSuperComPosition)     // SF2-n-x - Number of supercom word positions (or NO)
    DECODE_P_SFDEF(SF3, szSuperComDefined)      // SF3-n-x - How supercom word position is defined
    DECODE_P_SFDEFLOC(SF4, szSubframeLocation)  // SF4-n-x-y - Subframe location
    DECODE_P_SFDEF(SF5, szLocationInterval)     // SF5-n-x - Word location interval
    DECODE_P_SFDEF(SF6, szSubframeDepth)        // SF6-n-x - Subframe depth (whatever that is)
*/


    // Chapter 7 Format
    DECODE_P(C7\\N, szCh7NumSegments)           // C7\N
    DECODE_P_CH7(C7FW, szCh7FirstWord)          // P-d\C7FW-n
    DECODE_P_CH7(C7NW, szNumberOfPcmWords)      // (P-d\C7NW-n)

    return 0;
    }


#if 0
/* ----------------------------------------------------------------------- */

SuPRecord * psuGetPRecord(SuPRecord ** ppsuFirstPRecord, int iPIndex, int bMakeNew)
    {
    SuPRecord   ** ppsuCurrPRec = ppsuFirstPRecord;

    // Loop looking for matching index number or end of list
    while (bTRUE)
        {
        // Check for end of list
        if (*ppsuCurrPRec == NULL)
            break;

        // Check for matching index number
        if ((*ppsuCurrPRec)->iRecordNum == iPIndex)
            break;

        // Move on to the next record in the list
        ppsuCurrPRec = &((*ppsuCurrPRec)->psuNextPRecord);
        }

    // If no record found then put a new one on the end of the list
    if ((*ppsuCurrPRec == NULL) && (bMakeNew == bTRUE))
        {
        // Allocate memory for the new record
        *ppsuCurrPRec = (SuPRecord *)TmatsMalloc(sizeof(SuPRecord));
        memset(*ppsuCurrPRec, 0, sizeof(SuPRecord));
        (*ppsuCurrPRec)->iRecordNum = iPIndex;
        }

    return *ppsuCurrPRec;
    }



/* ----------------------------------------------------------------------- */

// Return the P record Asynchronous Embedded Format record with the given index or
// make a new one if necessary.

SuPAsyncEmbedded * psuGetPAsyncEmbedded(SuPRecord * psuPRec, int iAEFIndex, int bMakeNew)
    {

    SuPAsyncEmbedded   ** ppsuAEF = &(psuPRec->psuFirstAsyncEmbedded);

    // Walk the linked list of embedded streams, looking for a match or
    // the end of the list
    while (bTRUE)
        {
        // If record pointer in linked list is null then exit
        if (*ppsuAEF == NULL)
            {
            break;
            }

        // If the data source number matched then record found, exit
        if ((*ppsuAEF)->iEmbeddedStreamNum == iAEFIndex)
            {
            break;
            }

        // Not found but next record exists so make it our current pointer
        ppsuAEF = &((*ppsuAEF)->psuNextEmbedded);
        } // end

    // If no record found then put a new one on the end of the list
    if ((*ppsuAEF == NULL) && (bMakeNew == bTRUE))
        {
        // Allocate memory for the new record
        *ppsuAEF = (SuPAsyncEmbedded *)TmatsMalloc(sizeof(SuPAsyncEmbedded));
        memset(*ppsuAEF, 0, sizeof(SuPAsyncEmbedded));
        (*ppsuAEF)->iEmbeddedStreamNum = iAEFIndex;

        } // end if new record

    return *ppsuAEF;
    }


/* ----------------------------------------------------------------------- */

// Return the P record Subframe ID record with the given index or
// make a new one if necessary.

SuPSubframeId * psuGetPSubframeID(SuPRecord * psuPRec, int iSFIndex, int bMakeNew)
    {
    
    SuPSubframeId   ** ppsuSubframeID = &(psuPRec->psuFirstSubframeId);

    // Walk the linked list of subframe ids, looking for a match or
    // the end of the list
    while (bTRUE)
        {
        // If record pointer in linked list is null then exit
        if (*ppsuSubframeID == NULL)
            {
            break;
            }

        // If the data source number matched then record found, exit
        if ((*ppsuSubframeID)->iCounterNum == iSFIndex)
            {
            break;
            }

        // Not found but next record exists so make it our current pointer
        ppsuSubframeID = &((*ppsuSubframeID)->psuNextSubframeId);
        } // end

    // If no record found then put a new one on the end of the list
    if ((*ppsuSubframeID == NULL) && (bMakeNew == bTRUE))
        {
        // Allocate memory for the new record
        *ppsuSubframeID = (SuPSubframeId *)TmatsMalloc(sizeof(SuPSubframeId));
        memset(*ppsuSubframeID, 0, sizeof(SuPSubframeId));
        (*ppsuSubframeID)->iCounterNum = iSFIndex;

        } // end if new record

    return *ppsuSubframeID;
    }


/* ----------------------------------------------------------------------- */

// Return the P record Subframe ID record with the given index or
// make a new one if necessary.

SuPSubframeDef * psuGetPSubframeDef(SuPSubframeId * psuSubframeId, int iDefIndex, int bMakeNew)
    {
    SuPSubframeDef   ** ppsuSubframeDef = &(psuSubframeId->psuFirstSubframeDef);

    // Walk the linked list of subframe defs, looking for a match or
    // the end of the list
    while (bTRUE)
        {
        // If record pointer in linked list is null then exit
        if (*ppsuSubframeDef == NULL)
            {
            break;
            }

        // If the data source number matched then record found, exit
        if ((*ppsuSubframeDef)->iSubframeDefNum == iDefIndex)
            {
            break;
            }

        // Not found but next record exists so make it our current pointer
        ppsuSubframeDef = &((*ppsuSubframeDef)->psuNextSubframeDef);
        } // end

    // If no record found then put a new one on the end of the list
    if ((*ppsuSubframeDef == NULL) && (bMakeNew == bTRUE))
        {
        // Allocate memory for the new record
        *ppsuSubframeDef = (SuPSubframeDef *)TmatsMalloc(sizeof(SuPSubframeDef));
        memset(*ppsuSubframeDef, 0, sizeof(SuPSubframeDef));
        (*ppsuSubframeDef)->iSubframeDefNum = iDefIndex;

        } // end if new record

    return *ppsuSubframeDef;
    }



/* ----------------------------------------------------------------------- */

SuPSubframeLoc * psuGetPSubframeLoc(SuPSubframeDef *psuSubframeDef, int iLocationIndex, int bMakeNew)
    {
    SuPSubframeLoc ** ppsuSubframeLoc = &(psuSubframeDef->psuFirstSubframeLoc);

    // Walk the linked list of subframe locations, looking for a match or
    // the end of the list
    while (bTRUE)
        {
        // If record pointer in linked list is null then exit
        if (*ppsuSubframeLoc == NULL)
            {
            break;
            }

        // If the data source number matched then record found, exit
        if ((*ppsuSubframeLoc)->iSubframeLocNum == iLocationIndex)
            {
            break;
            }

        // Not found but next record exists so make it our current pointer
        ppsuSubframeLoc = &((*ppsuSubframeLoc)->psuNextSubframeLoc);
        } // end

    // If no record found then put a new one on the end of the list
    if ((*ppsuSubframeLoc == NULL) && (bMakeNew == bTRUE))
        {
        // Allocate memory for the new record
        *ppsuSubframeLoc = (SuPSubframeLoc *)TmatsMalloc(sizeof(SuPSubframeLoc));
        memset(*ppsuSubframeLoc, 0, sizeof(SuPSubframeLoc));
        (*ppsuSubframeLoc)->iSubframeLocNum = iLocationIndex;

        } // end if new record

    return *ppsuSubframeLoc;
    }
#endif
    
    


#ifdef __cplusplus
} // end namespace i106
#endif
