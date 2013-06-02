/****************************************************************************

 i106_index.c - A higher level interface to the indexing system

 This module provides higher level routines for reading and writing Ch 10
 index information.  It uses the structures and routines in i106_decode_index
 module to read and write data file root and node index packets.

 Copyright (c) 2010 Irig106.org

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

#include <string.h>
#include <stdio.h>

#if !defined(__GNUC__)
#include <io.h>
#endif

#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"

#include "i106_decode_tmats.h"
#include "i106_decode_time.h"
#include "i106_decode_index.h"
#include "i106_index.h"

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

typedef struct SuFileIndex_S
    {
    uint32_t            uNodesUsed;         // Number of index nodes actually used
    uint32_t            uNodesAvailable;    // Number of index nodes available in the table
    uint32_t            uNodeIncrement;     // Amount to increase the number of nodes
    SuI106Ch10Header    suTimeF1Hdr;        // Header and buffer for an IRIG Format 1 time packet. 
    void              * pvTimeF1Packet;     // This is necessary for date format and leap year flags
    SuIrig106Time       suTimeF1;           // as well as relative time to absolute time mapping
                                            // if absolute time isn't provided.
    SuPacketIndexInfo * psuIndexTable;      // The main table of indexes
    } SuCh10Index;

/*
 * Module data
 * -----------
 */

static int                  m_bIndexesInited = bFALSE;      /// Flag to see if index data has been init'ed yet

static SuCh10Index          m_asuCh10Index[MAX_HANDLES];

/*
 * Function Declaration
 * --------------------
 */

void            InitIndexes(void);

EnI106Status    ProcessNodeIndexPacket(int iHandle, int64_t lNodeIndexOffset);
EnI106Status    ProcessRootIndexPacket(int iHandle, int64_t lRootIndexOffset, int64_t * plNextIndexOffset);

void AddIndexNodeToIndex(int iHandle, SuIndex_CurrMsg * psuNodeIndexMsg, uint16_t uChID, uint8_t ubyDataType);
void AddNodeToIndex(int iHandle, SuPacketIndexInfo * psuIndexInfo);

EnI106Status FindTimePacket(int iHandle);

void RelInt2IrigTime(int            iHandle,
                    int64_t         llRelTime,
                    SuIrig106Time * psuTime);

void SortIndexes(int iHandle);

//EnI106Status ReallocIndexTable();
//void DeleteIndexTable();
//EnI106Status ReadTimeDataPacketBody(SuI106Ch10Header suTimeDataHeader);

/* ----------------------------------------------------------------------- */

/**
* Check an open Ch 10 file to see if it has a valid index 
* iHandle     : the handle of the specified IRIG file
* bFoundIndex : True if valid index, false if error or no index
* Return      : I106_OK if data valid
*/
EnI106Status I106_CALL_DECL 
    enIndexPresent(const int iHandle, int * bFoundIndex)
        {
        EnI106Status        enStatus;
        int64_t             llFileOffset;
        SuI106Ch10Header    suI106Hdr;
        unsigned long       ulBuffSize = 0L;
        void              * pvBuff = NULL;
        SuTmatsInfo         suTmatsInfo;

        *bFoundIndex = bFALSE;

        // If data file not open in read mode then return
        if (g_suI106Handle[iHandle].enFileMode != I106_READ)
            return I106_NOT_OPEN;

        // Save current position
        enI106Ch10GetPos(iHandle, &llFileOffset);

        // One time loop to make it easy to break out
        do
            {

            // Check for index support in TMATS
            enStatus = enI106Ch10FirstMsg(iHandle);
            if (enStatus != I106_OK)
                break;

            enStatus = enI106Ch10ReadNextHeader(iHandle, &suI106Hdr);
            if (enStatus != I106_OK)
                break;

            // Make sure TMATS exists
            if (suI106Hdr.ubyDataType != I106CH10_DTYPE_TMATS)
                {
                enStatus = I106_READ_ERROR;
                break;
                }

            // Read TMATS and parse it
            // Make sure the buffer is big enough and read the data
            if (ulBuffSize < suI106Hdr.ulPacketLen)
                {
                pvBuff     = realloc(pvBuff, suI106Hdr.ulPacketLen);
                ulBuffSize = suI106Hdr.ulPacketLen;
                }
            enStatus = enI106Ch10ReadData(iHandle, ulBuffSize, pvBuff);
            if (enStatus != I106_OK)
                break;

            // Process the TMATS info
            memset( &suTmatsInfo, 0, sizeof(suTmatsInfo) );
            enStatus = enI106_Decode_Tmats(&suI106Hdr, pvBuff, &suTmatsInfo);
            if (enStatus != I106_OK)
                break;
                
            // Check if index enabled
            if (suTmatsInfo.psuFirstGRecord->psuFirstGDataSource->psuRRecord->bIndexEnabled == bFALSE)
                {
                enStatus = I106_OK;
                break;
                }

            // Check for index as last packet
            enStatus = enI106Ch10LastMsg(iHandle);
            if (enStatus != I106_OK)
                break;

            enStatus = enI106Ch10ReadNextHeader(iHandle, &suI106Hdr);
            if (enStatus != I106_OK)
                break;

            if (suI106Hdr.ubyDataType == I106CH10_DTYPE_RECORDING_INDEX)
                *bFoundIndex = bTRUE;

            } while (bFALSE); // end one time breakout loop

        // Restore the file position
        enI106Ch10SetPos(iHandle, llFileOffset);

        return enStatus;
        } // end IndexPresent()



