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
#include "i106_stdint.h"

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

static SuTmatsInfo      * m_psuTmatsInfo;
static int                m_iTmatsVersion = 0;

/*
 * Function Declaration
 * --------------------
 */

void TmatsBufferToLines(void * pvBuff, uint32_t ulDataLen, SuTmatsInfo * psuTmatsInfo);

void vConnectG(SuTmatsInfo * psuTmatsInfo);
void vConnectR(SuTmatsInfo * psuTmatsInfo);
void vConnectM(SuTmatsInfo * psuTmatsInfo);
void vConnectP(SuTmatsInfo * psuTmatsInfo);
void vConnectB(SuTmatsInfo * psuTmatsInfo);
void vConnectD(SuTmatsInfo * psuTMatsInfo);

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
    int                 iCodeNameLength;
    char              * szCodeName;
    char              * szDataItem;

    // Store a copy for module wide use
    m_psuTmatsInfo = psuTmatsInfo;

    // Initialize the TMATS info data structure
    enI106_Free_TmatsInfo(psuTmatsInfo);

    // Read the buffer into an array of lines
    TmatsBufferToLines(pvBuff, ulDataLen, psuTmatsInfo);

    // Initialize the first (and only, for now) G record
    psuTmatsInfo->psuFirstGRecord = (SuGRecord *)TmatsMalloc(sizeof(SuGRecord));
    memset(psuTmatsInfo->psuFirstGRecord, 0, sizeof(SuGRecord));

    // Initialize the local code name storage
    iCodeNameLength = 2049;
    szCodeName      = (char *)malloc(iCodeNameLength);

    // Step through the array of TMATS lines
    iLineIdx = 0;
    while (iLineIdx < psuTmatsInfo->ulTmatsLines)
        {
        // Check if local line storages needs to be expanded
        if (strlen(psuTmatsInfo->pasuTmatsLines[iLineIdx].szCodeName) > (iCodeNameLength - 1))
            {
            iCodeNameLength += 100;
            szCodeName       = (char *)realloc(szCodeName, iCodeNameLength);
            }

        // Code Name gets parsed some more so make a copy of it
        strcpy(szCodeName, psuTmatsInfo->pasuTmatsLines[iLineIdx].szCodeName);

        szDataItem = psuTmatsInfo->pasuTmatsLines[iLineIdx].szDataItem;

        // Decode comments
        if (strcasecmp(szCodeName, "COMMENT") == 0)
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
                    bDecodeGLine(szCodeName,
                                               szDataItem, 
                                               &psuTmatsInfo->psuFirstGRecord);
                    break;

                case 'B' : // Bus Data Attributes
                    bDecodeBLine(szCodeName, 
                                               szDataItem,
                                               &psuTmatsInfo->psuFirstBRecord);
                    break;

                case 'R' : // Tape/Storage Source Attributes
                    bDecodeRLine(szCodeName, 
                                               szDataItem,
                                               &psuTmatsInfo->psuFirstRRecord);
                    break;

                case 'T' : // Transmission Attributes
                    break;

                case 'M' : // Multiplexing/Modulation Attributes
                    bDecodeMLine(szCodeName, 
                                               szDataItem,
                                               &psuTmatsInfo->psuFirstMRecord);
                    break;

                case 'P' : // PCM Format Attributes
                    bDecodePLine(szCodeName, 
                                               szDataItem,
                                               &psuTmatsInfo->psuFirstPRecord);
                    break;

                case 'D' : // PCM Measurement Description
                    bDecodeDLine(szCodeName, 
                                               szDataItem,
                                               &psuTmatsInfo->psuFirstDRecord);
                    break;

                case 'S' : // Packet Format Attributes
                    break;

                case 'A' : // PAM Attributes
                    break;

                case 'C' : // Data Conversion Attributes
                    bDecodeCLine(szCodeName, 
                                               szDataItem,
                                               &psuTmatsInfo->psuFirstCRecord);
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
    
    From IRIG 106-17

                G\DSI-n          -> R-x\ID, T-x\ID, M-x\ID, V-x\ID
                T-x\ID           -> M-x\ID
                R-x\EPL\LSNM-n   -> R-x\CGNM-n
                R-x\EPL\LSSN-n   -> R-x\CGSN-n
                R-x\EPL\LDEIP-n  -> R-x\EIIP-n
                R-x\EPL\LDEPA-n  -> R-x\EI\PA
                R-x\EV\DLN-n     -> P-d\DLN, B-x\DLN, S-d\DLN
                R-x\EV\PM\MN-n-m -> B-x\MN-i-n-p, D-x\MN-y-n, S-d\MN-i-n-p
                R-x\DSI-n        -> M-x\ID
                R-x\CDLN-n       -> P-d\DLN, B-x\DLN, S-d\DLN
                R-x\SMF\SMN-n-m  -> D-x\MN-y-n
                R-x\BME\SMN-n-m  -> B-x\MN-i-n-p
                R-x\AMN-n-m      -> C-d\DCN
                R-x\DMN-n-m      -> C-d\DCN
                R-x\OSNM-n       -> R-x\CGNM-n
                M-x\BB\DLN       -> P-d\DLN
                M-x\BB\MN        -> C-d\DCN
                M-x\SI\DLN-n     -> P-d\DLN
                M-x\SI\MN-n      -> C-d\DCN
                P-d\DLN          -> D-x\DLN, B-d\DLN
                P-d\AEF\DLN-n    -> P-d\DLN
                P-d\MLC2-n       -> D-x\MLN-y
                P-d\FSC2-n       -> P-d\DLN
                P-d\ADM\DMN-n    -> P-d\DLN
                D-x\MN-y-n       -> C-d\DCN
                D-x\REL1-y-n-m   -> D-x\MN-y-n
                B-x\UMN1-i       -> C-d\DCN
                B-x\UMN2-i       -> C-d\DCN
                B-x\UMN3-i       -> C-d\DCN
                B-x\MN-i-n-p     -> C-d\DCN
                S-d\MN-i-n-p     -> C-d\DCN
                C-d\DPTM         -> C-d\DCN
                C-d\DP-n         -> C-d\DCN

    From the TMATS Handbook

        3.1g    G\DSI-x    --> T-x\ID
                G\DSI-x    --> M-x\ID
        3.2a    G\DSI-x    --> T-x\ID
        3.3a    R-x\ID     --> M-x\ID (for recorders)
                T-x\ID     --> M-x\ID (for transmitters)
        3.3c    M-x\BB\DLN --> P-x\DLN (for PAM and PCM)
        3.4a    M-x\BB\DLN --> P-x\DLN (for PCM baseband signal)
                M-x\SI\DLN --> P-x\DLN (for IRIG subcarrier)
                P-x\DLN    --> D-x\DLN
        3.5a    P-x\DLN    --> D-x\DLN
        3.5e    D-x\MN-y-n --> C-x\DCN
        3.6a    D-x\MN-y-n --> C-x\DCN
    */

    /* Connect M first because some of the fields are going to be connected
       conditionally. There is quite a bit of confusion (at least on my part)
       about how a lot of the linkage happens through the M records. If there
       is an M record linked back to an R record and to a P record then I am
       not going to link the R-CDLN-n
        R-x\DSI-n <---> M-x\ID
                        M-x\BB\DLN    <-+-> P-x\DLN  \
                                        +-> B-x\DLN   - With M-x Baseband
                                        +-> S-x\DLN  /

                        M-x\SI\DLN-n  <-+-> P-x\DLN  \
                                        +-> B-x\DLN   - With M-x Subchannels
                                        +-> S-x\DLN  /

        R-x\CDLN-n <--------------------+-> P-x\DLN  \
                                        +-> B-x\DLN   - Without M-x
                                        +-> S-x\DLN  /
    */
    vConnectM(psuTmatsInfo);

    vConnectG(psuTmatsInfo);
    vConnectR(psuTmatsInfo);
    vConnectP(psuTmatsInfo);
    vConnectB(psuTmatsInfo);
    vConnectD(psuTmatsInfo);

    m_psuTmatsInfo = NULL;
    free(szCodeName);

    return I106_OK;
    }



