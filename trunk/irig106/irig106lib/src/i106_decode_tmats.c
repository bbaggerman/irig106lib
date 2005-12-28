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

 Created by Bob Baggerman

 $RCSfile: i106_decode_tmats.c,v $
 $Date: 2005-12-28 00:15:46 $
 $Revision: 1.9 $

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#include "stdint.h"

#include "irig106ch10.h"
#include "i106_decode_tmats.h"


/*
Make a linked list of G's
Make a linked list of R's
Make a linked list of M's
Make a linked list of P's
Make a linked list of B's
Step through B's, connecting them to M
Step through P's, connecting them to M
Step through M's, connecting them to R
Step through R's, connecting them to G
*/


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

// Individual bus attributes
/*
typedef struct SuBusAttr
    {
    char              * szDLN;
    int                 iNumBuses;
    struct SuBusAttr  * psuNext;
    } SuBusAttr;
*/

/*
 * Module data
 * -----------
 */

// This is an empty string that text fields can point to before
// they get a value. This ensures that if fields don't get set while
// reading the TMATS record they will point to something benign.
char                    m_szEmpty[] = "";

SuGRecord               m_suGRec = {m_szEmpty, m_szEmpty, 0, NULL};
SuRRecord             * m_psuFirstRRecord = NULL;
SuMRecord             * m_psuFirstMRecord = NULL;
SuBRecord             * m_psuFirstBRecord = NULL;


/*
 * Function Declaration
 * --------------------
 */

int bDecodeGLine(char * szCodeName, char * szDataItem);
int bDecodeRLine(char * szCodeName, char * szDataItem);
int bDecodeMLine(char * szCodeName, char * szDataItem);
int bDecodeBLine(char * szCodeName, char * szDataItem);

SuRRecord * psuGetRRecord(int iRIndex, int bMakeNew);
SuMRecord * psuGetMRecord(int iRIndex, int bMakeNew);
SuBRecord * psuGetBRecord(int iRIndex, int bMakeNew);

SuGDataSource * psuGetGDataSource(SuGRecord * psuGRec, int iDSIIndex, int bMakeNew);
SuRDataSource * psuGetRDataSource(SuRRecord * psuRRec, int iDSIIndex, int bMakeNew);

void vConnectRtoG(SuGRecord * psuGRec,         SuRRecord * psuFirstRRecord);
void vConnectMtoR(SuRRecord * psuFirstRRecord, SuMRecord * psuFirstMRecord);
void vConnectBtoM(SuMRecord * psuFirstMRecord, SuBRecord * psuFirstBRecord);

/* ======================================================================= */

