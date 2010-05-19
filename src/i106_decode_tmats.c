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
char                    m_szEmpty[] = "";

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

int bDecodeGLine(char * szCodeName, char * szDataItem, SuGRecord ** ppsuFirstGRec);
int bDecodeRLine(char * szCodeName, char * szDataItem, SuRRecord ** ppsuFirstRRec);
int bDecodeMLine(char * szCodeName, char * szDataItem, SuMRecord ** ppsuFirstMRec);
int bDecodeBLine(char * szCodeName, char * szDataItem, SuBRecord ** ppsuFirstBRec);
int bDecodePLine(char * szCodeName, char * szDataItem, SuPRecord ** ppsuFirstPRec);

SuRRecord * psuGetRRecord(SuRRecord ** ppsuFirstRRec, int iRIndex, int bMakeNew);
SuMRecord * psuGetMRecord(SuMRecord ** ppsuFirstMRec, int iRIndex, int bMakeNew);
SuBRecord * psuGetBRecord(SuBRecord ** ppsuFirstBRec, int iRIndex, int bMakeNew);
SuPRecord * psuGetPRecord(SuPRecord ** ppsuFirstPRec, int iRIndex, int bMakeNew);

SuGDataSource * psuGetGDataSource(SuGRecord * psuGRec, int iDSIIndex, int bMakeNew);
SuRDataSource * psuGetRDataSource(SuRRecord * psuRRec, int iDSIIndex, int bMakeNew);

void vConnectG(SuTmatsInfo * psuTmatsInfo);
void vConnectR(SuTmatsInfo * psuTmatsInfo);
void vConnectM(SuTmatsInfo * psuTmatsInfo);

/*
void vConnectRtoG(SuGRecord * psuFirstGRecord, SuRRecord * psuFirstRRecord);
void vConnectMtoR(SuRRecord * psuFirstRRecord, SuMRecord * psuFirstMRecord);
void vConnectBtoM(SuMRecord * psuFirstMRecord, SuBRecord * psuFirstBRecord);
void vConnectPtoM(SuMRecord * psuFirstMRecord, SuPRecord * psuFirstPRecord);
*/

void * TmatsMalloc(size_t iSize);

char * szFirstNonWhitespace(char * szInString);

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

    // Decode any available info from channel specific data word
    switch (psuHeader->ubyHdrVer)
        {
        case 0x03 :  // 106-07, 106-09
            psuTmats_ChanSpec = (SuTmats_ChanSpec *)pvBuff;
            psuTmatsInfo->iCh10Ver      = psuTmats_ChanSpec->iCh10Ver;
            psuTmatsInfo->bConfigChange = psuTmats_ChanSpec->bConfigChange;
            break;
        default :
            psuTmatsInfo->iCh10Ver      = 0x00;
            psuTmatsInfo->bConfigChange = 0x00;
            break;
        }

    pvTmatsText = (char *)pvBuff + sizeof(SuTmats_ChanSpec);
    enStatus = enI106_Decode_Tmats_Text(pvTmatsText, psuHeader->ulDataLen, psuTmatsInfo);

    return enStatus;
    }


// ------------------------------------------------------------------------

// This routine parses just the text portion of TMATS.

