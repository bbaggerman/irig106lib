/****************************************************************************

 i106_decode_tmats.h - 

 Copyright (c) 2005 Irig106.org

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

 Created by Bob Baggerman

 $RCSfile: i106_decode_tmats.h,v $
 $Date: 2006-10-01 17:16:43 $
 $Revision: 1.12 $

 ****************************************************************************/

#ifndef _I106_DECODE_TMATS_H
#define _I106_DECODE_TMATS_H

#ifdef __cplusplus
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

// NEED TO ADD STORAGE FOR REQUIRED DATA FIELDS
// NEED TO ADD SUPPORT OF "OTHER" DATA FIELDS TO PERMIT TMATS WRITE

// B Records
// ---------

typedef struct SuBRecord_S
    {
    int                         iRecordNum;             // B-x
    char                      * szDataLinkName;         // B-x\DLN
    int                         iNumBuses;              // B-x\NBS\N
    struct SuBRecord_S        * psuNextBRecord;
    } SuBRecord;


// M Records
// ---------

typedef struct SuMRecord_S
    {
    int                         iRecordNum;             // M-x
    char                      * szDataSourceID;         // M-x\ID
    char                      * szDataLinkName;         // M-x\BB\DLN
    char                      * szBasebandSignalType;   // M-x\BSG1
    struct SuBRecord_S        * psuBRecord;             // Corresponding B record
    struct SuMRecord_S        * psuNextMRecord;
    } SuMRecord;


// R Records
// ---------

// R record data source
typedef struct SuRDataSource_S
    {
    int                         iDataSourceNum;         // R-x\XXX-n
    char                      * szDataSourceID;         // R-x\DSI-n
    char                      * szChannelDataType;      // R-x\CDT-n
    int                         iTrackNumber;           // R-x\TK1-n
    struct SuMRecord_S        * psuMRecord;             // Corresponding M record
    struct SuRDataSource_S    * psuNextRDataSource;
    } SuRDataSource;    

// R record
typedef struct SuRRecord_S
    {
    int                         iRecordNum;             // R-x
    char                      * szDataSourceID;         // R-x\ID
    int                         iNumDataSources;        // R-x\N
    SuRDataSource             * psuFirstDataSource;     //
    struct SuRRecord_S        * psuNextRRecord;
    } SuRRecord;


// G Records
// ---------

// G record, data source
typedef struct SuGDataSource_S
    {
    int                         iDataSourceNum;         // G\XXX-n
    char                      * szDataSourceID;         // G\DSI-n
    char                      * szDataSourceType;       // G\DST-n
    struct SuRRecord_S        * psuRRecord;             // Corresponding R record
    struct SuGDataSource_S    * psuNextGDataSource;
    } SuGDataSource;

// G record
typedef struct
    {
    char                      * szProgramName;          // G\PN
    char                      * szIrig106Rev;           // G\106
    int                         iNumDataSources;        // G\DSI\N
    SuGDataSource             * psuFirstGDataSource;
    } SuGRecord;


// Decoded TMATS info
// ------------------

typedef struct
    {
    SuGRecord      * psuFirstGRecord;
    SuRRecord      * psuFirstRRecord;
    SuMRecord      * psuFirstMRecord;
    SuBRecord      * psuFirstBRecord;
    void           * psuFirstTRecord;
    void           * psuFirstPRecord;
    void           * psuFirstDRecord;
    void           * psuFirstSRecord;
    void           * psuFirstARecord;
    void           * psuFirstCRecord;
    void           * psuFirstHRecord;
    void           * psuFirstVRecord;
    } SuTmatsInfo;

/*
 * Function Declaration
 * --------------------
 */

I106_CALL_DECL EnI106Status 
    enI106_Decode_Tmats(SuI106Ch10Header * psuHeader,
                        void             * pvBuff,
                        unsigned long      iBuffSize,
                        SuTmatsInfo      * psuTmatsInfo);


I106_CALL_DECL EnI106Status 
    enI106_Encode_Tmats(SuI106Ch10Header * psuHeader,
                        void             * pvBuff,
                        char             * szTMATS);

#ifdef __cplusplus
}
#endif

#endif