/* The idea behind this routine is to read the TMATS record, parse it, and 
 * put the various data fields into a tree structure that can be used later
 * to find various settings.
 */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Decode_Tmats(SuI106Ch10Header * psuHeader,
                        void             * pvBuff,
                        unsigned long      iBuffSize,
                        SuTmatsInfo      * psuInfo)
    {
    unsigned long     iInBuffIdx;
    char            * achInBuff;
    char              szLine[2000];
    int               iLineIdx;
    char            * szCodeName;
    char            * szDataItem;
    int               bParseError;

    // Buffer starts past Channel Specific Data
    achInBuff    = (char *)pvBuff;
    iInBuffIdx   = 4;

    // Loop until we get to the end of the buffer
    while (bTRUE)
        {

        // If at the end of the buffer then break out of the big loop
        if (iInBuffIdx >= iBuffSize)
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
            if (iInBuffIdx >= iBuffSize)
                break;

            // If line terminator and line buffer not empty then break out
            if ((achInBuff[iInBuffIdx] == CR)  ||
                (achInBuff[iInBuffIdx] == LF))
                {
                if (strlen(szLine) != 0)
                    break;
                } // end if line terminator

            // Else copy next character to line buffer
            else
                {
                szLine[iLineIdx] = achInBuff[iInBuffIdx];
                if (iLineIdx < 2000)
                  iLineIdx++;
                szLine[iLineIdx] = '\0';
                }

            // Next character from buffer
            iInBuffIdx++;

            } // end while filling complete line

        // Decode the TMATS line
        // ---------------------

        // Go ahead and split the line into left hand and right hand sides
        szCodeName = strtok(szLine, ":");
        szDataItem = strtok(NULL, "");

        // If errors tokenizing the line then skip over them
        if ((szCodeName == NULL) || (szDataItem == NULL))
            continue;

        // Determine and decode different TMATS types
        switch (szCodeName[0])
        {
            case 'G' : // General Information
                bParseError = bDecodeGLine(szCodeName,szDataItem);
                break;

            case 'B' : // Bus Data Attributes
                bParseError = bDecodeBLine(szCodeName, szDataItem);
                break;

            case 'R' : // Tape/Storage Source Attributes
                bParseError = bDecodeRLine(szCodeName, szDataItem);
                break;

            case 'T' : // Transmission Attributes
                break;

            case 'M' : // Multiplexing/Modulation Attributes
                bParseError = bDecodeMLine(szCodeName, szDataItem);
                break;

            case 'P' : // PCM Format Attributes
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

    // Now link the various records together into a tree
    vConnectRtoG(&m_suGRec,         m_psuFirstRRecord);
    vConnectMtoR(m_psuFirstRRecord, m_psuFirstMRecord);
    vConnectBtoM(m_psuFirstMRecord, m_psuFirstBRecord);

    return I106_OK;
    }



/* -----------------------------------------------------------------------
 * G Records
 * ----------------------------------------------------------------------- 
 */

int bDecodeGLine(char * szCodeName, char * szDataItem)
    {
    char          * szCodeField;
    int             iTokens;
    int             iDSIIndex;
    SuGDataSource * psuDataSource;

    // See which G field it is
    szCodeField = strtok(szCodeName, "\\");
    assert(szCodeField[0] == 'G');

    szCodeField = strtok(NULL, "\\");

    // PN - Program Name
    if      (stricmp(szCodeField, "PN") == 0)
        {
        m_suGRec.szProgramName = malloc(strlen(szDataItem)+1);
        assert(m_suGRec.szProgramName != NULL);
        strcpy(m_suGRec.szProgramName, szDataItem);
        }

    // 106 - IRIG 106 rev level
    else if (stricmp(szCodeField, "106") == 0)
        {
        m_suGRec.szIrig106Rev = malloc(strlen(szDataItem)+1);
        assert(m_suGRec.szIrig106Rev != NULL);
        strcpy(m_suGRec.szIrig106Rev, szDataItem);
        } // end if 106

    // DSI - Data source identifier info
    else if (stricmp(szCodeField, "DSI") == 0)
        {
        szCodeField = strtok(NULL, "\\");
        // N - Number of data sources
        if (stricmp(szCodeField, "N") == 0)
            m_suGRec.iNumDataSources = atoi(szDataItem);
        } // end if DSI

    // DSI-n - Data source identifiers
    else if (strnicmp(szCodeField, "DSI-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetGDataSource(&m_suGRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szDataSourceID = malloc(strlen(szDataItem)+1);
            assert(psuDataSource->szDataSourceID != NULL);
            strcpy(psuDataSource->szDataSourceID, szDataItem);
            } // end if DSI Index found
        } // end if DSI-n

    // DST-n - Data source type
    else if (strnicmp(szCodeField, "DST-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetGDataSource(&m_suGRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szDataSourceType = malloc(strlen(szDataItem)+1);
            assert(psuDataSource->szDataSourceType != NULL);
            strcpy(psuDataSource->szDataSourceType, szDataItem);
            } // end if DSI Index found
        } // end if DST-n


    return 0;
    }

/* ----------------------------------------------------------------------- */

// Return the G record Data Source record with the given index or
// make a new one if necessary.

SuGDataSource * psuGetGDataSource(SuGRecord * psuGRec, int iDSIIndex, int bMakeNew)
    {
    SuGDataSource   **ppsuDataSrc = &(psuGRec->psuFirstGDataSource);

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
        *ppsuDataSrc = malloc(sizeof(SuGDataSource));
        assert(*ppsuDataSrc != NULL);

        // Now initialize some fields
        (*ppsuDataSrc)->iDataSourceNum     = iDSIIndex;
        (*ppsuDataSrc)->szDataSourceID     = m_szEmpty;
        (*ppsuDataSrc)->szDataSourceType   = m_szEmpty;
        (*ppsuDataSrc)->psuRRecord         = NULL;
        (*ppsuDataSrc)->psuNextGDataSource = NULL;
        }

    return *ppsuDataSrc;
    }



