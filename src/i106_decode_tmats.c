/****************************************************************************

 i106_decode_tmats.c - 

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <assert.h>

#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
#include "i106_decode_tmats.h"

#ifdef SHA256ENABLE
#include "sha-256.h"
#endif

#ifdef __cplusplus
namespace Irig106 {
#endif

/******************************************************************************

Here's how this module decodes and stores TMATS data. 

Any field that is to be decoded and stored must have a corresponding entry 
in one of the defined data structures.  In other words, there is no support 
for storing and later retrieving arbitrary TMATS lines.  Maybe that would have 
been better, but for now only TMATS lines that are understood by this module
will be stored.

This module makes no assumptions about the ordering, numbering, or
numbers of TMATS lines. Information is stored in linked lists that are
created and grow as needed.  For now there is the assumption that there
is only one G record, but there may be multiples of other records.

There is a linked list for each type of record (except the one and only
G record).  As the TMATS lines are read one by one, the info is decoded
and stored in the appropriate existing record, or a new record is create
if necessary.

After all TMATS lines are read and decoded, the linked lists are scanned
and connected into a tree.  That is, R records are connected to the
corresponding G, M records are connected to the corresponding R, etc.
When done, the TMATS info is organized into a tree similar to that depicted
in IRIG 106 Chapter 9 (TMATS) Figure 9-1 "Group relationships".

There are at least two ways to use this information.  One is to start with
the top level G record and walk the tree.  This is a good way to provide
access to all the decoded data, for example, to print out everything in 
the tree.  The other way to access data is to start at the beginning of
one of the linked lists of records, and walk the linked list.  This might
be a good way to get just some specific data, like a list of Channel ID's
for 1553IN type data sources.

******************************************************************************/


// NEED TO ADD STORAGE FOR REQUIRED DATA FIELDS
// NEED TO ADD SUPPORT OF "OTHER" DATA FIELDS TO PERMIT TMATS WRITE

/*
 * Macros and definitions
 * ----------------------
 */

#define CR      (13)
#define LF      (10)

/*
 * Data structures
 * ---------------
 */

// 1553 bus attributes
// -------------------

/*
 * Module data
 * -----------
 */

// This is an empty string that text fields can point to before
// they get a value. This ensures that if fields don't get set while
// reading the TMATS record they will point to something benign.
//char                    m_szEmpty[] = "";

//static SuGRecord      * m_psuFirstGRecord;
//static SuRRecord      * m_psuFirstRRecord = NULL;
//static SuMRecord      * m_psuFirstMRecord = NULL;
//static SuBRecord      * m_psuFirstBRecord = NULL;

static SuTmatsInfo      * m_psuTmatsInfo;
static int                m_iTmatsVersion = 0;

/*
 * Function Declaration
 * --------------------
 */

void TmatsBufferToLines(void * pvBuff, uint32_t ulDataLen, SuTmatsInfo * psuTmatsInfo);

int bDecodeMLine(char * szCodeName, char * szDataItem, SuMRecord ** ppsuFirstMRec);
int bDecodeBLine(char * szCodeName, char * szDataItem, SuBRecord ** ppsuFirstBRec);
//int bDecodePLine(char * szCodeName, char * szDataItem, SuPRecord ** ppsuFirstPRec);

SuMRecord          * psuGetMRecord(SuMRecord ** ppsuFirstMRec, int iRIndex, int bMakeNew);
SuBRecord          * psuGetBRecord(SuBRecord ** ppsuFirstBRec, int iRIndex, int bMakeNew);
//SuPRecord          * psuGetPRecord(SuPRecord ** ppsuFirstPRec, int iRIndex, int bMakeNew);
//
//SuPAsyncEmbedded   * psuGetPAsyncEmbedded(SuPRecord      * psuPRec,        int iAEFIndex, int bMakeNew);
//SuPSubframeId      * psuGetPSubframeID   (SuPRecord      * psuPRec,        int iSFIndex,  int bMakeNew);
//SuPSubframeDef     * psuGetPSubframeDef  (SuPSubframeId  * psuSubframeId,  int iDefIndex, int bMakeNew);
//SuPSubframeLoc     * psuGetPSubframeLoc  (SuPSubframeDef * psuSubframeDef, int iLocIndex, int bMakeNew);

void vConnectG(SuTmatsInfo * psuTmatsInfo);
void vConnectR(SuTmatsInfo * psuTmatsInfo);
void vConnectM(SuTmatsInfo * psuTmatsInfo);
void vConnectP(SuTmatsInfo * psuTmatsInfo);

// void * TmatsMalloc(size_t iSize);

char * szFirstNonWhitespace(char * szInString);

uint32_t Fletcher32(uint8_t * data, int count);

/* ======================================================================= */

