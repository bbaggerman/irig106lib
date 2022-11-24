/****************************************************************************

 i106_decode_tmats_g.h - Decode TMATS G fields

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

#ifndef _I106_DECODE_TMATS_G_H
#define _I106_DECODE_TMATS_G_H

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

// G Records
// ---------

// G record, data source
typedef PUBLIC struct SuGDataSource_S
    {
    int                         iIndex;                 // n
    char                      * szDataSourceID;         // G\DSI-n
    char                      * szDataSourceType;       // G\DST-n
    struct SuRRecord_S        * psuRRecord;             // Corresponding R record
    struct SuTRecord_S        * psuTRecord;             // Corresponding T record
    struct SuMRecord_S        * psuMRecord;             // Corresponding M record
    struct SuGDataSource_S    * psuNext;
    } SuGDataSource;

// G record
typedef PUBLIC struct GRecord_S
    {
    char                      * szProgramName;          // G\PN
    char                      * szTestItem;             // G\TA
    char                      * szFilename;             // G\FN
    char                      * szIrig106Rev;           // G\106
    char                      * szOriginationDate;      // G\OD
    char                      * szRevisionNumber;       // G\RN
    char                      * szRevisionDate;         // G\RD
    char                      * szUpdateNumber;         // G\UN
    char                      * szUpdateDate;           // G\UD
    char                      * szTestNumber;           // G\TN
    char                      * szNumOfContacts;        // G\POC\N
    SuPointOfContact          * psuFirstContact;        
    char                      * szNumDataSources;       // G\DSI\N
    SuGDataSource             * psuFirstGDataSource;
    char                      * szTestDuration;         // G\TI1
    char                      * szPreTestRequirement;   // G\TI2
    char                      * szPostTestRequirement;  // G\TI3
    char                      * szClassification;       // G\SC
    char                      * szChecksum;             // G\SHA
    SuComment                 * psuFirstComment;        // G\COM
    } SuGRecord;

/*
 * Function Declaration
 * --------------------
 */

int bDecodeGLine(char * szCodeName, char * szDataItem, SuGRecord ** ppsuFirstGRec);

#ifdef __cplusplus
}
}
#endif

#endif