/* ----------------------------------------------------------------------- */

/**
* Read an open Ch 10 file, read the various index packets, and build an 
* in-memory table of time and offsets.
* iHandle     : the handle of an IRIG file already opened for reading
* Return      : I106_OK if index data valid
*/

EnI106Status I106_CALL_DECL enReadIndexes(const int iHandle)
    {
    EnI106Status        enStatus = I106_OK;
    int                 bFoundIndex;
    int64_t             llStartingFileOffset;
    int64_t             llCurrRootIndexOffset;
    int64_t             llNextRootIndexOffset;

    // First, see if indexes have been init'ed
    if (m_bIndexesInited == bFALSE)
        InitIndexes();

    // Make sure indexes are in the file
    enStatus = enIndexPresent(iHandle, &bFoundIndex);
    if (enStatus != I106_OK)
        return enStatus;
    if (bFoundIndex == bFALSE)
        return I106_NO_INDEX;

    // Save current position
    enI106Ch10GetPos(iHandle, &llStartingFileOffset);

    // The reading mode must be I106_READ
// TODO : get rid of this global
    if (g_suI106Handle[iHandle].enFileMode != I106_READ )
        {
        return I106_WRONG_FILE_MODE;
        }

    // The optional intrapacket data header provides absolute time in IRIG Time
    // Format 1 format.  Unfortunately there are two important pieces of information
    // that are only in the CSDW, the date format flag and the leap year flag (need 
    // for DoY format).  The time CSDW isn't provided in the index.  So the plan
    // is to go read a time packet and hope that date format and leap year are
    // the same.

    FindTimePacket(iHandle);

    // Place the reading pointer at the last packet which is the Root Index Packet
    enI106Ch10LastMsg(iHandle);

    // Save this file offset
    enStatus = enI106Ch10GetPos(iHandle, &llCurrRootIndexOffset);

	// Root packet found so start processing root index packets
    while (1==1)
        {
        // Process the root packet at the given offset
        enStatus = ProcessRootIndexPacket(iHandle, llCurrRootIndexOffset, &llNextRootIndexOffset);

        // Check for exit conditions
        if (enStatus != I106_OK)
            break;

        if (llCurrRootIndexOffset == llNextRootIndexOffset)
            break;

        // Not done so setup for the next root index packet
        llCurrRootIndexOffset = llNextRootIndexOffset;

        } // end looping on root index packets

    // Sort the resultant index
    SortIndexes(iHandle);

    // Restore the file position
    enI106Ch10SetPos(iHandle, llStartingFileOffset);

	return enStatus;
    } // end enReadIndexes()



/* ----------------------------------------------------------------------- */