/* -----------------------------------------------------------------------
 * R Records
 * ----------------------------------------------------------------------- 
 */

int bDecodeRLine(char * szCodeName, char * szDataItem)
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
        psuRRec = psuGetRRecord(iRIdx, bTRUE);
        assert(psuRRec != NULL);
        }
    else
        return 1;
    
    szCodeField = strtok(NULL, "\\");

    // ID - Data source ID
    if     (stricmp(szCodeField, "ID") == 0)
        {
        psuRRec->szDataSourceID = malloc(strlen(szDataItem)+1);
        assert(psuRRec->szDataSourceID != NULL);
        strcpy(psuRRec->szDataSourceID, szDataItem);
        } // end if N

    // N - Number of data sources
    else if (stricmp(szCodeField, "N") == 0)
        {
        psuRRec->iNumDataSources = atoi(szDataItem);
        } // end if N

    // DSI-n - Data source identifier
    else if (strnicmp(szCodeField, "DSI-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szDataSourceID = malloc(strlen(szDataItem)+1);
            assert(psuDataSource->szDataSourceID != NULL);
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
    else if ((strnicmp(szCodeField, "CDT-",4) == 0) ||
             (strnicmp(szCodeField, "DST-",4) == 0))
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->szChannelDataType = malloc(strlen(szDataItem)+1);
            assert(psuDataSource->szChannelDataType != NULL);
            strcpy(psuDataSource->szChannelDataType, szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if DST-n

    // TK1-n - Track number / Channel number
    else if (strnicmp(szCodeField, "TK1-",4) == 0)
        {
        iTokens = sscanf(szCodeField, "%*3c-%i", &iDSIIndex);
        if (iTokens == 1)
            {
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);
            assert(psuDataSource != NULL);
            psuDataSource->iTrackNumber = atoi(szDataItem);
            } // end if DSI Index found
        else
            return 1;
        } // end if TK1-n

    return 0;
    }



/* ----------------------------------------------------------------------- */

SuRRecord * psuGetRRecord(int iRIndex, int bMakeNew)
    {
    SuRRecord   ** ppsuCurrRRec = &m_psuFirstRRecord;

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
        *ppsuCurrRRec = malloc(sizeof(SuRRecord));
        assert(*ppsuCurrRRec != NULL);

        // Now initialize some fields
        (*ppsuCurrRRec)->iRecordNum         = iRIndex;
        (*ppsuCurrRRec)->szDataSourceID     = m_szEmpty;
        (*ppsuCurrRRec)->iNumDataSources    = 0;
        (*ppsuCurrRRec)->psuFirstDataSource = NULL;
        (*ppsuCurrRRec)->psuNextRRecord     = NULL;
        }

    return *ppsuCurrRRec;
    }



/* ----------------------------------------------------------------------- */

// Return the R record Data Source record with the given index or
// make a new one if necessary.

SuRDataSource * psuGetRDataSource(SuRRecord * psuRRec, int iDSIIndex, int bMakeNew)
    {
    SuRDataSource   ** ppsuDataSrc = &(psuRRec->psuFirstDataSource);

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
        *ppsuDataSrc = malloc(sizeof(SuRDataSource));
        assert(*ppsuDataSrc != NULL);

        // Now initialize some fields
        (*ppsuDataSrc)->iDataSourceNum     = iDSIIndex;
        (*ppsuDataSrc)->szDataSourceID     = m_szEmpty;
        (*ppsuDataSrc)->szChannelDataType  = m_szEmpty;
        (*ppsuDataSrc)->iTrackNumber       = 0;
        (*ppsuDataSrc)->psuMRecord         = NULL;
        (*ppsuDataSrc)->psuNextRDataSource = NULL;
        }

    return *ppsuDataSrc;
    }



/* -----------------------------------------------------------------------
 * M Records
 * ----------------------------------------------------------------------- 
 */