// ------------------------------------------------------------------------

char * enI106_Tmats_Find(SuTmatsInfo * psuTmatsInfo, const char * szTmatsCode)
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
    char        * szLine;
    int           iLineLength;
    char        * achInBuff;
    int           iLineIdx;
    char        * szCodeName;
    char        * szDataItem;

    // Init buffer pointers
    iLineLength  = 2029;
    szLine       = (char *)malloc(iLineLength);
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
                // Check if local line storages needs to be expanded
                if (iLineIdx >= (iLineLength - 1))
                    {
                    iLineLength += 100;
                    szLine       = (char *)realloc(szLine, iLineLength);
                    }

                // Local storage is big enough so copy the next character
                szLine[iLineIdx] = achInBuff[iInBuffIdx];
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

#define LINK_NAMES_MATCH(LinkName1, LinkName2)      \
    ((LinkName1 != NULL) &&                         \
     (LinkName2 != NULL) &&                         \
     (strcasecmp(LinkName1, LinkName2) == 0))


/*
    Tie the G record data sources to their underlying R, T, and M 
    records.

    For recorder case...
    G/DSI-n <---> R-x\ID

    For telemetry case...
    G/DSI-n <-+-> T-x\ID
              +-> M-x\ID

    Here is more info from IRIG 106-17
        G\DSI-n          -> R-x\ID, T-x\ID, M-x\ID, V-x\ID

*/

