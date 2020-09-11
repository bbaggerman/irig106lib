/****************************************************************************

 i106_decode_tmats_m.c - Decode TMATS M fields

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
#include "i106_decode_tmats_m.h"

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
MAKE_GetRecordByIndex(SuMRecord)

 /* -----------------------------------------------------------------------
 * M Records
 * ----------------------------------------------------------------------- 
 */

// B record specific decode macros
// -------------------------------

// Decode an R record
#define DECODE_M(pattern, field)                                                \
    DECODE(pattern, psuMRec->field)


/* ----------------------------------------------------------------------- */

int bDecodeMLine(char * szCodeName, char * szDataItem, SuMRecord ** ppsuFirstMRecord)
    {
    SuMRecord     * psuMRec;

    char            szCodeName2[2000];
    char            szCodeField[2000];
    char          * pcChar;
    int             iMIdx;
    int             iTokens;
    int             iIndex1, iIndex2, iIndex3, iIndex4;

    // Parse to get the M record index number, the M record, and the rest of the line
    iTokens = sscanf(szCodeName, "%*1c-%i\\%s", &iMIdx, szCodeName2);
    if (iTokens == 2)
        {
        psuMRec = psuGetRecordByIndex_SuMRecord(ppsuFirstMRecord, iMIdx, bTRUE);
        assert(psuMRec != NULL);
        }
    else
        return 1;

    // Break the rest of the line apart for further analysis
    for (pcChar=szCodeName2; *pcChar!='\0'; pcChar++)
        if (*pcChar == '-') *pcChar = ' ';
    iTokens = sscanf(szCodeName2, "%s %d %d %d %d", szCodeField, &iIndex1, &iIndex2, &iIndex3, &iIndex4);

    if (bFALSE) {}                              // Keep macro logic happy

    DECODE_M(ID, szDataSourceID)                // ID - Data Source ID
    DECODE_M(BB1, szSignalStuctType)            // BB1 - Signal Structure Type
    DECODE_M(BB2, szModulationSense)            // BB2 - Modulation Sense
    DECODE_M(BB3, szCompLPFBandwidth)           // BB3 - Composite Low Pass Filter Bandwidth
    DECODE_M(BSG1, szBasebandSignalType)        // BSG1 - Baseband Signal Type
    DECODE_M(BSF1, szBasebandLPFBandwidth)      // BSF1 - Baseband Low Pass Filter Bandwidth
    DECODE_M(BSF2, szBasebandLPFType)           // BSF2 - Baseband Low Pass Filter Type
    DECODE_M(BB\\DLN, szBBDataLinkName)         // BB\DLN - Baseband Data Link Name (PCM)
    DECODE_M(BB\\MN, szMeasurementName)         // BB\MN  - Measurement Name (Analog)

    return -1;
    } // end bDecodeMLine



#ifdef __cplusplus
} // end namespace i106
#endif