/* The idea behind this routine is to read the TMATS record, parse it, and 
 * put the various data fields into a tree structure that can be used later
 * to find various settings.  This routine assumes the buffer is a complete
 * TMATS record from a file, including the Channel Specific Data Word.  After
 * pulling out the CSDW stuff, it punts to the text decoder which does the
 * actual heaving lifting.
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_Tmats(SuI106Ch10Header * psuHeader,
                        void             * pvBuff,
                        SuTmatsInfo      * psuTmatsInfo)
    {
    EnI106Status        enStatus;
    SuTmats_ChanSpec  * psuTmats_ChanSpec;
    void              * pvTmatsText;

    // Decode the text portion
    pvTmatsText = (char *)pvBuff + sizeof(SuTmats_ChanSpec);
    enStatus = enI106_Decode_Tmats_Text(pvTmatsText, psuHeader->ulDataLen-sizeof(SuTmats_ChanSpec), psuTmatsInfo);

    // Decode any available info from channel specific data word
    psuTmats_ChanSpec           = (SuTmats_ChanSpec *)pvBuff;
    psuTmatsInfo->iCh10Ver      = psuTmats_ChanSpec->iCh10Ver;
    psuTmatsInfo->bConfigChange = psuTmats_ChanSpec->bConfigChange;

    return enStatus;
    }


// ------------------------------------------------------------------------

// This routine parses just the text portion of TMATS.

EnI106Status I106_CALL_DECL 
    enI106_Decode_Tmats_Text(void             * pvBuff,
                             uint32_t           ulDataLen,
                             SuTmatsInfo      * psuTmatsInfo)
    {
    unsigned long       iLineIdx;
    char              * szCodeName;
    char              * szDataItem;
    int                 bParseError;

    // Store a copy for module wide use
    m_psuTmatsInfo = psuTmatsInfo;

    // Initialize the TMATS info data structure
    enI106_Free_TmatsInfo(psuTmatsInfo);

    // Read the buffer into an array of lines
    TmatsBufferToLines(pvBuff, ulDataLen, psuTmatsInfo);

    // Initialize the first (and only, for now) G record
    psuTmatsInfo->psuFirstGRecord = (SuGRecord *)TmatsMalloc(sizeof(SuGRecord));
    memset(psuTmatsInfo->psuFirstGRecord, 0, sizeof(SuGRecord));

    // Step through the array of TMATS lines
    iLineIdx = 0;
    while (iLineIdx < psuTmatsInfo->ulTmatsLines)
        {
        // Code Name get parsed some more so make a copy of it
        szCodeName = strdup(psuTmatsInfo->pasuTmatsLines[iLineIdx].szCodeName);
        szDataItem = psuTmatsInfo->pasuTmatsLines[iLineIdx].szDataItem;

        // Decode comments
        if (stricmp(szCodeName, "COMMENT") == 0)
            {
            StoreComment(szDataItem, &(psuTmatsInfo->psuFirstComment));
            } // end if comment

        // Decode everything else
        else
            {
            // Determine and decode different TMATS types
            switch (szCodeName[0])
            {
                case 'G' : // General Information
                    bParseError = bDecodeGLine(szCodeName,
                                               szDataItem, 
                                               &psuTmatsInfo->psuFirstGRecord);
                    break;

                case 'B' : // Bus Data Attributes
                    bParseError = bDecodeBLine(szCodeName, 
                                               szDataItem,
                                               &psuTmatsInfo->psuFirstBRecord);
                    break;

                case 'R' : // Tape/Storage Source Attributes
                    bParseError = bDecodeRLine(szCodeName, 
                                               szDataItem,
                                               &psuTmatsInfo->psuFirstRRecord);
                    break;

                case 'T' : // Transmission Attributes
                    break;

                case 'M' : // Multiplexing/Modulation Attributes
                    bParseError = bDecodeMLine(szCodeName, 
                                               szDataItem,
                                               &psuTmatsInfo->psuFirstMRecord);
                    break;

                case 'P' : // PCM Format Attributes
                    bParseError = bDecodePLine(szCodeName, 
                                               szDataItem,
                                               &psuTmatsInfo->psuFirstPRecord);
                    break;

                case 'D' : // PCM Measurement Description
                    break;

                case 'S' : // Packet Format Attributes
                    break;

                case 'A' : // PAM Attributes
                    break;

                case 'C' : // Data Conversion Attributes
                    break;

                case 'H' : // Airborne Hardware Attributes
                    break;

                case 'V' : // Vendor Specific Attributes
                    break;

                default :
                    break;

                } // end decoding switch
            } // end if not comment

        // Next TMATS line
        iLineIdx++;

        } // end looping forever on reading TMATS buffer


    // Figure out the TMATS version
    if (psuTmatsInfo->psuFirstGRecord->szIrig106Rev != NULL)
        m_iTmatsVersion = atoi(psuTmatsInfo->psuFirstGRecord->szIrig106Rev);

    /* Now link the various records together into a tree.  This is a bit involved.
    
    G/DSI-n <-+-> T-x\ID
              +-> M-x\ID
    
    G/DSI-n <---> R-x\ID
                  R-x\DSI-n <---> M-x\ID
                                  M-x\BB\DLN   <-+-> P-x\DLN  \
                                                 +-> B-x\DLN   - With M-x Baseband
                                                 +-> S-x\DLN  /

                                  M-x\SI\DLN-n <-+-> P-x\DLN  \
                                                 +-> B-x\DLN   - With M-x Subchannels
                                                 +-> S-x\DLN  /

                  R-x\CDLN-n <-------------------+-> P-x\DLN  \
                                                 +-> B-x\DLN   - Without M-x
                                                 +-> S-x\DLN  /

    -> D-x\DLN
    -> D-x\DLN

    -> D-x\DLN
    */

    vConnectG(psuTmatsInfo);
    vConnectR(psuTmatsInfo);
    vConnectM(psuTmatsInfo);
    vConnectP(psuTmatsInfo);

    m_psuTmatsInfo = NULL;

    return I106_OK;
    }



// ------------------------------------------------------------------------

char * enI106_Tmats_Find(SuTmatsInfo * psuTmatsInfo, char * szTmatsCode)
    {
    unsigned long   iLineIdx;

    // Step through each TMATS line looking for the code string to match
    for (iLineIdx = 0; iLineIdx < psuTmatsInfo->ulTmatsLines; iLineIdx++)
        {
        if (strcasecmp(szTmatsCode, psuTmatsInfo->pasuTmatsLines[iLineIdx].szCodeName) == 0)
            return psuTmatsInfo->pasuTmatsLines[iLineIdx].szDataItem;
        }

    return NULL;
    }


// ------------------------------------------------------------------------

