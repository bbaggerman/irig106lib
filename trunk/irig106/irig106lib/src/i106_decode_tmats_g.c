/****************************************************************************

 i106_decode_tmats_g.c - Decode TMATS G fields

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
//#include "i106_decode_tmats_g.h"
#include "i106_decode_tmats.h"

#ifdef __cplusplus
namespace Irig106 {
#endif


/*
 * Macros and definitions
 * ----------------------
 */

// Macros to make decoding G record logic more compact

// Decode a G record
#define DECODE_G(pattern, field)                                                \
    DECODE(pattern, psuGRec->field)

#define DECODE_G_DS(pattern, field)                                             \
    DECODE_1(pattern, field, psuGRec->psuFirstGDataSource, SuGDataSource)

#define DECODE_G_POC(pattern, field)                                            \
    DECODE_1(pattern, field, psuGRec->psuFirstContact, SuPointOfContact)

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

MAKE_GetRecordByIndex(SuGDataSource)
MAKE_GetRecordByIndex(SuPointOfContact)


/* -----------------------------------------------------------------------
 * G Records
 * ----------------------------------------------------------------------- 
 */

int bDecodeGLine(char * szCodeName, char * szDataItem, SuGRecord ** ppsuGRecord)
    {
    int             iTokens;
//    int             iDSIIndex;
    SuGRecord     * psuGRec;
//    SuGDataSource * psuDataSource;

    char            szCodeName2[2000];
    char            szCodeField[2000];
    char          * pcChar;
    int             iIndex1, iIndex2, iIndex3, iIndex4;

    // Parse to get the G record index number, the G record, and the rest of the line
    iTokens = sscanf(szCodeName, "%*1c\\%s", szCodeName2);

    // Get the G record
    psuGRec = *ppsuGRecord;

    // Break the rest of the line apart for further analysis
    for (pcChar=szCodeName2; *pcChar!='\0'; pcChar++)
        if (*pcChar == '-') *pcChar = ' ';
    iTokens = sscanf(szCodeName2, "%s %d %d %d %d", szCodeField, &iIndex1, &iIndex2, &iIndex3, &iIndex4);

    if (bFALSE) {}                              // Keep macro logic happy
    
    DECODE_G(PN, szProgramName)
    DECODE_G(TA, szTestItem)
    DECODE_G(FN, szFilename)
    DECODE_G(106, szIrig106Rev)
    DECODE_G(OD, szOriginationDate)
    DECODE_G(RN, szRevisionNumber)
    DECODE_G(RD, szRevisionDate)
    DECODE_G(UN, szUpdateNumber)
    DECODE_G(UD, szUpdateDate)
    DECODE_G(TN, szTestNumber)

    // Point of Contacts
    DECODE_G(POC\\N, szNumOfContacts)
    DECODE_G_POC(POC1, szName)
    DECODE_G_POC(POC2, szAgency)
    DECODE_G_POC(POC3, szAddress)
    DECODE_G_POC(POC4, szTelephone)

    // Data sources
    DECODE_G(DSI\\N, szNumDataSources)
    DECODE_G_DS(DSI, szDataSourceID)
    DECODE_G_DS(DST, szDataSourceType)

    DECODE_G(TI1, szTestDuration)
    DECODE_G(TI2, szPreTestRequirement)
    DECODE_G(TI3, szPostTestRequirement)
    DECODE_G(SC,  szClassification)
    DECODE_G(SHA, szChecksum)

    else if (strcasecmp(szCodeField, "COM") == 0)
        {
        StoreComment(szDataItem, &psuGRec->psuFirstComment);
        }

    return 0;
    }