EnI106Status I106_CALL_DECL 
    enI106_Decode_Tmats_Text(void             * pvBuff,
                             uint32_t           ulDataLen,
                             SuTmatsInfo      * psuTmatsInfo)
    {
    unsigned long       iInBuffIdx;
    char              * achInBuff;
    char                szLine[2048];
    int                 iLineIdx;
    char              * szCodeName;
    char              * szDataItem;
    int                 bParseError;

    // Store a copy for module wide use
    m_psuTmatsInfo = psuTmatsInfo;

    // Initialize the TMATS info data structure
    enI106_Free_TmatsInfo(psuTmatsInfo);

    // Initialize the first (and only, for now) G record
    psuTmatsInfo->psuFirstGRecord = (SuGRecord *)TmatsMalloc(sizeof(SuGRecord));
    psuTmatsInfo->psuFirstGRecord->szProgramName       = NULL;
    psuTmatsInfo->psuFirstGRecord->szIrig106Rev        = NULL;
    psuTmatsInfo->psuFirstGRecord->szNumDataSources    = NULL;
    psuTmatsInfo->psuFirstGRecord->psuFirstGDataSource = NULL;

    // Init buffer pointers
    achInBuff    = (char *)pvBuff;
    iInBuffIdx   = 0;

    // Loop until we get to the end of the buffer
    while (bTRUE)
        {

        // If at the end of the buffer then break out of the big loop
//      if (iInBuffIdx >= iBuffSize)
        if (iInBuffIdx >= ulDataLen)
            break;

        // Fill a local buffer with one line
        // ---------------------------------

        // Initialize input line buffer
        szLine[0] = '\0';
        iLineIdx  = 0;

        // Read from buffer until complete line
        while (bTRUE)
            {
            // If at the end of the buffer then break out
//          if (iInBuffIdx >= iBuffSize)
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

        // If errors tokenizing the line then skip over them
        if ((szCodeName == NULL) || (szDataItem == NULL))
            continue;

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

        } // end looping forever on reading TMATS buffer

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

/*
    vConnectRtoG(psuTmatsInfo->psuFirstGRecord, psuTmatsInfo->psuFirstRRecord);
    vConnectMtoR(psuTmatsInfo->psuFirstRRecord, psuTmatsInfo->psuFirstMRecord);
    vConnectBtoM(psuTmatsInfo->psuFirstMRecord, psuTmatsInfo->psuFirstBRecord);
    vConnectPtoM(psuTmatsInfo->psuFirstMRecord, psuTmatsInfo->psuFirstPRecord);
*/

    m_psuTmatsInfo = NULL;

    return I106_OK;
    }



/* -----------------------------------------------------------------------
 * G Records
 * ----------------------------------------------------------------------- 
 */

int bDecodeGLine(char * szCodeName, char * szDataItem, SuGRecord ** ppsuGRecord)
    {
    char          * szCodeField;
    int             iTokens;
    int             iDSIIndex;
    SuGRecord     * psuGRec;
    SuGDataSource * psuDataSource;

    // See which G field it is
    szCodeField = strtok(szCodeName, "\\");
    assert(szCodeField[0] == 'G');

    // Get the G record
    psuGRec = *ppsuGRecord;

    szCodeField = strtok(NULL, "\\");

    // PN - Program Name
    if      (strcasecmp(szCodeField, "PN") == 0)
        {
        psuGRec->szProgramName = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuGRec->szProgramName, szDataItem);
        }

    // 106 - IRIG 106 Ch 9 rev level
    else if (strcasecmp(szCodeField, "106") == 0)
        {
        psuGRec->szIrig106Rev = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuGRec->szIrig106Rev, szDataItem);
        m_iTmatsVersion = atoi(szDataItem);
        } // end if 106

    // DSI - Data source identifier info
    else if (strcasecmp(szCodeField, "DSI") == 0)
        {
        szCodeField = strtok(NULL, "\\");
        // N - Number of data sources
        if (strcasecmp(szCodeField, "N") == 0)
//            psuGRec->iNumDataSources = atoi(szDataItem);
            psuGRec->szNumDataSources = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuGRec->szNumDataSources, szDataItem);
        } // end if DSI

    // DSI-n - Data source identifiers
    else if (strncasecmp(szCodeField, "DSI-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetGDataSource(psuGRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szDataSourceID = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szDataSourceID, szDataItem);
            } // end if DSI Index found
        } // end if DSI-n

    // DST-n - Data source type
    else if (strncasecmp(szCodeField, "DST-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetGDataSource(psuGRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szDataSourceType = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szDataSourceType, szDataItem);
            } // end if DSI Index found
        } // end if DST-n


    return 0;
    }



/* ----------------------------------------------------------------------- */

// Return the G record Data Source record with the given index or
// make a new one if necessary.