void vConnectG(SuTmatsInfo * psuTmatsInfo)
    {
    SuRRecord       * psuCurrRRec;
    SuMRecord       * psuCurrMRec;
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
            if (LINK_NAMES_MATCH(psuCurrGDataSrc->szDataSourceID, psuCurrRRec->szDataSourceID))
                {
                // Note, if psuCurrGDataSrc->psuRRecord != NULL then that 
                // is probably an error in the TMATS file
                assert(psuCurrGDataSrc->psuRRecord == NULL);
                psuCurrGDataSrc->psuRRecord = psuCurrRRec;
                } // end if match

            // Get the next R record
            psuCurrRRec = psuCurrRRec->psuNext;
            } // end while walking the R record list

        // Walk through the M records linked list looking for a match
        psuCurrMRec = psuTmatsInfo->psuFirstMRecord;
        while (psuCurrMRec != NULL)
            {
            // See if G/DSI-n = M-x\ID
            if (LINK_NAMES_MATCH(psuCurrGDataSrc->szDataSourceID, psuCurrMRec->szDataSourceID))
                {
                // Note, if psuCurrGDataSrc->psuMRecord != NULL then that 
                // is probably an error in the TMATS file
                assert(psuCurrGDataSrc->psuMRecord == NULL);
                psuCurrGDataSrc->psuMRecord = psuCurrMRec;
                } // end if match

            // Get the next M record
            psuCurrMRec = psuCurrMRec->psuNext;
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

    106-07 and beyond...
    R-x\CDLN-n <-------------------+-> P-x\DLN  \
                                   +-> B-x\DLN   - Without M-x
                                   +-> S-x\DLN  /

    Here is more info from IRIG 106-17
        R-x\CDLN-n       -> P-d\DLN, B-x\DLN, S-d\DLN
        R-x\EPL\LSNM-n   -> R-x\CGNM-n
        R-x\EPL\LSSN-n   -> R-x\CGSN-n
        R-x\EPL\LDEIP-n  -> R-x\EIIP-n
        R-x\EPL\LDEPA-n  -> R-x\EI\PA
        R-x\EV\DLN-n     -> P-d\DLN, B-x\DLN, S-d\DLN
        R-x\EV\PM\MN-n-m -> B-x\MN-i-n-p, D-x\MN-y-n, S-d\MN-i-n-p
        R-x\DSI-n        -> M-x\ID
        R-x\SMF\SMN-n-m  -> D-x\MN-y-n
        R-x\BME\SMN-n-m  -> B-x\MN-i-n-p
        R-x\AMN-n-m      -> C-d\DCN
        R-x\DMN-n-m      -> C-d\DCN
        R-x\OSNM-n       -> R-x\CGNM-n

*/

void vConnectR(SuTmatsInfo * psuTmatsInfo)
    {
    SuRRecord       * psuCurrRRec;
    SuRDataSource   * psuCurrRDataSrc;
    SuMRecord       * psuCurrMRec;
    SuPRecord       * psuCurrPRec;
    SuBRecord       * psuCurrBRec;

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
                if (LINK_NAMES_MATCH(psuCurrRDataSrc->szDataSourceID, psuCurrMRec->szDataSourceID))
                    {
                    // Note, if psuCurrRDataSrc->psuMRecord != NULL then that 
                    // is probably an error in the TMATS file
                    assert(psuCurrRDataSrc->psuMRecord == NULL);
                    psuCurrRDataSrc->psuMRecord = psuCurrMRec;
                    }

                // Get the next M record
                psuCurrMRec = psuCurrMRec->psuNext;
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
                    if (LINK_NAMES_MATCH(psuCurrRDataSrc->szPcmDataLinkName, psuCurrPRec->szDataLinkName))
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
                    if (LINK_NAMES_MATCH(psuCurrRDataSrc->szChanDataLinkName, psuCurrPRec->szDataLinkName))
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

            // Walk through the B records linked list
            psuCurrBRec = psuTmatsInfo->psuFirstBRecord;
            while (psuCurrBRec != NULL)
                {
                // R to B tieing changed with the -07 release.  Try to do it the
                // "right" way first, but accept the "wrong" way if that doesn't work.
                // TMATS 04 and 05
                if ((m_iTmatsVersion == 4) ||
                    (m_iTmatsVersion == 5))
                    {
                    // See if R-x\BDLN-n = B-x\DLN, aka the "right" way
                    if (LINK_NAMES_MATCH(psuCurrRDataSrc->szBusDataLinkName, psuCurrBRec->szDataLinkName))
                        {
                        // Note, if psuCurrRDataSrc->psuBRecord != NULL then that 
                        // is probably an error in the TMATS file
                        assert(psuCurrRDataSrc->psuBRecord == NULL);
                        psuCurrRDataSrc->psuBRecord = psuCurrBRec;
                        }

                    // Try some "wrong" ways
                    } // end if TMATS 04 or 05

                // TMATS 07, 09, and beyond (I hope)
                else
                    {
                    // See if R-x\CDLN-n = B-x\DLN, aka the "right" way
                    if (LINK_NAMES_MATCH(psuCurrRDataSrc->szChanDataLinkName, psuCurrBRec->szDataLinkName))
                        {
                        // Note, if psuCurrRDataSrc->psuBRecord != NULL then that 
                        // is probably an error in the TMATS file
                        assert(psuCurrRDataSrc->psuBRecord == NULL);
                        psuCurrRDataSrc->psuBRecord = psuCurrBRec;
                        }

                    // Try some "wrong" ways
                    } // end if TMATS 07 or 09 (or beyond)

                // Get the next B record
                psuCurrBRec = psuCurrBRec->psuNext;
                } // end while walking the P record list

            // Walk the C record link lists


            // Walk the S record link lists


            // Get the next R data source record
            psuCurrRDataSrc = psuCurrRDataSrc->psuNext;
            } // end while walking the R data source records

        // Get the next R record
        psuCurrRRec = psuCurrRRec->psuNext;
        }

    return;
    } // end vConnectR()