int bDecodeMLine(char * szCodeName, char * szDataItem)
    {
    char          * szCodeField;
    int             iTokens;
    int             iRIdx;
    SuMRecord     * psuMRec;
//    SuRDataSource * psuDataSource;

    // See which M field it is
    szCodeField = strtok(szCodeName, "\\");
    assert(szCodeField[0] == 'M');

    // Get the M record index number
    iTokens = sscanf(szCodeField, "%*1c-%i", &iRIdx);
    if (iTokens == 1)
        {
        psuMRec = psuGetMRecord(iRIdx, bTRUE);
        assert(psuMRec != NULL);
        }
    else
        return 1;
    
    szCodeField = strtok(NULL, "\\");

    // ID - Data source ID
    if     (stricmp(szCodeField, "ID") == 0)
        {
        psuMRec->szDataSourceID = malloc(strlen(szDataItem)+1);
        assert(psuMRec->szDataSourceID != NULL);
        strcpy(psuMRec->szDataSourceID, szDataItem);
        } // end if ID

    // BSG1 - Baseband signal type
    else if (stricmp(szCodeField, "BSG1") == 0)
        {
        psuMRec->szBasebandSignalType = malloc(strlen(szDataItem)+1);
        assert(psuMRec->szBasebandSignalType != NULL);
        strcpy(psuMRec->szBasebandSignalType, szDataItem);
        } // end if BSG1

    // BB\DLN - Data link name
    else if (strnicmp(szCodeField, "BB",2) == 0)
        {
        szCodeField = strtok(NULL, "\\");
        // DLN - Data link name
        if (stricmp(szCodeField, "DLN") == 0)
            {
            psuMRec->szDataLinkName = malloc(strlen(szDataItem)+1);
            assert(psuMRec->szDataLinkName != NULL);
            strcpy(psuMRec->szDataLinkName, szDataItem);
            }
        } // end if BB\DLN

    return 0;
    }



/* ----------------------------------------------------------------------- */

SuMRecord * psuGetMRecord(int iRIndex, int bMakeNew)
    {
    SuMRecord   ** ppsuCurrMRec = &m_psuFirstMRecord;

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
        *ppsuCurrMRec = malloc(sizeof(SuMRecord));
        assert(*ppsuCurrMRec != NULL);

        // Now initialize some fields
        (*ppsuCurrMRec)->iRecordNum           = iRIndex;
        (*ppsuCurrMRec)->szDataSourceID       = m_szEmpty;
        (*ppsuCurrMRec)->szDataLinkName       = m_szEmpty;
        (*ppsuCurrMRec)->szBasebandSignalType = m_szEmpty;
        (*ppsuCurrMRec)->psuNextMRecord       = NULL;
        }

    return *ppsuCurrMRec;
    }




/* -----------------------------------------------------------------------
 * M Records
 * ----------------------------------------------------------------------- 
 */

int bDecodeBLine(char * szCodeName, char * szDataItem)
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
        psuBRec = psuGetBRecord(iRIdx, bTRUE);
        assert(psuBRec != NULL);
        }
    else
        return 1;
    
    szCodeField = strtok(NULL, "\\");

    // DLN - Data link name
    if     (stricmp(szCodeField, "DLN") == 0)
        {
        psuBRec->szDataLinkName = malloc(strlen(szDataItem)+1);
        assert(psuBRec->szDataLinkName != NULL);
        strcpy(psuBRec->szDataLinkName, szDataItem);
        } // end if ID

    // NBS\N - Data link name
    else if (strnicmp(szCodeField, "NBS",3) == 0)
        {
        szCodeField = strtok(NULL, "\\");
        // N - Number of channels
        if (stricmp(szCodeField, "N") == 0)
            {
            psuBRec->iNumBuses = atoi(szDataItem);
            }
        } // end if BB\DLN

    return 0;
    }



/* ----------------------------------------------------------------------- */

SuBRecord * psuGetBRecord(int iRIndex, int bMakeNew)
    {
    SuBRecord   ** ppsuCurrBRec = &m_psuFirstBRecord;

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
        *ppsuCurrBRec = malloc(sizeof(SuBRecord));
        assert(*ppsuCurrBRec != NULL);

        // Now initialize some fields
        (*ppsuCurrBRec)->iRecordNum         = iRIndex;
        (*ppsuCurrBRec)->szDataLinkName     = m_szEmpty;
        (*ppsuCurrBRec)->iNumBuses          = 0;
        (*ppsuCurrBRec)->psuNextBRecord     = NULL;
        }

    return *ppsuCurrBRec;
    }



/* -----------------------------------------------------------------------
 * Connect records into a tree structure
 * ----------------------------------------------------------------------- 
 */