EnI106Status ProcessRootIndexPacket(int iHandle, int64_t lRootIndexOffset, int64_t * plNextIndexOffset)
    {
    EnI106Status        enStatus = I106_OK;
    SuI106Ch10Header    suHdr;
    void              * pvRootBuff = NULL;
    SuIndex_CurrMsg     suCurrRootIndexMsg;

    // Go to what should be a root index packet
    enStatus = enI106Ch10SetPos(iHandle, lRootIndexOffset);
    if (enStatus != I106_OK)
        return enStatus;

    // Read what should be a root index packet
    enStatus = enI106Ch10ReadNextHeader(iHandle, &suHdr);

    if (enStatus != I106_OK)
        return enStatus;

    if (suHdr.ubyDataType != I106CH10_DTYPE_RECORDING_INDEX)
        return I106_INVALID_DATA;

    // Read the index packet
    pvRootBuff = malloc(suHdr.ulPacketLen);

    // Read the data buffer
    enStatus = enI106Ch10ReadData(iHandle, suHdr.ulPacketLen, pvRootBuff);

    // Check for data read errors
    if (enStatus != I106_OK)
        return enStatus;

    // Decode the first root index message
    enStatus = enI106_Decode_FirstIndex(&suHdr, pvRootBuff, &suCurrRootIndexMsg);

    // Loop on all root index messages
    while (1==1)
        {
        // Root message, go to node packet and decode
        if      (enStatus == I106_INDEX_ROOT)
            {
            // Go process the node packet
            enStatus = ProcessNodeIndexPacket(iHandle, *(suCurrRootIndexMsg.plFileOffset));
            if (enStatus != I106_OK)
                break;                
            } // end if root index message

        // Last root message links to the next root packet
        else if (enStatus == I106_INDEX_ROOT_LINK)
            {
            *plNextIndexOffset = *(suCurrRootIndexMsg.plFileOffset);
            }

        // If it comes back as a node message then there was a problem
        else if (enStatus == I106_INDEX_NODE)
            {
            enStatus = I106_INVALID_DATA;
            break;
            }

        // Done reading messages from the index buffer
        else if (enStatus == I106_NO_MORE_DATA)
            {
            enStatus = I106_OK;
            break;
            }

        // Any other return status is an error of some sort
        else
            {
            break;
            }

        // Get the next root index message
        enStatus = enI106_Decode_NextIndex(&suCurrRootIndexMsg);

        } // end while walking root index packet

    free(pvRootBuff);

    return enStatus;
    } // end ProcessRootIndexPacket()



/* ----------------------------------------------------------------------- */

// Go to the given offset, read what should be a node index packet, read
// the index values, and add the info to the index memory array.

EnI106Status ProcessNodeIndexPacket(int iHandle, int64_t lNodeIndexOffset)
    {
    EnI106Status        enStatus = I106_OK;
    SuI106Ch10Header    suHdr;
    void              * pvNodeBuff = NULL;
    SuIndex_CurrMsg     suCurrNodeIndexMsg;

    // Go to what should be a node index packet
    enStatus = enI106Ch10SetPos(iHandle, lNodeIndexOffset);
    if (enStatus != I106_OK)
        return enStatus;

    // Read the packet header
    enStatus = enI106Ch10ReadNextHeader(iHandle, &suHdr);

    if (enStatus != I106_OK)
        return enStatus;

    if (suHdr.ubyDataType != I106CH10_DTYPE_RECORDING_INDEX)
        return I106_INVALID_DATA;

    // Make sure our buffer is big enough, size *does* matter
    pvNodeBuff = malloc(suHdr.ulPacketLen);

    // Read the data buffer
    enStatus = enI106Ch10ReadData(iHandle, suHdr.ulPacketLen, pvNodeBuff);

    // Check for data read errors
    if (enStatus != I106_OK)
        return enStatus;

    // Decode the first node index message
    enStatus = enI106_Decode_FirstIndex(&suHdr, pvNodeBuff, &suCurrNodeIndexMsg);

    // Loop on all node index messages
    while (1==1)
        {
        // Node message, go to node packet and decode
        if     (enStatus == I106_INDEX_NODE)
            {
            // Add this node to the index
            AddIndexNodeToIndex(iHandle, &suCurrNodeIndexMsg, suHdr.uChID, suHdr.ubyDataType);
            } // end if node index message

        else if ((enStatus == I106_INDEX_ROOT) || (enStatus == I106_INDEX_ROOT_LINK))
            {
            enStatus = I106_INVALID_DATA;
            break;
            }

        // Done reading messages from the index buffer
        else if (enStatus == I106_NO_MORE_DATA)
            {
            enStatus = I106_OK;
            break;
            }

        // Any other return status is an error of some sort
        else
            break;

        // Get the next node index message
        enStatus = enI106_Decode_NextIndex(&suCurrNodeIndexMsg);

        } // end while walking node index packet

    free(pvNodeBuff);

    return enStatus;
    } // end ProcessNodeIndexPacket()