// ----------------------------------------------------------------------------

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

    Here is more info from IRIG 106-17

        M-x\BB\DLN       -> P-d\DLN
        M-x\BB\MN        -> C-d\DCN
        M-x\SI\DLN-n     -> P-d\DLN
        M-x\SI\MN-n      -> C-d\DCN

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
            // See if M-x\BB\DLN = P-x\DLN
            if (LINK_NAMES_MATCH(psuCurrMRec->szBBDataLinkName, psuCurrPRec->szDataLinkName))
                {
                // Note, if psuCurrRRecord->psuPRecord != NULL then that 
                // is probably an error in the TMATS file
                assert(psuCurrMRec->psuPRecord == NULL);
                psuCurrMRec->psuPRecord = psuCurrPRec;
                } // end if name match

            // Get the next P record
            psuCurrPRec = psuCurrPRec->psuNext;
            } // end while walking the P record list

        // Walk through the B record linked list
        psuCurrBRec = psuTmatsInfo->psuFirstBRecord;
        while (psuCurrBRec != NULL)
            {

            // See if M-x\BB\DLN = B-x\DLN
            if (LINK_NAMES_MATCH(psuCurrMRec->szBBDataLinkName, psuCurrBRec->szDataLinkName))
                {
                // Note, if psuCurrMRecord->psuBRecord != NULL then that 
                // is probably an error in the TMATS file
                assert(psuCurrMRec->psuBRecord == NULL);
                psuCurrMRec->psuBRecord = psuCurrBRec;
                } // end if match

            // Get the next B record
            psuCurrBRec = psuCurrBRec->psuNext;
            } // end while walking the B record list

        // Walk through the S record linked list another day


        // Get the next M record
        psuCurrMRec = psuCurrMRec->psuNext;
        } // end while walking M records

    // Do subchannels some other day!

    return;
    } // end vConnectM()