SuGDataSource * psuGetGDataSource(SuGRecord * psuGRecord, int iDSIIndex, int bMakeNew)
    {
    SuGDataSource   **ppsuDataSrc = &(psuGRecord->psuFirstGDataSource);

    // Walk the linked list of data sources, looking for a match or
    // the end of the list
    while (bTRUE)
        {
        // If record pointer in linked list is null then exit
        if (*ppsuDataSrc == NULL)
            {
            break;
            }

        // If the data source number matched then record found, exit
        if ((*ppsuDataSrc)->iDataSourceNum == iDSIIndex)
            {
            break;
            }

        // Not found but next record exists so make it our current pointer
        ppsuDataSrc = &((*ppsuDataSrc)->psuNextGDataSource);
        } // end

    // If no record found then put a new one on the end of the list
    if ((*ppsuDataSrc == NULL) && (bMakeNew == bTRUE))
        {
        // Allocate memory for the new record
        *ppsuDataSrc = (SuGDataSource *)TmatsMalloc(sizeof(SuGDataSource));
        memset(*ppsuDataSrc, 0, sizeof(SuGDataSource));
        (*ppsuDataSrc)->iDataSourceNum = iDSIIndex;

        // Now initialize some fields
        //(*ppsuDataSrc)->iDataSourceNum     = iDSIIndex;
        //(*ppsuDataSrc)->szDataSourceID     = NULL;
        //(*ppsuDataSrc)->szDataSourceType   = NULL;
        //(*ppsuDataSrc)->psuRRecord         = NULL;
        //(*ppsuDataSrc)->psuNextGDataSource = NULL;
        }

    return *ppsuDataSrc;
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
            psuCurrRRec = psuCurrRRec->psuNextRRecord;
            } // end while walking the R record list

        // Get the next G data source record
        psuCurrGDataSrc = psuCurrGDataSrc->psuNextGDataSource;
        } // end while walking the G data source records

    return;
    } // end vConnectG()



/* -----------------------------------------------------------------------
 * R Records
 * ----------------------------------------------------------------------- 
 */