/* ----------------------------------------------------------------------- */

/** Add an index packet node to the in memory index array
 */

void AddIndexNodeToIndex(int iHandle, SuIndex_CurrMsg * psuNodeIndexMsg, uint16_t uChID, uint8_t ubyDataType)
    {
    SuPacketIndexInfo   suIndexInfo;
    EnI106Status        enStatus;
    SuI106Ch10Header    suHdr;
    void              * pvTimeBuff;
    SuTimeF1_ChanSpec * psuTimeCSDW;

    // Store the info
    suIndexInfo.uChID       =   uChID;
    suIndexInfo.ubyDataType =   ubyDataType;
    suIndexInfo.lFileOffset = *(psuNodeIndexMsg->plFileOffset);
    suIndexInfo.lRelTime    =   psuNodeIndexMsg->psuTime->llTime;

    // If the optional intrapacket data header exists then get absolute time from it
    if (psuNodeIndexMsg->psuChanSpec->bIntraPckHdr == 1)
        {
        psuTimeCSDW = (SuTimeF1_ChanSpec *)m_asuCh10Index[iHandle].pvTimeF1Packet;
        enI106_Decode_TimeF1_Buff(
            psuTimeCSDW->uDateFmt, psuTimeCSDW->bLeapYear,
            psuNodeIndexMsg->psuOptionalTime, &suIndexInfo.suIrigTime);
        }

    // Else if the indexed packet is a time packet then get the time from it
    else if (psuNodeIndexMsg->psuNodeData->uDataType == I106CH10_DTYPE_IRIG_TIME)
        {
        // Go to what should be a time packet
        enStatus = enI106Ch10SetPos(iHandle, *(psuNodeIndexMsg->plFileOffset));
        //if (enStatus != I106_OK)
        //    return enStatus;

        // Read the packet header
        enStatus = enI106Ch10ReadNextHeader(iHandle, &suHdr);

        //if (enStatus != I106_OK)
        //    return enStatus;

        //if (suHdr.ubyDataType != I106CH10_DTYPE_RECORDING_INDEX)
        //    return I106_INVALID_DATA;

        // Make sure our buffer is big enough
        pvTimeBuff = malloc(suHdr.ulPacketLen);

        // Read the data buffer
        enStatus = enI106Ch10ReadData(iHandle, suHdr.ulPacketLen, pvTimeBuff);

        // Check for data read errors
        //if (enStatus != I106_OK)
        //    return enStatus;

        // Decode the time packet
        enI106_Decode_TimeF1(&suHdr, pvTimeBuff, &suIndexInfo.suIrigTime);

        free(pvTimeBuff);

        }

    // Else no absolute time available, so make it from relative time
    else
        {
        RelInt2IrigTime(iHandle, psuNodeIndexMsg->psuTime->llTime, &suIndexInfo.suIrigTime);
        }

    AddNodeToIndex(iHandle, &suIndexInfo);

    return;
    }



/* ----------------------------------------------------------------------- */

/** Add decoded index information to the in memory index array
 */