void TmatsBufferToLines(void             * pvBuff,
                        uint32_t           ulDataLen,
                        SuTmatsInfo      * psuTmatsInfo)

    {
    uint32_t      iInBuffIdx = 0;
    char          szLine[2048];
    char        * achInBuff;
    int           iLineIdx;
    char        * szCodeName;
    char        * szDataItem;

    // Init buffer pointers
    achInBuff    = (char *)pvBuff;
    iInBuffIdx   = 0;
    psuTmatsInfo->ulTmatsLines = 0;

    // Loop until we get to the end of the buffer
    while (bTRUE)
        {

        // Initialize input line buffer
        szLine[0] = '\0';
        iLineIdx  = 0;

        // If at the end of the buffer then break out of the big loop
        if (iInBuffIdx >= ulDataLen)
            break;

        // Fill a local buffer with one line
        // ---------------------------------

        // Read from buffer until complete line
        while (bTRUE)
            {
            // If at the end of the buffer then break out
            if (iInBuffIdx >= ulDataLen)
                break;

            // If CR or LF then swallow them, they mean nothing to TMATS
            // Else copy next character to line buffer
            if ((achInBuff[iInBuffIdx] != CR)  &&
                (achInBuff[iInBuffIdx] != LF))
                {
                szLine[iLineIdx] = achInBuff[iInBuffIdx];
                if (iLineIdx < 2048)
                  iLineIdx++;
                szLine[iLineIdx] = '\0';
                }
#if 0
            // Sometimes we need to be nice and treat a CR or LF like an end of line.
            // In particular, the TMATS produced by the F-16 DVADR recorder doesn't terminate
            // comments correctly.
            else
                {
                }
#endif

            // Next character from buffer
            iInBuffIdx++;

            // If line terminator and line buffer not empty then break out
            if (achInBuff[iInBuffIdx-1] == ';')
                {
                if (strlen(szLine) != 0)
                    break;
                } // end if line terminator

            } // end while filling complete line

        // Decode the TMATS line
        // ---------------------

        // Go ahead and split the line into left hand and right hand sides
        szCodeName = strtok(szLine, ":");
        szDataItem = strtok(NULL, ";");

        // Store the TMATS line
        // --------------------

        // Only keep properly formatted TMATS lines
        if ((szCodeName != NULL) && (szDataItem != NULL))
            {
            psuTmatsInfo->ulTmatsLines++;
            // Make sure we have enough lines in the array of lines
            if (psuTmatsInfo->ulTmatsLines >= psuTmatsInfo->ulTmatsLinesAvail)
                {
                psuTmatsInfo->ulTmatsLinesAvail += 100;
                psuTmatsInfo->pasuTmatsLines = (SuTmatsLine  *)realloc(psuTmatsInfo->pasuTmatsLines, 
                                                                       sizeof(SuTmatsLine ) * psuTmatsInfo->ulTmatsLinesAvail);
                } // end if extend lines array

            // Malloc some memory for the line
            psuTmatsInfo->pasuTmatsLines[psuTmatsInfo->ulTmatsLines-1].szCodeName = (char *)TmatsMalloc(strlen(szCodeName)+1);
            psuTmatsInfo->pasuTmatsLines[psuTmatsInfo->ulTmatsLines-1].szDataItem = (char *)TmatsMalloc(strlen(szDataItem)+1);

            // Save the values
            strcpy(psuTmatsInfo->pasuTmatsLines[psuTmatsInfo->ulTmatsLines-1].szCodeName, szCodeName);
            strcpy(psuTmatsInfo->pasuTmatsLines[psuTmatsInfo->ulTmatsLines-1].szDataItem, szDataItem);
            } // end if TMATS line OK
        } // end while reading from input buffer

    return;
    }


/* ----------------------------------------------------------------------- */

/*
    Tie the G record data sources to their underlying R and T
    records.

    For recorder case...
    G/DSI-n <---> R-x\ID

    For telemetry case...
    G/DSI-n <-+-> T-x\ID
              +-> R-x\ID

*/

void vConnectG(SuTmatsInfo * psuTmatsInfo)
    {
    SuRRecord       * psuCurrRRec;
    SuGDataSource   * psuCurrGDataSrc;

    // Step through the G data source records
    psuCurrGDataSrc = psuTmatsInfo->psuFirstGRecord->psuFirstGDataSource;
    while (psuCurrGDataSrc != NULL)
        {
        // Walk through the R records linked list looking for a match
        psuCurrRRec = psuTmatsInfo->psuFirstRRecord;
        while (psuCurrRRec != NULL)
            {
            // See if G/DSI-n = R-x\ID
            if ((psuCurrGDataSrc->szDataSourceID != NULL) &&
                (psuCurrRRec->szDataSourceID     != NULL) &&
                (strcasecmp(psuCurrGDataSrc->szDataSourceID,
                        psuCurrRRec->szDataSourceID) == 0))
                {
                // Note, if psuCurrGDataSrc->psuRRecord != NULL then that 
                // is probably an error in the TMATS file
                assert(psuCurrGDataSrc->psuRRecord == NULL);
                psuCurrGDataSrc->psuRRecord = psuCurrRRec;
                } // end if match

            // Get the next R record
            psuCurrRRec = psuCurrRRec->psuNext;
            } // end while walking the R record list

        // Get the next G data source record
        psuCurrGDataSrc = psuCurrGDataSrc->psuNext;
        } // end while walking the G data source records

    return;
    } // end vConnectG()


/* ----------------------------------------------------------------------- */

/*
    Tie the R record data sources to their underlying M records. In some
    cases the M record is skipped and the P, B, and S records tie directly.
    The fields used for these ties varies based on the Ch 9 release version.

    R-x\DSI-n <---> M-x\ID

    106-04 and 106-05 version...
    R-x\PDLN-n <---------------------> P-x\DLN   - Without M-x
    R-x\BDLN-n <---------------------> B-x\DLN   - Without M-x

    106-07 and 106-09 version...
    R-x\CDLN-n <-------------------+-> P-x\DLN  \
                                   +-> B-x\DLN   - Without M-x
                                   +-> S-x\DLN  /
*/