int bDecodeRLine(char * szCodeName, char * szDataItem, SuRRecord ** ppsuFirstRRecord)
    {
    char          * szCodeField;
    int             iTokens;
    int             iRIdx;
    int             iDSIIndex;
    SuRRecord     * psuRRec;
    SuRDataSource * psuDataSource;

    // See which R field it is
    szCodeField = strtok(szCodeName, "\\");
    assert(szCodeField[0] == 'R');

    // Get the R record index number
    iTokens = sscanf(szCodeField, "%*1c-%i", &iRIdx);
    if (iTokens == 1)
        {
        psuRRec = psuGetRRecord(ppsuFirstRRecord, iRIdx, bTRUE);
        assert(psuRRec != NULL);
        }
    else
        return 1;
    
    szCodeField = strtok(NULL, "\\");

    // ID - Data source ID
    if     (strcasecmp(szCodeField, "ID") == 0)
        {
        psuRRec->szDataSourceID = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuRRec->szDataSourceID, szDataItem);
        } // end if N

    // N - Number of data sources
    else if (strcasecmp(szCodeField, "N") == 0)
        {
//        psuRRec->iNumDataSources = atoi(szDataItem);
        psuRRec->szNumDataSources = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuRRec->szNumDataSources, szDataItem);
        } // end if N

    // IDX\E - Index enabled
    else if (strcasecmp(szCodeField, "IDX") == 0)
        {
        szCodeField = strtok(NULL, "\\");
        if (strcasecmp(szCodeField, "E") == 0)
            {
            psuRRec->szIndexEnabled = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuRRec->szIndexEnabled, szDataItem);
    //      psuDataSource->bIndexEnabled = (strncasecmp(szDataItem, "T",1) == 0);
            //if (toupper(szDataItem[0]) == 'T')
            //    psuRRec->bIndexEnabled = bTRUE;
            //else
            //    psuRRec->bIndexEnabled = bFALSE;
            psuRRec->bIndexEnabled = toupper(*szFirstNonWhitespace(szDataItem)) == 'T';
            } // end if E
        } // end if IDX

    // EV\E - Events enabled
    else if (strcasecmp(szCodeField, "EV") == 0)
        {
        szCodeField = strtok(NULL, "\\");
        if (strcasecmp(szCodeField, "E") == 0)
            {
            //if (toupper(szDataItem[0]) == 'T')
            //    psuRRec->bEventsEnabled = bTRUE;
            //else
            //    psuRRec->bEventsEnabled = bFALSE;
            psuRRec->szEventsEnabled = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuRRec->szEventsEnabled, szDataItem);
            psuRRec->bEventsEnabled = toupper(*szFirstNonWhitespace(szDataItem)) == 'T';
            } // end if E
        } // end if EV

    // DSI-n - Data source identifier
    else if (strncasecmp(szCodeField, "DSI-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szDataSourceID = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szDataSourceID, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if DSI-n

    // CDT-n/DST-n - Channel data type
    // A certain vendor who will remain nameless (mainly because I don't
    // know which one) encodes the channel data type as a Data Source
    // Type.  This appears to be incorrect according to the Chapter 9
    // spec but can be readily found in Chapter 10 data files.
    else if ((strncasecmp(szCodeField, "CDT-",4) == 0) ||
             (strncasecmp(szCodeField, "DST-",4) == 0))
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szChannelDataType = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szChannelDataType, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if DST-n

    // TK1-n - Track number / Channel number
    else if (strncasecmp(szCodeField, "TK1-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szTrackNumber = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szTrackNumber, szDataItem);
            psuDataSource->iTrackNumber = atoi(szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if TK1-n

    // CHE-n - Channel Enabled
    else if (strncasecmp(szCodeField, "CHE-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szEnabled = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szEnabled, szDataItem);
//            psuDataSource->bEnabled = (strncasecmp(szDataItem, "T",1) == 0);
            psuDataSource->bEnabled = toupper(*szFirstNonWhitespace(szDataItem)) == 'T';
            } // end if DSI Index found
        else
            return 1;
        } // end if CHE-n

    // BDLN-n - Data Link Name (-04, -05)
    else if (strncasecmp(szCodeField, "BDLN-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*4c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szBusDataLinkName = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szBusDataLinkName, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if BDLN-n

    // PDLN-n - PCM Data Link Name (-04, -05)
    else if (strncasecmp(szCodeField, "PDLN-",5) == 0)
        {
        iTokens = sscanf(szCodeField, "%*4c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szPcmDataLinkName = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szPcmDataLinkName, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if PDLN-n

    // CDLN-n - Channel Data Link Name (-07, -09)
    else if (strncasecmp(szCodeField, "CDLN-",5) == 0)
        {
        iTokens = sscanf(szCodeField, "%*4c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szChanDataLinkName = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szChanDataLinkName, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if CDLN-n

    // Video Attributes
    // ----------------

    // VTF-n - Video Data Type Format
    else if (strncasecmp(szCodeField, "VTF-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szVideoDataType = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szVideoDataType, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if VTF-n

    // VXF-n - Video Encoder Type
    else if (strncasecmp(szCodeField, "VXF-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szVideoEncodeType = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szVideoEncodeType, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if VXF-n

    // VST-n - Video Signal Type
    else if (strncasecmp(szCodeField, "VST-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szVideoSignalType = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szVideoSignalType, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if VST-n

    // VSF-n - Video Signal Format
    else if (strncasecmp(szCodeField, "VSF-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szVideoSignalFormat = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szVideoSignalFormat, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if VSF-n

    // CBR-n - Video Constant Bit Rate
    else if (strncasecmp(szCodeField, "CBR-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szVideoConstBitRate = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szVideoConstBitRate, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if CBR-n

    // VBR-n - Video Variable Peak Bit Rate
    else if (strncasecmp(szCodeField, "VBR-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szVideoVarPeakBitRate = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szVideoVarPeakBitRate, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if VBR-n

    // VED-n - Video Encoding Delay
    else if (strncasecmp(szCodeField, "VED-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szVideoEncodingDelay = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szVideoEncodingDelay, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if VED-n

    // PCM Attributes
    // --------------

    // PDTF-n - PCM Data Type Format
    else if (strncasecmp(szCodeField, "PDTF-",5) == 0)
        {
        iTokens = sscanf(szCodeField, "%*4c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szPcmDataTypeFormat = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szPcmDataTypeFormat, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if PDTF-n

    // PDP-n - PCM Data Packing Option
    else if (strncasecmp(szCodeField, "PDP-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szPcmDataPacking = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szPcmDataPacking, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if PDP-n

    // ICE-n - PCM Input Clock Edge
    else if (strncasecmp(szCodeField, "ICE-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szPcmInputClockEdge = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szPcmInputClockEdge, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if ICE-n

    // IST-n - PCM Input Signal Type
    else if (strncasecmp(szCodeField, "IST-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szPcmInputSignalType = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szPcmInputSignalType, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if IST-n

    // ITH-n - PCM Input Threshold
    else if (strncasecmp(szCodeField, "ITH-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szPcmInputThreshold = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szPcmInputThreshold, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if ITH-n

    // ITM-n - PCM Input Termination
    else if (strncasecmp(szCodeField, "ITM-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szPcmInputTermination = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szPcmInputTermination, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if ITM-n

    // PTF-n - PCM Video Type Format
    else if (strncasecmp(szCodeField, "-PTF",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szPcmVideoTypeFormat = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szPcmVideoTypeFormat, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if PTF-n

    // Analog Attributes
    // -----------------

    // ACH - Analog channels
    else if (strncasecmp(szCodeField, "ACH",3) == 0)
        {
        szCodeField = strtok(NULL, "\\");

        // N - Analog channels per packet
        if (strncasecmp(szCodeField, "N-",2) == 0)
            {
            iTokens = sscanf(szCodeField, "N-%i", &iDSIIndex);
            if (iTokens == 1)
                {
                psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
                assert(psuDataSource != NULL);
                psuDataSource->szAnalogChansPerPkt = (char *)TmatsMalloc(strlen(szDataItem)+1);
                strcpy(psuDataSource->szAnalogChansPerPkt, szDataItem);
                } // end if DSI Index found
            else
                return 1;
            } // end if N-n
        } // end if ACH

    // ASR-n - Analog sample rate
    else if (strncasecmp(szCodeField, "ASR-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szAnalogSampleRate = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szAnalogSampleRate, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if ASR-n

    // ADP-n - Analog data packing
    else if (strncasecmp(szCodeField, "ADP-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szAnalogDataPacking = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuDataSource->szAnalogDataPacking, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if ADP-n


    return 0;
    }



/* ----------------------------------------------------------------------- */

SuRRecord * psuGetRRecord(SuRRecord ** ppsuFirstRRecord, int iRIndex, int bMakeNew)
    {
    SuRRecord   ** ppsuCurrRRec = ppsuFirstRRecord;

    // Loop looking for matching index number or end of list
    while (bTRUE)
        {
        // Check for end of list
        if (*ppsuCurrRRec == NULL)
            break;

        // Check for matching index number
        if ((*ppsuCurrRRec)->iRecordNum == iRIndex)
            break;

        // Move on to the next record in the list
        ppsuCurrRRec = &((*ppsuCurrRRec)->psuNextRRecord);
        }

    // If no record found then put a new one on the end of the list
    if ((*ppsuCurrRRec == NULL) && (bMakeNew == bTRUE))
        {
        // Allocate memory for the new record
        *ppsuCurrRRec = (SuRRecord *)TmatsMalloc(sizeof(SuRRecord));
        memset(*ppsuCurrRRec, 0, sizeof(SuRRecord));
        (*ppsuCurrRRec)->iRecordNum = iRIndex;

        } // end if new record

    return *ppsuCurrRRec;
    }



/* ----------------------------------------------------------------------- */

// Return the R record Data Source record with the given index or
// make a new one if necessary.

SuRDataSource * psuGetRDataSource(SuRRecord * psuRRecord, int iDSIIndex, int bMakeNew)
    {
    SuRDataSource   ** ppsuDataSrc = &(psuRRecord->psuFirstDataSource);

    // Walk the linked list of data sources, looking for a match or
    // the end of the list
    while (bTRUE)
        {
        // If record pointer in linked list is null then exit
        if (*ppsuDataSrc == NULL)
            {
            break;
            }

        // If the data source number matched then record found, exit
        if ((*ppsuDataSrc)->iDataSourceNum == iDSIIndex)
            {
            break;
            }

        // Not found but next record exists so make it our current pointer
        ppsuDataSrc = &((*ppsuDataSrc)->psuNextRDataSource);
        } // end

    // If no record found then put a new one on the end of the list
    if ((*ppsuDataSrc == NULL) && (bMakeNew == bTRUE))
        {
        // Allocate memory for the new record
        *ppsuDataSrc = (SuRDataSource *)TmatsMalloc(sizeof(SuRDataSource));
        memset(*ppsuDataSrc, 0, sizeof(SuRDataSource));
        (*ppsuDataSrc)->iDataSourceNum = iDSIIndex;

        } // end if new record

    return *ppsuDataSrc;
    }



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
                psuCurrPRec = psuCurrPRec->psuNextPRecord;
                } // end while walking the P record list

            // Walk the P, B, and S record link lists

            // Get the next R data source record
            psuCurrRDataSrc = psuCurrRDataSrc->psuNextRDataSource;
            } // end while walking the R data source records

        // Get the next R record
        psuCurrRRec = psuCurrRRec->psuNextRRecord;
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
        psuMRec->szDataSourceID = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuMRec->szDataSourceID, szDataItem);
        } // end if ID

    // BSG1 - Baseband signal type
    else if (strcasecmp(szCodeField, "BSG1") == 0)
        {
        psuMRec->szBasebandSignalType = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuMRec->szBasebandSignalType, szDataItem);
        } // end if BSG1

    // BB\DLN - Data link name
    else if (strcasecmp(szCodeField, "BB") == 0)
        {
        szCodeField = strtok(NULL, "\\");
        // DLN - Data link name
        if (strcasecmp(szCodeField, "DLN") == 0)
            {
            psuMRec->szBBDataLinkName = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuMRec->szBBDataLinkName, szDataItem);
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

        // Now initialize some fields
        //(*ppsuCurrMRec)->iRecordNum           = iRIndex;
        //(*ppsuCurrMRec)->szDataSourceID       = NULL;
        //(*ppsuCurrMRec)->szBBDataLinkName     = NULL;
        //(*ppsuCurrMRec)->szBasebandSignalType = NULL;
        //(*ppsuCurrMRec)->psuBRecord           = NULL;
        //(*ppsuCurrMRec)->psuPRecord           = NULL;
        //(*ppsuCurrMRec)->psuNextMRecord       = NULL;
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
            psuCurrPRec = psuCurrPRec->psuNextPRecord;
            } // end while walking the M record list

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
            } // end while walking the M record list

        // Walk through the P record linked list another day


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

    // DLN - Data link name
    if     (strcasecmp(szCodeField, "DLN") == 0)
        {
        psuBRec->szDataLinkName = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuBRec->szDataLinkName, szDataItem);
        } // end if DLN

    // NBS\N - Number of buses
    else if (strncasecmp(szCodeField, "NBS",3) == 0)
        {
        szCodeField = strtok(NULL, "\\");
        // N - Number of channels
        if (strcasecmp(szCodeField, "N") == 0)
            {
            psuBRec->iNumBuses = atoi(szDataItem);
            }
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

        // Now initialize some fields
        //(*ppsuCurrBRec)->iRecordNum         = iRIndex;
        //(*ppsuCurrBRec)->szDataLinkName     = NULL;
        //(*ppsuCurrBRec)->szNumBuses         = NULL;
        //(*ppsuCurrBRec)->psuNextBRecord     = NULL;
        }

    return *ppsuCurrBRec;
    }



/* -----------------------------------------------------------------------
 * P Records
 * ----------------------------------------------------------------------- 
 */

int bDecodePLine(char * szCodeName, char * szDataItem, SuPRecord ** ppsuFirstPRecord)
    {
    char          * szCodeField;
    int             iTokens;
    int             iPIdx;
    SuPRecord     * psuPRec;

    // See which P field it is
    szCodeField = strtok(szCodeName, "\\");
    assert(szCodeField[0] == 'P');

    // Get the P record index number
    iTokens = sscanf(szCodeField, "%*1c-%i", &iPIdx);
    if (iTokens == 1)
        {
        psuPRec = psuGetPRecord(ppsuFirstPRecord, iPIdx, bTRUE);
        assert(psuPRec != NULL);
        }
    else
        return 1;
    
    szCodeField = strtok(NULL, "\\");

    // DLN - Data link name
    if     (strcasecmp(szCodeField, "DLN") == 0)
        {
        psuPRec->szDataLinkName = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szDataLinkName, szDataItem);
        } // end if DLN

    // D1 - PCM Code
    else if (strcasecmp(szCodeField, "D1") == 0)
        {
        psuPRec->szPcmCode = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szPcmCode, szDataItem);
        } // end if D1

    // D2 - Bit Rate
    else if (strcasecmp(szCodeField, "D2") == 0)
        {
        psuPRec->szBitsPerSec = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szBitsPerSec, szDataItem);
        } // end if D2

    // D4 - Polarity
    else if (strcasecmp(szCodeField, "D4") == 0)
        {
        psuPRec->szPolarity = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szPolarity, szDataItem);
        } // end if D4

    // TF - Type Format
    else if (strcasecmp(szCodeField, "TF") == 0)
        {
        psuPRec->szTypeFormat = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szTypeFormat, szDataItem);
        } // end if TF

    // F1 - Common World Length
    else if (strcasecmp(szCodeField, "F1") == 0)
        {
        psuPRec->szCommonWordLen = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szCommonWordLen, szDataItem);
        } // end if F1

    // MF
    else if (strcasecmp(szCodeField, "MF") == 0)
        {
        szCodeField = strtok(NULL, "\\");
        // MF\N - Number of minor frames
        if (strcasecmp(szCodeField, "N") == 0)
            {
            psuPRec->szNumMinorFrames = (char *)TmatsMalloc(strlen(szDataItem)+1);
            strcpy(psuPRec->szNumMinorFrames, szDataItem);
            } // end if MF\N
        } // end if MF

    // MF1 - Number of word in minor frame
    else if (strcasecmp(szCodeField, "MF1") == 0)
        {
        psuPRec->szWordsInMinorFrame = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szWordsInMinorFrame, szDataItem);
        } // end if MF1

    // MF2 - Number of bits in minor frame
    else if (strcasecmp(szCodeField, "MF2") == 0)
        {
        psuPRec->szBitsInMinorFrame = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szBitsInMinorFrame, szDataItem);
        } // end if MF2

    // MF3 - Minor Frame Sync Type
    else if (strcasecmp(szCodeField, "MF3") == 0)
        {
        psuPRec->szMinorFrameSyncType = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szMinorFrameSyncType, szDataItem);
        } // end if MF3

    // MF4 - Minor frame sync pattern length
    else if (strcasecmp(szCodeField, "MF4") == 0)
        {
        psuPRec->szMinorFrameSyncPatLen = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szMinorFrameSyncPatLen, szDataItem);
        } // end if MF4

    // MF5 - Minor frame sync pattern
    else if (strcasecmp(szCodeField, "MF5") == 0)
        {
        psuPRec->szMinorFrameSyncPat = (char *)TmatsMalloc(strlen(szDataItem)+1);
        strcpy(psuPRec->szMinorFrameSyncPat, szDataItem);
        } // end if MF5

    return 0;
    }



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

        // Now initialize some fields
        //(*ppsuCurrPRec)->iRecordNum         = iPIndex;
        //(*ppsuCurrPRec)->szDataLinkName     = NULL;
        //(*ppsuCurrPRec)->szPcmCode          = NULL;
        //(*ppsuCurrPRec)->iBitPerSec         = NULL;
        //(*ppsuCurrPRec)->szPolarity         = NULL;
        //(*ppsuCurrPRec)->szTypeFormat       = NULL;
        //(*ppsuCurrPRec)->szCommonWordLen     = NULL;
        //(*ppsuCurrPRec)->szNumMinorFrames    = 0;
        //(*ppsuCurrPRec)->szWordsInMinorFrame = 0;
        //(*ppsuCurrPRec)->szBitsInMinorFrame  = 0;
        //(*ppsuCurrPRec)->psuNextPRecord     = NULL;
        }

    return *ppsuCurrPRec;
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

#ifdef __cplusplus
} // end namespace i106
#endif