void AddNodeToIndex(int iHandle, SuPacketIndexInfo * psuIndexInfo)
    {

    // See if we need to make the node table bigger
    if (m_asuCh10Index[iHandle].uNodesAvailable <= m_asuCh10Index[iHandle].uNodesUsed)
        {
        // Reallocate memory
        m_asuCh10Index[iHandle].psuIndexTable = 
            (SuPacketIndexInfo *)realloc(m_asuCh10Index[iHandle].psuIndexTable, 
            (m_asuCh10Index[iHandle].uNodesAvailable + m_asuCh10Index[iHandle].uNodeIncrement) * sizeof(SuPacketIndexInfo));

        m_asuCh10Index[iHandle].uNodesAvailable += m_asuCh10Index[iHandle].uNodeIncrement;

        // Make increment bigger next time
        m_asuCh10Index[iHandle].uNodeIncrement = (uint32_t)(m_asuCh10Index[iHandle].uNodeIncrement * 1.5);
        }

    memcpy(&m_asuCh10Index[iHandle].psuIndexTable[m_asuCh10Index[iHandle].uNodesUsed], psuIndexInfo, sizeof(SuPacketIndexInfo));
    m_asuCh10Index[iHandle].uNodesUsed++;

    return;
    }


/* ----------------------------------------------------------------------- */

/** Make an index of a channel by reading through the data file.
*/