/* -----------------------------------------------------------------------
 * B Records
 * ----------------------------------------------------------------------- 

    B-x\MN-i-n-p     -> C-d\DCN
    B-x\UMN1-i       -> C-d\DCN
    B-x\UMN2-i       -> C-d\DCN
    B-x\UMN3-i       -> C-d\DCN

 */

void vConnectB(SuTmatsInfo * psuTmatsInfo)
    {
    SuBRecord           * psuCurrBRec;
    SuBBusInfo          * psuCurrBBusInfo;
    SuBMsgContentDef    * psuCurrBMsgContentDef;
    SuBMeasurand        * psuCurrBMeasurand;
    SuCRecord           * psuCurrCRec;

    // Walk the linked list of B records
    psuCurrBRec = psuTmatsInfo->psuFirstBRecord;
    while (psuCurrBRec != NULL)
        {

        // Walk the linked list of bus info
        psuCurrBBusInfo = psuCurrBRec->psuFirstBBusInfo;
        while (psuCurrBBusInfo != NULL)
            {

            // Walk the list of message content definitions
            psuCurrBMsgContentDef = psuCurrBBusInfo->psuFirstMsgContentDef;
            while (psuCurrBMsgContentDef != NULL)
                {

                // Walk the list of measurand description sets
                psuCurrBMeasurand = psuCurrBMsgContentDef->psuFirstMeasurand;
                while (psuCurrBMeasurand != NULL)
                    {

                    // Walk the list of data conversion names, looking for a match
                    psuCurrCRec = psuTmatsInfo->psuFirstCRecord;
                    while (psuCurrCRec != NULL)
                        {
                        // See if B-x\MN-i-n-p = C-x\DCN
                        if (LINK_NAMES_MATCH(psuCurrBMeasurand->szName, psuCurrCRec->szMeasurementName))
                            {
                            psuCurrBMeasurand->psuCRec = psuCurrCRec;
                            } // end if names match

                        // Get the next C record
                        psuCurrCRec = psuCurrCRec->psuNext;

                        } // end for all data conversion records

                    // Get the next measureand description set
                    psuCurrBMeasurand = psuCurrBMeasurand->psuNext;

                    } // end for all measurand description sets

                // Get the next message content definition
                psuCurrBMsgContentDef = psuCurrBMsgContentDef->psuNext;

                } // end for all message content definitions

            // Get the next bus info
            psuCurrBBusInfo = psuCurrBBusInfo->psuNext;

            } // end for all bus info

        // Get the next B record
        psuCurrBRec = psuCurrBRec->psuNext;
        }

    return;
    } // end vConnectB()