void vConnectR(SuTmatsInfo * psuTmatsInfo)
    {
    SuRRecord       * psuCurrRRec;
    SuRDataSource   * psuCurrRDataSrc;
    SuMRecord       * psuCurrMRec;
    SuPRecord       * psuCurrPRec;

    // Walk the linked list of R records
    psuCurrRRec = psuTmatsInfo->psuFirstRRecord;
    while (psuCurrRRec != NULL)
        {

        // Walk the linked list of R data sources
        psuCurrRDataSrc = psuCurrRRec->psuFirstDataSource;
        while (psuCurrRDataSrc != NULL)
            {

            // Walk through the M records linked list
            psuCurrMRec = psuTmatsInfo->psuFirstMRecord;
            while (psuCurrMRec != NULL)
                {
                // See if R-x\DSI-n = M-x\ID
                if ((psuCurrRDataSrc->szDataSourceID != NULL) &&
                    (psuCurrMRec->szDataSourceID     != NULL) &&
                    (strcasecmp(psuCurrRDataSrc->szDataSourceID,
                                psuCurrMRec->szDataSourceID) == 0))
                    {
                    // Note, if psuCurrRDataSrc->psuMRecord != NULL then that 
                    // is probably an error in the TMATS file
                    assert(psuCurrRDataSrc->psuMRecord == NULL);
                    psuCurrRDataSrc->psuMRecord = psuCurrMRec;
                    }

                // Get the next M record
                psuCurrMRec = psuCurrMRec->psuNextMRecord;
                } // end while walking the M record list

            // Walk through the P records linked list
            psuCurrPRec = psuTmatsInfo->psuFirstPRecord;
            while (psuCurrPRec != NULL)
                {
                // R to P tieing changed with the -07 release.  Try to do it the
                // "right" way first, but accept the "wrong" way if that doesn't work.
                // TMATS 04 and 05
                if ((m_iTmatsVersion == 4) ||
                    (m_iTmatsVersion == 5))
                    {
                    // See if R-x\PDLN-n = P-x\DLN, aka the "right" way
                    if ((psuCurrRDataSrc->szPcmDataLinkName  != NULL) &&
                        (psuCurrPRec->szDataLinkName         != NULL) &&
                        (strcasecmp(psuCurrRDataSrc->szPcmDataLinkName,
                                    psuCurrPRec->szDataLinkName) == 0))
                        {
                        // Note, if psuCurrRDataSrc->psuPRecord != NULL then that 
                        // is probably an error in the TMATS file
                        assert(psuCurrRDataSrc->psuPRecord == NULL);
                        psuCurrRDataSrc->psuPRecord = psuCurrPRec;
                        }

                    // Try some "wrong" ways
                    } // end if TMATS 04 or 05

                // TMATS 07, 09, and beyond (I hope)
                else
                    {
                    // See if R-x\CDLN-n = P-x\DLN, aka the "right" way
                    if ((psuCurrRDataSrc->szChanDataLinkName != NULL) &&
                        (psuCurrPRec->szDataLinkName         != NULL) &&
                        (strcasecmp(psuCurrRDataSrc->szChanDataLinkName,
                                    psuCurrPRec->szDataLinkName) == 0))
                        {
                        // Note, if psuCurrRDataSrc->psuPRecord != NULL then that 
                        // is probably an error in the TMATS file
                        assert(psuCurrRDataSrc->psuPRecord == NULL);
                        psuCurrRDataSrc->psuPRecord = psuCurrPRec;
                        }

                    // Try some "wrong" ways
                    } // end if TMATS 07 or 09 (or beyond)

                // Get the next P record
                psuCurrPRec = psuCurrPRec->psuNext;
                } // end while walking the P record list

            // Walk the P, B, and S record link lists

            // Get the next R data source record
            psuCurrRDataSrc = psuCurrRDataSrc->psuNext;
            } // end while walking the R data source records

        // Get the next R record
        psuCurrRRec = psuCurrRRec->psuNext;
        }

    return;
    } // end vConnectR()


/* -----------------------------------------------------------------------
 * M Records
 * ----------------------------------------------------------------------- 
 */

int bDecodeMLine(char * szCodeName, char * szDataItem, SuMRecord ** ppsuFirstMRecord)
    {
    char          * szCodeField;
    int             iTokens;
    int             iRIdx;
    SuMRecord     * psuMRec;

    // See which M field it is
    szCodeField = strtok(szCodeName, "\\");
    assert(szCodeField[0] == 'M');

    // Get the M record index number
    iTokens = sscanf(szCodeField, "%*1c-%i", &iRIdx);
    if (iTokens == 1)
        {
        psuMRec = psuGetMRecord(ppsuFirstMRecord, iRIdx, bTRUE);
        assert(psuMRec != NULL);
        }
    else
        return 1;
    
    szCodeField = strtok(NULL, "\\");

    // ID - Data source ID
    if     (strcasecmp(szCodeField, "ID") == 0)
        {
        psuMRec->szDataSourceID = szDataItem;
        } // end if ID

    // BSG1 - Baseband signal type
    else if (strcasecmp(szCodeField, "BSG1") == 0)
        {
        psuMRec->szBasebandSignalType = szDataItem;
        } // end if BSG1

    // BB\DLN - Data link name
    else if (strcasecmp(szCodeField, "BB") == 0)
        {
        szCodeField = strtok(NULL, "\\");
        // DLN - Data link name
        if (strcasecmp(szCodeField, "DLN") == 0)
            {
            psuMRec->szBBDataLinkName = szDataItem;
            }
        } // end if BB\DLN

    return 0;
    }



/* ----------------------------------------------------------------------- */