// Connect R records with the coresponding G data source record.

void vConnectRtoG(SuGRecord * psuGRec, SuRRecord * psuFirstRRecord)
    {
    SuRRecord       * psuCurrRRec;
    SuGDataSource   * psuCurrGDataSrc;

    // Walk through the R record linked list, looking for a match to the 
    // appropriate G data source record.
    psuCurrRRec = psuFirstRRecord;
    while (psuCurrRRec != NULL)
        {

        // Step through the G data source records looking for a match
        psuCurrGDataSrc = psuGRec->psuFirstGDataSource;
        while (psuCurrGDataSrc != NULL)
            {
            // See if IDs match
            if (stricmp(psuCurrGDataSrc->szDataSourceID,
                        psuCurrRRec->szDataSourceID) == 0)
                {
// If psuCurrGDataSrc->psuRRecord != NULL then that is probably an error in the TMATS file
                psuCurrGDataSrc->psuRRecord = psuCurrRRec;
// If R can't connect to more than one G then we could break here.
                } // end if match

            // Get the next G data source record
            psuCurrGDataSrc = psuCurrGDataSrc->psuNextGDataSource;
            } // end while walking the G data source records

        // Get the next R record
        psuCurrRRec = psuCurrRRec->psuNextRRecord;

        } // end while walking the R record list

    return;
    }



/* ----------------------------------------------------------------------- */

void vConnectMtoR(SuRRecord * psuFirstRRecord, SuMRecord * psuFirstMRecord)
    {
    SuMRecord       * psuCurrMRec;
    SuRRecord       * psuCurrRRec;
    SuRDataSource   * psuCurrRDataSrc;

    // Walk through the M record linked list, looking for a match to the 
    // appropriate R data source record.
    psuCurrMRec = psuFirstMRecord;
    while (psuCurrMRec != NULL)
        {

        // Walk the linked list of R records
        psuCurrRRec = psuFirstRRecord;
        while (psuCurrRRec != NULL)
            {

            // Walk the linked list of R data sources
            psuCurrRDataSrc = psuCurrRRec->psuFirstDataSource;
            while (psuCurrRDataSrc != NULL)
                {

                // See if IDs match
                if (stricmp(psuCurrRDataSrc->szDataSourceID,
                            psuCurrMRec->szDataLinkName) == 0)
                    {
// If psuCurrRDataSrc->psuMRecord != NULL then that is probably an error in the TMATS file
                    psuCurrRDataSrc->psuMRecord = psuCurrMRec;
// If M can't connect to more than one R then we could break here.
                    } // end if match

                // Get the next R data source record
                psuCurrRDataSrc = psuCurrRDataSrc->psuNextRDataSource;
                } // end while walking the R data source records

            // Get the next R record
            psuCurrRRec = psuCurrRRec->psuNextRRecord;
            }

        // Get the next M record
        psuCurrMRec = psuCurrMRec->psuNextMRecord;
        } // end while walking the M record list

    return;
    }



/* ----------------------------------------------------------------------- */

void vConnectBtoM(SuMRecord * psuFirstMRecord, SuBRecord * psuFirstBRecord)
    {
    SuBRecord       * psuCurrBRec;
    SuMRecord       * psuCurrMRec;

    // Walk through the B record linked list, looking for a match to the 
    // appropriate M data source record.
    psuCurrBRec = psuFirstBRecord;
    while (psuCurrBRec != NULL)
        {

        // Walk the linked list of M records
        psuCurrMRec = psuFirstMRecord;
        while (psuCurrMRec != NULL)
            {

            // See if IDs match
            if (stricmp(psuCurrMRec->szDataLinkName,
                        psuCurrBRec->szDataLinkName) == 0)
                {
// If psuCurrMRecord->psuBRecord != NULL then that is probably an error in the TMATS file
                psuCurrMRec->psuBRecord = psuCurrBRec;
// If B can't connect to more than one M then we could break here.
                } // end if match

            // Get the next R record
            psuCurrMRec = psuCurrMRec->psuNextMRecord;
            }

        // Get the next M record
        psuCurrBRec = psuCurrBRec->psuNextBRecord;
        } // end while walking the M record list

    return;
    }



/* ----------------------------------------------------------------------- */