/* ----------------------------------------------------------------------- */

/*
    Tie the P record asynchronous embedded format field to the definition
    of the embedded stream P record.

      * P-x\AEF\DLN-n <---> P-x\DLN

    Here is more info from IRIG 106-17
      * P-d\DLN          -> D-x\DLN, B-d\DLN
      * P-d\AEF\DLN-n    -> P-d\DLN
        P-d\MLC2-n       -> D-x\MLN-y
        P-d\FSC2-n       -> P-d\DLN
        P-d\ADM\DMN-n    -> P-d\DLN

*/

void vConnectP(SuTmatsInfo * psuTmatsInfo)
    {
    SuPRecord           * psuCurrPRec;
    SuPRecord           * psuCurrPEmbedRec;
    SuPAsyncEmbedded    * psuCurrPAEF;
    SuDRecord           * psuCurrDRec;
    SuBRecord           * psuCurrBRec;

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
                if (LINK_NAMES_MATCH(psuCurrPEmbedRec->szDataLinkName, psuCurrPAEF->szDataLinkName))
                    {
                    psuCurrPAEF->psuPRecord = psuCurrPEmbedRec;
                    }

                // Get the next embedded P record
                psuCurrPEmbedRec = psuCurrPEmbedRec->psuNext;
                }

            // Get the next P AEF record
            psuCurrPAEF = psuCurrPAEF->psuNext;
            } // end while walking the P AEF record list

        // Walk the list of D records
        psuCurrDRec = psuTmatsInfo->psuFirstDRecord;
        while (psuCurrDRec != NULL)
            {

            // See if P-x\DLN = D-x\DLN
            if (LINK_NAMES_MATCH(psuCurrPRec->szDataLinkName, psuCurrDRec->szDataLinkName))
                {
                psuCurrPRec->psuDRecord = psuCurrDRec;
                }

            psuCurrDRec = psuCurrDRec->psuNext;
            } // end while walking D record list


        // Walk the list of B records
        psuCurrBRec = psuTmatsInfo->psuFirstBRecord;
        while (psuCurrBRec != NULL)
            {

            // See if P-x\DLN = B-x\DLN (IS THIS REALLY TRUE???)
            if (LINK_NAMES_MATCH(psuCurrPRec->szDataLinkName, psuCurrBRec->szDataLinkName))
                {
                psuCurrPRec->psuBRecord = psuCurrBRec;
                }

            // Get the next B record
            psuCurrBRec = psuCurrBRec->psuNext;
            } // end while walking the B record list

        // Get the next P record
        psuCurrPRec = psuCurrPRec->psuNext;
        }

    return;
    } // end vConnectPAsyncEmbedded()


// -----------------------------------------------------------------------

/*
    The PCM Measurement Description Group (D) connects to the Data
    Conversion Group (C).

      * D-x\MN-y-n       -> C-d\DCN
        D-x\REL1-y-n-m   -> D-x\MN-y-n
*/