SuMRecord * psuGetMRecord(SuMRecord ** ppsuFirstMRecord, int iRIndex, int bMakeNew)
    {
    SuMRecord   ** ppsuCurrMRec = ppsuFirstMRecord;

    // Loop looking for matching index number or end of list
    while (bTRUE)
        {
        // Check for end of list
        if (*ppsuCurrMRec == NULL)
            break;

        // Check for matching index number
        if ((*ppsuCurrMRec)->iRecordNum == iRIndex)
            break;

        // Move on to the next record in the list
        ppsuCurrMRec = &((*ppsuCurrMRec)->psuNextMRecord);
        }

    // If no record found then put a new one on the end of the list
    if ((*ppsuCurrMRec == NULL) && (bMakeNew == bTRUE))
        {
        // Allocate memory for the new record
        *ppsuCurrMRec = (SuMRecord *)TmatsMalloc(sizeof(SuMRecord));
        memset(*ppsuCurrMRec, 0, sizeof(SuMRecord));
        (*ppsuCurrMRec)->iRecordNum  = iRIndex;
        }

    return *ppsuCurrMRec;
    }



/* ----------------------------------------------------------------------- */

/*
    Tie the M record baseband and subchannel sources to their underlying P,
    B, and S records.

    Baseband case...
    M-x\BB\DLN   <-+-> P-x\DLN
                   +-> B-x\DLN
                   +-> S-x\DLN

    Subchannel case...
    M-x\SI\DLN-n <-+-> P-x\DLN
                   +-> B-x\DLN
                   +-> S-x\DLN

*/

void vConnectM(SuTmatsInfo * psuTmatsInfo)
    {
    SuMRecord       * psuCurrMRec;
    SuBRecord       * psuCurrBRec;
    SuPRecord       * psuCurrPRec;

    // Walk the linked list of M records
    psuCurrMRec = psuTmatsInfo->psuFirstMRecord;
    while (psuCurrMRec != NULL)
        {

        // Walk through the P record linked list
        psuCurrPRec = psuTmatsInfo->psuFirstPRecord;
        while (psuCurrPRec != NULL)
            {

            // Note, if psuCurrRRecord->psuPRecord != NULL then that 
            // is probably an error in the TMATS file
// HMMM... CHECK THESE TIE FIELDS
            if ((m_iTmatsVersion == 4) ||
                (m_iTmatsVersion == 5))
                {
                if ((psuCurrMRec->szBBDataLinkName != NULL) &&
                    (psuCurrPRec->szDataLinkName   != NULL) &&
                    (strcasecmp(psuCurrMRec->szBBDataLinkName,
                               psuCurrPRec->szDataLinkName) == 0))
                    {
                    assert(psuCurrMRec->psuPRecord == NULL);
                    psuCurrMRec->psuPRecord = psuCurrPRec;
                    } // end if name match
                } // end if TMATS version 4 or 5

            else if ((m_iTmatsVersion == 7) ||
                     (m_iTmatsVersion == 9))
                {
                if ((psuCurrMRec->szBBDataLinkName != NULL) &&
                    (psuCurrPRec->szDataLinkName   != NULL) &&
                    (strcasecmp(psuCurrMRec->szBBDataLinkName,
                               psuCurrPRec->szDataLinkName) == 0))
                    {
                    assert(psuCurrMRec->psuPRecord == NULL);
                    psuCurrMRec->psuPRecord = psuCurrPRec;
                    } // end if name match
                } // end if TMATS version 7 or 9

            // Get the next P record
            psuCurrPRec = psuCurrPRec->psuNext;
            } // end while walking the P record list

        // Walk through the B record linked list
        psuCurrBRec = psuTmatsInfo->psuFirstBRecord;
        while (psuCurrBRec != NULL)
            {

                // See if M-x\BB\DLN = B-x\DLN
                if ((psuCurrMRec->szBBDataLinkName != NULL) &&
                    (psuCurrBRec->szDataLinkName   != NULL) &&
                    (strcasecmp(psuCurrMRec->szBBDataLinkName,
                               psuCurrBRec->szDataLinkName) == 0))
                    {
                    // Note, if psuCurrMRecord->psuBRecord != NULL then that 
                    // is probably an error in the TMATS file
                    assert(psuCurrMRec->psuBRecord == NULL);
                    psuCurrMRec->psuBRecord = psuCurrBRec;
                    } // end if match

            // Get the next B record
            psuCurrBRec = psuCurrBRec->psuNextBRecord;
            } // end while walking the B record list

        // Walk through the S record linked list another day


        // Get the next M record
        psuCurrMRec = psuCurrMRec->psuNextMRecord;
        } // end while walking M records

    // Do subchannels some other day!

    return;
    } // end vConnectM()



/* -----------------------------------------------------------------------
 * B Records
 * ----------------------------------------------------------------------- 
 */

// Macros to make decoding B record logic more compact

