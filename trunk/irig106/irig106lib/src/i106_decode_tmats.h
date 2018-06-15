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

 ****************************************************************************/

#ifndef _I106_DECODE_TMATS_H
#define _I106_DECODE_TMATS_H

#include "i106_decode_tmats_g.h"
#include "i106_decode_tmats_r.h"
#include "i106_decode_tmats_p.h"

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */

/// TMATS signature generating options
#define TMATS_SIGFLAG_NONE          0x0000
#define TMATS_SIGFLAG_INC_ALL       0x0001  ///< Include all fields
#define TMATS_SIGFLAG_INC_COMMENT   0x0002  ///< Include comment fields
#define TMATS_SIGFLAG_INC_VENDOR    0x0004  ///< Include vendor fields
#define TMATS_SIGFLAG_INC_G         0x0008  ///< Exclude G fields

/// TMATS signature version
#define TMATS_SIGVER_1              1
#define TMATS_SIGVER_DEFAULT        TMATS_SIGVER_1
#define TMATS_SIGVER_MAX            TMATS_SIGVER_1

/*
 * Data structures
 * ---------------
 */

// Channel specific data word
// --------------------------

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

typedef PUBLIC struct Tmats_ChanSpec_S
    {
    uint32_t    iCh10Ver        :  8;      // Recorder Ch 10 Version
    uint32_t    bConfigChange   :  1;      // Recorder configuration changed
    uint32_t    iFormat         :  1;      // TMATS / XML Format
    uint32_t    iReserved       : 22;      // Reserved
#if !defined(__GNUC__)
    } SuTmats_ChanSpec;
#else
    } __attribute__ ((packed)) SuTmats_ChanSpec;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

// NEED TO ADD STORAGE FOR REQUIRED DATA FIELDS
// NEED TO ADD SUPPORT OF "OTHER" DATA FIELDS TO PERMIT TMATS WRITE

// B Records
// ---------

typedef PUBLIC struct SuBRecord_S
    {
    int                         iRecordNum;             // B-x
    char                      * szDataLinkName;         // B-x\DLN
    char                      * szNumBuses;             // B-x\NBS\N
//    int                         iNumBuses;              
    struct SuBRecord_S        * psuNextBRecord;
    } SuBRecord;


// M Records
// ---------

typedef PUBLIC struct SuMRecord_S
    {
    int                         iRecordNum;             // M-x
//    char                      * szRecordNum;            // M-x
    char                      * szDataSourceID;         // M-x\ID
    char                      * szBBDataLinkName;       // M-x\BB\DLN
    char                      * szBasebandSignalType;   // M-x\BSG1
    struct SuPRecord_S        * psuPRecord;             // Corresponding P record
    struct SuBRecord_S        * psuBRecord;             // Corresponding B record
    struct SuMRecord_S        * psuNextMRecord;         // Used to keep track of M records
    } SuMRecord;


// Memory linked list
// ------------------

// Linked list that keeps track of malloc'ed memory
typedef PUBLIC struct MemBlock_S
    {
    void                    * pvMemBlock;
    struct MemBlock_S       * psuNextMemBlock;
    } SuMemBlock;

// TMATS lines
// -----------

typedef PUBLIC struct TmatsLines_S
    {
    char                    * szCodeName;
    char                    * szDataItem;
    } SuTmatsLine;

// Decoded TMATS info
// ------------------

typedef PUBLIC struct SuTmatsInfo_S
    {
    SuTmatsLine    * pasuTmatsLines;
    unsigned long    ulTmatsLines;
    unsigned long    ulTmatsLinesAvail;
    int              iCh10Ver;
    int              bConfigChange;
    SuComment      * psuFirstComment;   // Comments of the form "COMMENT:comment;"
    SuGRecord      * psuFirstGRecord;
    SuRRecord      * psuFirstRRecord;
    SuMRecord      * psuFirstMRecord;
    SuBRecord      * psuFirstBRecord;
    SuPRecord      * psuFirstPRecord;
    void           * psuFirstTRecord;
    void           * psuFirstDRecord;
    void           * psuFirstSRecord;
    void           * psuFirstARecord;
    void           * psuFirstCRecord;
    void           * psuFirstHRecord;
    void           * psuFirstVRecord;
    SuMemBlock     * psuFirstMemBlock;
    } SuTmatsInfo;

/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_Tmats(SuI106Ch10Header  * psuHeader,
                        void              * pvBuff,
                        SuTmatsInfo       * psuTmatsInfo);

EnI106Status I106_CALL_DECL 
    enI106_Decode_Tmats_Text(void         * pvBuff,
                             uint32_t       ulDataLen,
                             SuTmatsInfo  * psuTmatsInfo);
 
char * I106_CALL_DECL
    enI106_Tmats_Find(SuTmatsInfo         * psuTmatsInfo,
                      char                * szTmatsCode);

void I106_CALL_DECL 
    enI106_Free_TmatsInfo(SuTmatsInfo     * psuTmatsInfo);

I106_CALL_DECL EnI106Status 
    enI106_Encode_Tmats(SuI106Ch10Header  * psuHeader,
                        void              * pvBuff,
                        char              * szTMATS);

#if 0
I106_CALL_DECL EnI106Status 
    enI106_Tmats_Signature(void         * pvBuff,       ///< TMATS text without CSDW
                           uint32_t       ulDataLen,    ///< Length of TMATS in pvBuff
                           int            iSigVersion,  ///< Request signature version (0 = default)
                           int            iSigFlags,    ///< Additional flags
                           uint16_t     * piOpCode,     ///< Version and flag op code
                           uint32_t     * piSignature); ///< TMATS signature
 #else
I106_CALL_DECL EnI106Status 
    enI106_Tmats_Signature(SuTmatsLine  * aszLines,     ///< Array of TMATS lines
                           unsigned long  ulTmatsLines, ///< Number of TMATS line in array
                           int            iSigVersion,  ///< Request signature version (0 = default)
                           int            iSigFlags,    ///< Additional flags
                           uint16_t     * piOpCode,     ///< Version and flag op code
                           uint32_t     * piSignature); ///< TMATS signature

#ifdef SHA256
I106_CALL_DECL EnI106Status 
    enI106_Tmats_IRIG_Signature(void    * pvBuff,       ///< TMATS text without CSDW
                                uint32_t  ulDataLen,    ///< Length of TMATS in pvBuff
                                uint8_t   auHash[]);     ///< 32 byte array for SHA-256
#endif

#endif

void * TmatsMalloc(size_t iSize);
char * szFirstNonWhitespace(char * szInString);

void StoreComment(char * szComment, SuComment ** ppsuFirstComment);

#ifdef __cplusplus
}
}
#endif

#endif