void vConnectD(SuTmatsInfo * psuTmatsInfo)
    {
    SuDRecord           * psuCurrDRec;
    SuDMeasurementList  * psuCurrDMeasList;
    SuDMeasurand        * psuCurrDMeasurand;
    SuCRecord           * psuCurrCRec;

    // Walk the list of D records
    for (psuCurrDRec  = psuTmatsInfo->psuFirstDRecord;
         psuCurrDRec != NULL;
         psuCurrDRec  = psuCurrDRec->psuNext)
        {

        // Walk the list of measurement lists
        for (psuCurrDMeasList = psuCurrDRec->psuFirstMeasurementList;
             psuCurrDMeasList != NULL;
             psuCurrDMeasList = psuCurrDMeasList->psuNext)
            {

            // Walk the list of measurands
            for (psuCurrDMeasurand  = psuCurrDMeasList->psuFirstMeasurand;
                 psuCurrDMeasurand != NULL;
                 psuCurrDMeasurand  = psuCurrDMeasurand->psuNext)
                {

                // Walk the list of C records
                for (psuCurrCRec  = psuTmatsInfo->psuFirstCRecord;
                     psuCurrCRec != NULL;
                     psuCurrCRec  = psuCurrCRec->psuNext)
                    {
                    // See if D-x\MN-y-n = C-d\DCN
                    if (LINK_NAMES_MATCH(psuCurrDMeasurand->szName, psuCurrCRec->szMeasurementName))
                        {
                        psuCurrDMeasurand->psuCRec = psuCurrCRec;
                        }

                    } // end for each C record

                // Walk the list of relative measurands... someday.

                } // end for each D measurand

            } // end for each D measurement list

        } // end for each D record

    }

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
    SuMemBlock      * psuNewMemBlock;

    // Malloc the new memory
    pvNewBuff = malloc(iSize);
    assert(pvNewBuff != NULL);

    // Populate the memory block struct
    psuNewMemBlock = (SuMemBlock *)malloc(sizeof(SuMemBlock));
    assert(psuNewMemBlock != NULL);
    psuNewMemBlock->pvMemBlock       = pvNewBuff;
    psuNewMemBlock->psuNextMemBlock  = m_psuTmatsInfo->psuFirstMemBlock;
    m_psuTmatsInfo->psuFirstMemBlock = psuNewMemBlock;

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
    psuHeader->ulDataLen = (uint32_t)strlen(szTMATS) + 4;

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
    int                 iLineLength;
    char              * szLine;
    char              * szLINE;
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

    // Malloc some memory for the TMATS line
    iLineLength = 2050;
    szLine      = (char *)malloc(iLineLength);
    szLINE      = (char *)malloc(iLineLength);

    for (ulLineIdx = 0; ulLineIdx < ulTmatsLines; ulLineIdx++)
        {

        // Make an upper case copy
        int iCurrLineLength = strlen(aszLines[ulLineIdx].szCodeName) + strlen(aszLines[ulLineIdx].szDataItem) + 3;
        if (iCurrLineLength > iLineLength)
            {
            iLineLength = iCurrLineLength + 1000;
            szLine = (char *)realloc(szLine, iLineLength);
            szLINE = (char *)realloc(szLINE, iLineLength);
            }
        strcpy(szLine, aszLines[ulLineIdx].szCodeName);
        strcat(szLine, ":");
        strcat(szLine, aszLines[ulLineIdx].szDataItem);
        strcat(szLine, ";");

        // Convert to upper case
        iCopyIdx = 0;
        while (bTRUE)
            {
            if (islower((unsigned char)(szLine[iCopyIdx]))) szLINE[iCopyIdx] = toupper(szLine[iCopyIdx]);
            else                                            szLINE[iCopyIdx] =         szLine[iCopyIdx];
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

        // If the szCode is null then TMATS line is malformed. The best we can do is bail out of this line.
        if (szCode == NULL)
            continue;

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
            if (islower((unsigned char)(szLine[iCopyIdx])))
                szLINE[iCopyIdx] = toupper(szLine[iCopyIdx]);
            else
                szLINE[iCopyIdx] = szLine[iCopyIdx];
            if (szLine[iCopyIdx] == '\0')
                break;
            iCopyIdx++;
            }

        *piSignature += Fletcher32((uint8_t *)szLINE, (int)strlen(szLINE));

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