#define DECODE_B(pattern, field)                                                \
    else if (strcasecmp(szCodeField, #pattern) == 0)                            \
        {                                                                       \
        psuBRec->field = szDataItem;                                            \
        }


int bDecodeBLine(char * szCodeName, char * szDataItem, SuBRecord ** ppsuFirstBRecord)
    {
    char          * szCodeField;
    int             iTokens;
    int             iRIdx;
    SuBRecord     * psuBRec;

    // See which B field it is
    szCodeField = strtok(szCodeName, "\\");
    assert(szCodeField[0] == 'B');

    // Get the B record index number
    iTokens = sscanf(szCodeField, "%*1c-%i", &iRIdx);
    if (iTokens == 1)
        {
        psuBRec = psuGetBRecord(ppsuFirstBRecord, iRIdx, bTRUE);
        assert(psuBRec != NULL);
        }
    else
        return 1;
    
    szCodeField = strtok(NULL, "\\");

    if (bFALSE) {}                              // Keep macro logic happy
    DECODE_B(DLN, szDataLinkName)               // DLN - Data link name

    // NBS\N - Number of buses
    else if (strncasecmp(szCodeField, "NBS",3) == 0)
        {
        szCodeField = strtok(NULL, "\\");

        if (bFALSE) {}                          // Keep macro logic happy
        DECODE_B(N, szNumBuses)                 // NBS\N - Number of buses
        } // end if NBS

    return 0;
    }



/* ----------------------------------------------------------------------- */

SuBRecord * psuGetBRecord(SuBRecord ** ppsuFirstBRecord, int iRIndex, int bMakeNew)
    {
    SuBRecord   ** ppsuCurrBRec = ppsuFirstBRecord;

    // Loop looking for matching index number or end of list
    while (bTRUE)
        {
        // Check for end of list
        if (*ppsuCurrBRec == NULL)
            break;

        // Check for matching index number
        if ((*ppsuCurrBRec)->iRecordNum == iRIndex)
            break;

        // Move on to the next record in the list
        ppsuCurrBRec = &((*ppsuCurrBRec)->psuNextBRecord);
        }

    // If no record found then put a new one on the end of the list
    if ((*ppsuCurrBRec == NULL) && (bMakeNew == bTRUE))
        {
        // Allocate memory for the new record
        *ppsuCurrBRec = (SuBRecord *)TmatsMalloc(sizeof(SuBRecord));
        memset(*ppsuCurrBRec, 0, sizeof(SuBRecord));
        (*ppsuCurrBRec)->iRecordNum = iRIndex;
        }

    return *ppsuCurrBRec;
    }



/* ----------------------------------------------------------------------- */

/*
    Tie the P record asynchronous embedded format field to the definition
    of the embedded stream P record.

    P-x\AEF\DLN-n <---> P-x\DLN

*/

void vConnectP(SuTmatsInfo * psuTmatsInfo)
    {
    SuPRecord           * psuCurrPRec;
    SuPRecord           * psuCurrPEmbedRec;
    SuPAsyncEmbedded    * psuCurrPAEF;

    // Walk the linked list of P records
    psuCurrPRec = psuTmatsInfo->psuFirstPRecord;
    while (psuCurrPRec != NULL)
        {

        // Walk the list of P embedded stream records
        psuCurrPAEF = psuCurrPRec->psuFirstAsyncEmbedded;
        while (psuCurrPAEF != NULL)
            {

            // Walk the linked list of P records to find matching async embedded stream
            psuCurrPEmbedRec = psuTmatsInfo->psuFirstPRecord;
            while (psuCurrPEmbedRec != NULL)
                {

                // See if P-x\AEF\DLN-n = P-x\DLN
                if ((psuCurrPEmbedRec->szDataLinkName != NULL) &&
                    (psuCurrPAEF->szDataLinkName      != NULL) &&
                    (strcasecmp(psuCurrPEmbedRec->szDataLinkName,
                                psuCurrPAEF->szDataLinkName) == 0))
                    {
                    psuCurrPAEF->psuPRecord = psuCurrPEmbedRec;
                    }

                // Get the next embedded P record
                psuCurrPEmbedRec = psuCurrPEmbedRec->psuNext;
                }

            // Get the next P AEF record
            psuCurrPAEF = psuCurrPAEF->psuNext;
            } // end while walking the P AEF record list

        // Get the next P record
        psuCurrPRec = psuCurrPRec->psuNext;
        }

    return;
    } // end vConnectPAsyncEmbedded()



/* -----------------------------------------------------------------------
 * Comments
 * ----------------------------------------------------------------------- 
 */

void StoreComment(char * szComment, SuComment ** ppsuFirstComment)
    {
    SuComment ** ppsuCurrComment = ppsuFirstComment;

    // Walk to the end of the linked list of comments
    while (*ppsuCurrComment != NULL)
        ppsuCurrComment = &(*ppsuCurrComment)->psuNext;

    // Allocate memory for the new comment
    *ppsuCurrComment = (SuComment *)TmatsMalloc(sizeof(SuComment));
    memset(*ppsuCurrComment, 0, sizeof(SuComment));

    // Store the comment
    (*ppsuCurrComment)->szComment = szComment;
    
    return;
    }


// -----------------------------------------------------------------------

// The enI106_Decode_Tmats() procedure malloc()'s a lot of memory.  This
// procedure walks the SuMemBlock list, freeing memory as it goes.

void I106_CALL_DECL 
    enI106_Free_TmatsInfo(SuTmatsInfo    * psuTmatsInfo)
    {
    SuMemBlock     * psuCurrMemBlock;
    SuMemBlock     * psuNextMemBlock;

    if (psuTmatsInfo == NULL)
        return;

    // Walk the linked memory list, freely freeing as we head down the freeway
    psuCurrMemBlock = psuTmatsInfo->psuFirstMemBlock;
    while (psuCurrMemBlock != NULL)
        {
        // Free the memory
        free(psuCurrMemBlock->pvMemBlock);

        // Free the memory block and move to the next one
        psuNextMemBlock = psuCurrMemBlock->psuNextMemBlock;
        free(psuCurrMemBlock);
        psuCurrMemBlock = psuNextMemBlock;
        }

    // Dont' forget to free the TMATS lines array
    if (psuTmatsInfo->pasuTmatsLines != NULL)
        free(psuTmatsInfo->pasuTmatsLines);

    // Initialize the TMATS info data structure
    memset(psuTmatsInfo, 0, sizeof(SuTmatsInfo));

    return;
    }


// -----------------------------------------------------------------------

// Allocate memory but keep track of it for enI106_Free_TmatsInfo() later.

void * TmatsMalloc(size_t iSize)
    {
    void            * pvNewBuff;
    SuMemBlock     ** ppsuCurrMemBlock;

    // Malloc the new memory
    pvNewBuff = malloc(iSize);
    assert(pvNewBuff != NULL);

    // Walk to (and point to) the last linked memory block
    ppsuCurrMemBlock = &m_psuTmatsInfo->psuFirstMemBlock;
    while (*ppsuCurrMemBlock != NULL)
        ppsuCurrMemBlock = &(*ppsuCurrMemBlock)->psuNextMemBlock;
        
    // Populate the memory block struct
    *ppsuCurrMemBlock = (SuMemBlock *)malloc(sizeof(SuMemBlock));
    assert(*ppsuCurrMemBlock != NULL);
    (*ppsuCurrMemBlock)->pvMemBlock      = pvNewBuff;
    (*ppsuCurrMemBlock)->psuNextMemBlock = NULL;

    return pvNewBuff;
    }



/* -----------------------------------------------------------------------
 * Write procedures
 * ----------------------------------------------------------------------- */

I106_CALL_DECL EnI106Status 
    enI106_Encode_Tmats(SuI106Ch10Header * psuHeader,
                        void             * pvBuff,
                        char             * szTMATS)
    {
    // Channel specific data word
    *(uint32_t *)pvBuff = 0;

    // Figure out the total TMATS message length
    psuHeader->ulDataLen = strlen(szTMATS) + 4;

    // Copy TMATS setup info to buffer.  This assumes there is enough
    // space in the buffer to hold the TMATS string.
    strcpy((char *)pvBuff+4, szTMATS);

    // Make the data buffer checksum and update the header
    uAddDataFillerChecksum(psuHeader, (unsigned char *)pvBuff);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

// A little routine to return the first non-white space character in string

char * szFirstNonWhitespace(char * szInString)
    {
    char * szFirstChar = szInString;
    while (isspace(*szFirstChar) && (*szFirstChar != '\0'))
        szFirstChar++;
    return szFirstChar;
    }



/* ----------------------------------------------------------------------- */

// Calculate a "fingerprint" checksum code from TMATS info
// Do not include CSDW!!!

I106_CALL_DECL EnI106Status 
    enI106_Tmats_Signature(SuTmatsLine  * aszLines,     // Array of TMATS lines
                           unsigned long  ulTmatsLines, // Number of TMATS line in array
                           int            iSigVersion,  // Request signature version (0 = default)
                           int            iSigFlags,    // Additional flags
                           uint16_t     * piOpCode,     // Version and flag op code
                           uint32_t     * piSignature)  // TMATS signature
    {
    char                szLine[2048];
    char                szLINE[2048];
    unsigned long       ulLineIdx;
    int                 iCopyIdx;
    char              * szCodeName;
    char              * szDataItem;
    char              * szCode;
    char              * szSection;

    // Check the requested signature version
    if (iSigVersion == 0)
        iSigVersion = TMATS_SIGVER_DEFAULT;

    if (iSigVersion > 1)
        return I106_INVALID_PARAMETER;

    *piSignature = 0;

    for (ulLineIdx = 0; ulLineIdx < ulTmatsLines; ulLineIdx++)
        {

        // Make an upper case copy
        strcpy(szLine, aszLines[ulLineIdx].szCodeName);
        strcat(szLine, ":");
        strcat(szLine, aszLines[ulLineIdx].szDataItem);
        strcat(szLine, ";");

        // Convert to upper case
        iCopyIdx = 0;
        while (bTRUE)
            {
            if (islower(szLine[iCopyIdx])) szLINE[iCopyIdx] = toupper(szLine[iCopyIdx]);
            else                           szLINE[iCopyIdx] = szLine[iCopyIdx];
            if (szLine[iCopyIdx] == '\0')
                break;
            iCopyIdx++;
            }
        
        szCodeName = strtok(szLINE, ":");
        szDataItem = strtok(NULL,   ";");

        // If not code name then break out
        if ((szCodeName == NULL) || (szDataItem == NULL))
            continue;

        // Test for COMMENT field
        if (((iSigFlags & TMATS_SIGFLAG_INC_COMMENT) != TMATS_SIGFLAG_INC_COMMENT) &&
            ((iSigFlags & TMATS_SIGFLAG_INC_ALL    ) != TMATS_SIGFLAG_INC_ALL    ) && 
            (strcmp(szCodeName, "COMMENT") == 0))
            continue;

        szSection = strtok(szCodeName, "\\");
        szCode    = strtok(NULL, ":");

        // Comment fields
        if (((iSigFlags & TMATS_SIGFLAG_INC_COMMENT) != TMATS_SIGFLAG_INC_COMMENT) &&
            ((iSigFlags & TMATS_SIGFLAG_INC_ALL    ) != TMATS_SIGFLAG_INC_ALL    ) && 
            (strcmp(szCode, "COM") == 0))
            continue;

        // Ignored R sections
        if (((iSigFlags & TMATS_SIGFLAG_INC_ALL) != TMATS_SIGFLAG_INC_ALL) && 
            (szSection[0] == 'R'))
            {
            if ((strcmp (szCode, "RI1"      ) == 0) ||
                (strcmp (szCode, "RI2"      ) == 0) ||
                (strcmp (szCode, "RI3"      ) == 0) ||
                (strcmp (szCode, "RI4"      ) == 0) ||
                (strcmp (szCode, "RI5"      ) == 0) ||
                (strcmp (szCode, "RI6"      ) == 0) ||
                (strcmp (szCode, "RI7"      ) == 0) ||
                (strcmp (szCode, "RI8"      ) == 0) ||
                (strcmp (szCode, "RI9"      ) == 0) ||
                (strcmp (szCode, "RI10"     ) == 0) ||
                (strcmp (szCode, "DPOC1"    ) == 0) ||
                (strcmp (szCode, "DPOC2"    ) == 0) ||
                (strcmp (szCode, "DPOC3"    ) == 0) ||
                (strcmp (szCode, "MPOC4"    ) == 0) ||
                (strcmp (szCode, "MPOC1"    ) == 0) ||
                (strcmp (szCode, "MPOC2"    ) == 0) ||
                (strcmp (szCode, "MPOC3"    ) == 0) ||
                (strcmp (szCode, "MPOC4"    ) == 0) ||
                (strcmp (szCode, "RIM\\N"   ) == 0) ||
                (strncmp(szCode, "RIMI-",  5) == 0) ||
                (strncmp(szCode, "RIMS-",  5) == 0) ||
                (strncmp(szCode, "RIMF-",  5) == 0) ||
                (strcmp (szCode, "RMM\\N"   ) == 0) ||
                (strncmp(szCode, "RMMID-", 6) == 0) ||
                (strncmp(szCode, "RMMS-",  5) == 0) ||
                (strncmp(szCode, "RMMF-",  5) == 0))
                continue;
            }

        // Vendor fields
        if (((iSigFlags & TMATS_SIGFLAG_INC_VENDOR) != TMATS_SIGFLAG_INC_VENDOR) && 
            ((iSigFlags & TMATS_SIGFLAG_INC_ALL   ) != TMATS_SIGFLAG_INC_ALL   ) && 
            (szSection[0] == 'V'))
            continue;
                
        // G fields
        if (((iSigFlags & TMATS_SIGFLAG_INC_G     ) != TMATS_SIGFLAG_INC_G     ) && 
            ((iSigFlags & TMATS_SIGFLAG_INC_ALL   ) != TMATS_SIGFLAG_INC_ALL   ) && 
            (szSection[0] == 'G'))
            continue;

        // Make another upper case copy
        iCopyIdx = 0;
        while (bTRUE)
            {
            if (islower(szLine[iCopyIdx]))
                szLINE[iCopyIdx] = toupper(szLine[iCopyIdx]);
            else
                szLINE[iCopyIdx] = szLine[iCopyIdx];
            if (szLine[iCopyIdx] == '\0')
                break;
            iCopyIdx++;
            }

        *piSignature += Fletcher32((uint8_t *)szLINE, strlen(szLINE));

        } // end while reading chars from the buffer
    
    // Everything seems OK so make the op code
    *piOpCode = ((iSigFlags & 0x000F) << 4) | (iSigVersion & 0x000F);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

// http://en.wikipedia.org/wiki/Fletcher%27s_checksum

uint32_t Fletcher32(uint8_t * data, int count)
    {
    uint32_t    addend;
    uint32_t    sum1 = 0;
    uint32_t    sum2 = 0;
    int         index;
 
    index = 0;
    while (index < count)
        {
        addend  = data[index++];
        if (index < count)
            addend |= data[index++] << 8;
        sum1 = (sum1 + addend) % 0xffff;
        sum2 = (sum2 +   sum1) % 0xffff;
        }
 
    return (sum2 << 16) | sum1;
    }


/* ----------------------------------------------------------------------- */

#ifdef SHA256ENABLE
I106_CALL_DECL EnI106Status 
    enI106_Tmats_IRIG_Signature(void         * pvBuff,      // TMATS text without CSDW
                                uint32_t       ulDataLen,   // Length of TMATS in pvBuff
                                uint8_t        auHash[])    // 32 byte array for SHA-256
    {
    uint32_t        ulInBuffIdx;
    char          * achInBuff;
    int             bShaStartFound;
    int             bShaEndFound;
    unsigned long   ulShaStartIdx;  // Index of the G in G\SHA
    unsigned long   ulShaEndIdx;    // Index of the ;
    SHA256_CTX      suShaContext;

    achInBuff   = (char *)pvBuff;

    // Search for any G\SHA code
    bShaStartFound   = bFALSE;
    bShaEndFound     = bFALSE;
    for (ulInBuffIdx = 0; ulInBuffIdx < ulDataLen; ulInBuffIdx++)
        {
        // Only search for G\SHA if there are enough characters left
        if ((bShaStartFound == bFALSE) && (ulInBuffIdx <= ulDataLen-5))
            if (strncasecmp(&achInBuff[ulInBuffIdx], "G\\SHA", 5) == 0)
                {
                bShaStartFound = bTRUE;
                ulShaStartIdx  = ulInBuffIdx;
                continue;
                } // end if G\SHA found

        // G\SHA found so look for the terminating ";"
        if ((bShaStartFound == bTRUE) && (bShaEndFound == bFALSE))
            if (achInBuff[ulInBuffIdx] == ';')
                {
                bShaEndFound = bTRUE;
                ulShaEndIdx  = ulInBuffIdx;
                break;
                } // end if ';' found
        } // end for all characters in the buffer

    // Now calculate the SHA-256
    sha256_init(&suShaContext);

    // G\SHA is at the very beginning
    if (bShaStartFound && (ulShaStartIdx == 0))
    	sha256_update(&suShaContext, (BYTE *)&achInBuff[ulShaEndIdx+1], ulDataLen - ulShaEndIdx - 1);

    // G\SHA is at the very end
    else if (bShaEndFound && (ulShaEndIdx == ulDataLen-1))
    	sha256_update(&suShaContext, (BYTE *)&achInBuff[0], ulDataLen - ulShaStartIdx);

    // G\SHA is somewhere in the middle
    else if (bShaStartFound && bShaEndFound)
        {
    	sha256_update(&suShaContext, (BYTE *)&achInBuff[0], ulShaStartIdx);
    	sha256_update(&suShaContext, (BYTE *)&achInBuff[ulShaEndIdx+1], ulDataLen - ulShaEndIdx - 1);
        }

    // G\SHA must not have been found
    else
    	sha256_update(&suShaContext, (BYTE *)&achInBuff[0], ulDataLen);

    // Get the final hash
	sha256_final(&suShaContext, auHash);

    return I106_OK;
    }
#endif


#ifdef __cplusplus
} // end namespace i106
#endif