EnI106Status I106_CALL_DECL enMakeIndex(const int iHandle, uint16_t uChID)
    {
    EnI106Status            enStatus;
    SuI106Ch10Header        suI106Hdr;
    unsigned long           ulBuffSize = 0L;
    unsigned char         * pvBuff = NULL;
    SuPacketIndexInfo       suIndexInfo;
    int64_t                 llOffset;

    // First establish time
    FindTimePacket(iHandle);

    // Loop through the file
    while (1==1) 
        {
        // Get the current file offset
        enI106Ch10GetPos(iHandle, &llOffset);

        // Read the next header
        enStatus = enI106Ch10ReadNextHeader(iHandle, &suI106Hdr);

        // Setup a one time loop to make it easy to break out on error
        do
            {
            if (enStatus == I106_EOF)
                break;

            // Check for header read errors
            if (enStatus != I106_OK)
                break;

            // If selected channel then put info into the index
            if (suI106Hdr.uChID == uChID)
                {

                // Make sure our buffer is big enough, size *does* matter
                if (ulBuffSize < suI106Hdr.ulPacketLen)
                    {
                    pvBuff = (unsigned char *)realloc(pvBuff, suI106Hdr.ulPacketLen);
                    ulBuffSize = suI106Hdr.ulPacketLen;
                    }

                // Read the data buffer
                enStatus = enI106Ch10ReadData(iHandle, ulBuffSize, pvBuff);

                // Populate index info
                suIndexInfo.lFileOffset = llOffset;
// TODO:        suIndexInfo.suIrigTime  = ;
                suIndexInfo.ubyDataType = suI106Hdr.ubyDataType;
                suIndexInfo.uChID       = uChID;
                vTimeArray2LLInt(suI106Hdr.aubyRefTime, &suIndexInfo.lRelTime);
                AddNodeToIndex(iHandle, &suIndexInfo);

                } // end if selected Channel ID

            } while (bFALSE); // end one time loop

        // If EOF break out of main read loop
        if (enStatus == I106_EOF)
            break;

        } // End while looping forever

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

/** Find a valid time packet for the index. 
 *  Read the data file from the middle of the file to try to determine a 
 *  valid relative time to clock time from a time packet. Store the result 
 *  the time field in the index.
 */

EnI106Status FindTimePacket(int iHandle)
    {
    int                 bRequireSync = bFALSE;   // Require external time source sync
    int                 iTimeLimit   = 10;       // Max time to look in seconds

    int64_t             llCurrOffset;
    int64_t             llLastMsgOffset;
    int64_t             llTimeLimit;
    int64_t             llCurrTime;
    EnI106Status        enStatus;
    EnI106Status        enRetStatus;
    SuI106Ch10Header    suHdr;
    unsigned long       ulBuffSize = 0;
    void              * pvBuff = NULL;
    SuTimeF1_ChanSpec * psuChanSpecTime = NULL;

    // Get and save the current file position
    enStatus = enI106Ch10GetPos(iHandle, &llCurrOffset);
    if (enStatus != I106_OK)
        return enStatus;

#if 0
    // Get time from the beginning of the file
    enStatus = enI106Ch10FirstMsg(iHandle);;
    if (enStatus != I106_OK)
        return enStatus;
#else
    // Get time from the middle of the file
    enStatus = enI106Ch10LastMsg(iHandle);
    enStatus = enI106Ch10GetPos(iHandle, &llLastMsgOffset);
    enStatus = enI106Ch10SetPos(iHandle,  llLastMsgOffset/2);
#endif

    // Read the next header
    enStatus = enI106Ch10ReadNextHeaderFile(iHandle, &suHdr);
    if (enStatus == I106_EOF)
        return I106_TIME_NOT_FOUND;

    if (enStatus != I106_OK)
        return enStatus;

    // Calculate the time limit if there is one
    if (iTimeLimit > 0)
        {
        vTimeArray2LLInt(suHdr.aubyRefTime, &llTimeLimit);
        llTimeLimit = llTimeLimit + (int64_t)iTimeLimit * (int64_t)10000000;
        }
    else
        llTimeLimit = 0;

    // Loop, looking for appropriate time message
    while (bTRUE)
        {

        // See if we've passed our time limit
        if (llTimeLimit > 0)
            {
            vTimeArray2LLInt(suHdr.aubyRefTime, &llCurrTime);
            if (llTimeLimit < llCurrTime)
                {
                enRetStatus = I106_TIME_NOT_FOUND;
                break;
                }
            } // end if there is a time limit

        // If IRIG time type then process it
        if (suHdr.ubyDataType == I106CH10_DTYPE_IRIG_TIME)
            {

            // Read header OK, make buffer for time message
            if (ulBuffSize < suHdr.ulPacketLen)
                {
                pvBuff          = realloc(pvBuff, suHdr.ulPacketLen);
                psuChanSpecTime = (SuTimeF1_ChanSpec *)pvBuff;
                ulBuffSize      = suHdr.ulPacketLen;
                }

            // Read the data buffer
            enStatus = enI106Ch10ReadData(iHandle, ulBuffSize, pvBuff);
            if (enStatus != I106_OK)
                {
                enRetStatus = I106_TIME_NOT_FOUND;
                break;
                }

            // If external sync OK then decode it and set relative time
            if ((bRequireSync == bFALSE) || (psuChanSpecTime->uTimeSrc == 1))
                {
                memcpy(&m_asuCh10Index[iHandle].suTimeF1Hdr, &suHdr, sizeof(SuI106Ch10Header));
                m_asuCh10Index[iHandle].pvTimeF1Packet = pvBuff;
                enI106_Decode_TimeF1(&suHdr, pvBuff, &m_asuCh10Index[iHandle].suTimeF1);
                enRetStatus = I106_OK;
                break;
                }
            } // end if IRIG time message

        // Read the next header and try again
        enStatus = enI106Ch10ReadNextHeaderFile(iHandle, &suHdr);
        if (enStatus == I106_EOF)
            {
            enRetStatus = I106_TIME_NOT_FOUND;
            break;
            }

        if (enStatus != I106_OK)
            {
            enRetStatus = enStatus;
            break;
            }

        } // end while looping looking for time message

    // Restore file position
    enStatus = enI106Ch10SetPos(iHandle, llCurrOffset);
    if (enStatus != I106_OK)
        {
        enRetStatus = enStatus;
        }

    return enRetStatus;
    } // end FindDateFmtLeapYear()



/* ----------------------------------------------------------------------- */

// Take a 64 bit relative time value and turn it into a real time based on 
// the reference IRIG time from the index.
// This routines was lifted from the enI106_RelInt2IrigTime() from i106_time.
// The difference is that that routine uses the index relative time reference
// rather than the globally maintained reference.

void RelInt2IrigTime(int            iHandle,
                    int64_t         llRelTime,
                    SuIrig106Time * psuTime)
    {
    int64_t         uRefRelTime;
    int64_t         uTimeDiff;
    int64_t         lFracDiff;
    int64_t         lSecDiff;

    int64_t         lSec;
    int64_t         lFrac;

    // Figure out the relative time difference
    vTimeArray2LLInt(m_asuCh10Index[iHandle].suTimeF1Hdr.aubyRefTime, &uRefRelTime);
    uTimeDiff = llRelTime - uRefRelTime;
    lSecDiff  = uTimeDiff / 10000000;
    lFracDiff = uTimeDiff % 10000000;

    lSec      = m_asuCh10Index[iHandle].suTimeF1.ulSecs + lSecDiff;
    lFrac     = m_asuCh10Index[iHandle].suTimeF1.ulFrac + lFracDiff;

    // This seems a bit extreme but it's defensive programming
    while (lFrac < 0)
        {
        lFrac += 10000000;
        lSec  -= 1;
        }
        
    while (lFrac >= 10000000)
        {
        lFrac -= 10000000;
        lSec  += 1;
        }

    // Now add the time difference to the last IRIG time reference
    psuTime->ulFrac = (unsigned long)lFrac;
    psuTime->ulSecs = (unsigned long)lSec;
    psuTime->enFmt  = m_asuCh10Index[iHandle].suTimeF1.enFmt;

    return;
    }



/* ----------------------------------------------------------------------- */

#if 0
int ReallocIndexTable()
    {
    int iReturnValue = 1;
    uiCapacity *= 2;
    IndexTable = (SuIndexTableNode*)realloc(IndexTable, uiCapacity * sizeof(SuIndexTableNode));
    
    return iReturnValue;
    }


/* ----------------------------------------------------------------------- */

void DeleteIndexTable()
    {
    if ( IndexTable )
        free(IndexTable);
    IndexTable = NULL;
    }


/* ----------------------------------------------------------------------- */

int ReadTimeDataPacketBody(SuI106Ch10Header suTimeDataHeader)
    {
    int iRetValue = 1;
    return iRetValue;
    }

#endif

/* ----------------------------------------------------------------------- */

/**
* Initialize an index table.
*/

void InitIndex(int iHandle)
    {
    m_asuCh10Index[iHandle].uNodesAvailable = 0;
    m_asuCh10Index[iHandle].uNodesUsed      = 0;
    m_asuCh10Index[iHandle].uNodeIncrement  = 1000;

    if ((m_bIndexesInited) && (m_asuCh10Index[iHandle].pvTimeF1Packet != NULL))
        free(m_asuCh10Index[iHandle].pvTimeF1Packet);
    m_asuCh10Index[iHandle].pvTimeF1Packet = NULL;

    if ((m_bIndexesInited) && (m_asuCh10Index[iHandle].psuIndexTable != NULL))
        free(m_asuCh10Index[iHandle].psuIndexTable);
    m_asuCh10Index[iHandle].psuIndexTable = NULL;
  
    return;
    }



/* ----------------------------------------------------------------------- */

/**
* Initialize all index tables for the first time
*/

void InitIndexes(void)
    {
    int     iHandleIdx;

    for (iHandleIdx=0; iHandleIdx<MAX_HANDLES; iHandleIdx++)
        InitIndex(iHandleIdx);

    m_bIndexesInited = bTRUE;
  
    return;
    }



/* ----------------------------------------------------------------------- */

/**
* Sort an existing index table in memory by relative time
*/

int CompareIndexes(const void * pIndex1, const void * pIndex2)
    {
    if (((SuPacketIndexInfo *)pIndex1)->lRelTime > ((SuPacketIndexInfo *)pIndex2)->lRelTime)
        return 1;

    if (((SuPacketIndexInfo *)pIndex1)->lRelTime < ((SuPacketIndexInfo *)pIndex2)->lRelTime)
        return -1;

    return 0;
    }


void SortIndexes(int iHandle)
    {
    qsort(
        m_asuCh10Index[iHandle].psuIndexTable, 
        m_asuCh10Index[iHandle].uNodesUsed, 
        sizeof(SuPacketIndexInfo), 
        &CompareIndexes);
    return;
    }



#ifdef __cplusplus
}
#endif

