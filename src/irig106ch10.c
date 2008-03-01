/****************************************************************************

 irig106ch10.c - 

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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <io.h>
//#include <windows.h>    // Needed for Windows 64 bit file I/O???
#endif


#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"

/*
 * Macros and definitions
 * ----------------------
 */



/*
 * Data structures
 * ---------------
 */

/*
struct SuInOrderHdrInfo
    {
    SuI106Ch10Header            suHdr;
    struct SuInOrderHdrInfo   * psuNext;
    struct SuInOrderHdrInfo   * psuPrev;
    };
*/

/*
 * Module data
 * -----------
 */

SuI106Ch10Handle  g_suI106Handle[MAX_HANDLES];
static int        m_bHandlesInited = bFALSE;

// In order read linked list pointers
/*
struct SuInOrderHdrInfo   * m_psuFirstInOrderHdr  = NULL;
struct SuInOrderHdrInfo   * m_psuLastInOrderHdr   = NULL;
struct SuInOrderHdrInfo   * m_psuCurrInOrderHdr   = NULL;

struct SuInOrderHdrInfo   * m_psuFirstInOrderFree = NULL;
*/

/*
 * Function Declaration
 * --------------------
 */

#ifdef LOOK_AHEAD
void vCheckFillLookAheadBuffer(int iHandle);
#endif

/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106Ch10Open(int             * piHandle,
                   const char        szFileName[],
                   EnI106Ch10Mode    enMode)
    {
    int                 iReadCnt;
    int                 iIdx;
    int                 iFlags;
    uint16_t            uSignature;
    EnI106Status        enStatus;
    SuI106Ch10Header    suI106Hdr;

    // Initialize handle data if necessary
    if (m_bHandlesInited == bFALSE)
        {
        for (iIdx=0; iIdx<MAX_HANDLES; iIdx++)
            g_suI106Handle[iIdx].bInUse = bFALSE;
        m_bHandlesInited = bTRUE;
        } // end if file handles not inited yet

    // Get the next available handle
    *piHandle = -1;
    for (iIdx=0; iIdx<MAX_HANDLES; iIdx++)
        {
        if (g_suI106Handle[iIdx].bInUse == bFALSE)
            {
            g_suI106Handle[iIdx].bInUse = bTRUE;
            *piHandle = iIdx;
            break;
            } // end if unused handle found
        } // end looking for unused handle

    if (*piHandle == -1)
        {
        return I106_NO_FREE_HANDLES;
        } // end if handle not found

    // Initialize some data
    g_suI106Handle[*piHandle].enFileState = enClosed;
    g_suI106Handle[*piHandle].suIndex.enSortStatus = enUnsorted;

    // Get a copy of the file name
    strncpy (g_suI106Handle[*piHandle].szFileName, szFileName, sizeof(g_suI106Handle[*piHandle].szFileName));
    g_suI106Handle[*piHandle].szFileName[sizeof(g_suI106Handle[*piHandle].szFileName) - 1] = '\0';

    // Reset total bytes written
    g_suI106Handle[*piHandle].ulTotalBytesWritten = 0L;

/*** Read Mode ***/

    // Open for read
    if ((I106_READ == enMode) || (I106_READ_IN_ORDER == enMode))
        {

        //// Try to open file
#if defined(_MSC_VER)
	iFlags = O_RDONLY | O_BINARY;
#elif defined(__GCC__)
	iFlags = O_RDONLY | O_LARGEFILE;
#else
	iFlags = O_RDONLY;
#endif
        g_suI106Handle[*piHandle].iFile = open(szFileName, iFlags, 0);
        if (g_suI106Handle[*piHandle].iFile == -1)
            return I106_OPEN_ERROR;
    
        //// Check to make sure it is a valid IRIG 106 Ch 10 data file

        // Check for valid signature

        // If we couldn't even read the first 2 bytes then return error
        iReadCnt = read(g_suI106Handle[*piHandle].iFile, &uSignature, 2);
        if (iReadCnt != 2)
            {
            close(g_suI106Handle[*piHandle].iFile);
            return I106_OPEN_ERROR;
            }

        // If the first word isn't the sync value then return error
        if (uSignature != IRIG106_SYNC)
            {
            close(g_suI106Handle[*piHandle].iFile);
            return I106_OPEN_ERROR;
            }

        //// Reading data file looks OK so check some other stuff

        // Open OK and sync character OK so set read state to reflect this
        g_suI106Handle[*piHandle].enFileState = enReadHeader;

        // Make sure first packet is a config packet
//      fseek(g_suI106Handle[*piHandle].pFile, 0L, SEEK_SET);
        enI106Ch10SetPos(*piHandle, 0L);
        enStatus = enI106Ch10ReadNextHeaderFile(*piHandle, &suI106Hdr);
        if (enStatus != I106_OK)
            return I106_OPEN_WARNING;
        if (suI106Hdr.ubyDataType != I106CH10_DTYPE_COMPUTER_1)
            return I106_OPEN_WARNING;

//        // Make sure first dynamic data packet is a time packet
// THERE MAY BE MULTIPLE COMPUTER GENERATED PACKETS AT THE BEGINNING
//        fseek(psuHandle->pFile, suI106Hdr.ulPacketLen, SEEK_SET);
//        enStatus = enI106Ch10ReadNextHeaderFile(*piHandle, &suI106Hdr);
//        if (enStatus != I106_OK)
//            return I106_OPEN_WARNING;
//        if (suI106Hdr.ubyDataType != I106CH10_DTYPE_IRIG_TIME)
//            return I106_OPEN_WARNING;

        // Everything OK so get time and reset back to the beginning
//      fseek(g_suI106Handle[*piHandle].pFile, 0L, SEEK_SET);
        enI106Ch10SetPos(*piHandle, 0L);
        g_suI106Handle[*piHandle].enFileState = enReadHeader;
        g_suI106Handle[*piHandle].enFileMode  = enMode;

        // Do any presorting or indexing
/*
        if (I106_READ_IN_ORDER == enMode)
            {
            g_suI106Handle[*piHandle].suIndex.iArrayUsed = 0;
            vMakeInOrderIndex(*piHandle);
            g_suI106Handle[*piHandle].suIndex.iArrayCurr = 0;
            }
*/

        } // end if read mode


/*** Overwrite Mode ***/

    // Open for overwrite
    else if (I106_OVERWRITE == enMode)
        {

        /// Try to open file
#if defined(_MSC_VER)
	    iFlags = O_WRONLY | O_CREAT | O_BINARY;
#elif defined(__GCC__)
	    iFlags = O_WRONLY | O_CREAT | O_LARGEFILE;
#else
	    iFlags = O_WRONLY | O_CREAT;
#endif
        g_suI106Handle[*piHandle].iFile = open(szFileName, iFlags, _S_IREAD | _S_IWRITE);
        if (g_suI106Handle[*piHandle].iFile == -1)
            return I106_OPEN_ERROR;

        // Open OK and write state to reflect this
        g_suI106Handle[*piHandle].enFileState = enWrite;
        g_suI106Handle[*piHandle].enFileMode  = enMode;
        } // end if read mode


/*** Any other mode is an error ***/

    else
        {
        return I106_OPEN_ERROR;
        }

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106Ch10Close(int iHandle)
    {

    // If handles have been init'ed then bail
    if (m_bHandlesInited == bFALSE)
        return I106_NOT_OPEN;

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
        {
        return I106_INVALID_HANDLE;
        }

    // Make sure the file is really open
    if ((g_suI106Handle[iHandle].iFile   != -1) &&
        (g_suI106Handle[iHandle].bInUse  == bTRUE))
        {
        // Close the file
        close(g_suI106Handle[iHandle].iFile);
        }

    // Free index buffer and mark unsorted
    free(g_suI106Handle[iHandle].suIndex.asuIndex);
    g_suI106Handle[iHandle].suIndex.asuIndex        = NULL;
    g_suI106Handle[iHandle].suIndex.iArraySize      = 0;
    g_suI106Handle[iHandle].suIndex.iArrayUsed      = 0;
    g_suI106Handle[iHandle].suIndex.iNumSearchSteps = 0;
    g_suI106Handle[iHandle].suIndex.enSortStatus    = enUnsorted;

    // Reset some status variables
    g_suI106Handle[iHandle].iFile       = -1;
    g_suI106Handle[iHandle].bInUse      = bFALSE;
    g_suI106Handle[iHandle].enFileState = enClosed;

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

// Get the next header.  Depending on how the file was opened for reading,
// call the appropriate routine.

EnI106Status I106_CALL_DECL 
    enI106Ch10ReadNextHeader(int                iHandle,
                             SuI106Ch10Header * psuHeader)
    {
    EnI106Status    enStatus;

    switch (g_suI106Handle[iHandle].enFileMode)
        {
        case I106_READ : 
            enStatus = enI106Ch10ReadNextHeaderFile(iHandle, psuHeader);
            break;

        case I106_READ_IN_ORDER : 
            if (g_suI106Handle[iHandle].suIndex.enSortStatus == enSorted)
                enStatus = enI106Ch10ReadNextHeaderInOrder(iHandle, psuHeader);
            else
                enStatus = enI106Ch10ReadNextHeaderFile(iHandle, psuHeader);
            break;

        default :
            enStatus = I106_WRONG_FILE_MODE;
            break;
        } // end switch on read mode
    
    return enStatus;
    }



/* ----------------------------------------------------------------------- */

// Get the next header in the file from the current position

EnI106Status I106_CALL_DECL 
    enI106Ch10ReadNextHeaderFile(int                iHandle,
                                 SuI106Ch10Header * psuHeader)
    {
    int                 iReadCnt;
    int                 bReadHeaderWasOK;
    int64_t             llSkipSize;
    int64_t             llFileOffset;
    EnI106Status        enStatus;

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
        {
        return I106_INVALID_HANDLE;
        }

    // Check file state
    switch (g_suI106Handle[iHandle].enFileState)
        {
        case enClosed :
            return I106_NOT_OPEN;
            break;

        case enWrite :
            return I106_WRONG_FILE_MODE;
            break;

        case enReadHeader :
            break;

        case enReadData :
            llSkipSize = g_suI106Handle[iHandle].ulCurrPacketLen - 
                         g_suI106Handle[iHandle].ulCurrHeaderBuffLen -
                         g_suI106Handle[iHandle].ulCurrDataBuffReadPos;
            enStatus = enI106Ch10GetPos(iHandle, &llFileOffset);
            if (enStatus != I106_OK)
                return I106_SEEK_ERROR;

            llFileOffset += llSkipSize;

            enStatus = enI106Ch10SetPos(iHandle, llFileOffset);
            if (enStatus != I106_OK)
                return I106_SEEK_ERROR;

            break;

        case enReadUnsynced :
/*
            // Make sure we are on a 4 byte offset
            enStatus = enI106Ch10GetPos(iHandle, &llFileOffset);
            if (enStatus != I106_OK)
                return I106_SEEK_ERROR;
//          if ((llFileOffset % 4) != 0)
            if ((llFileOffset & 0xfffffffffffffffc) != 0)
                {
//              llFileOffset %= 4;
                llFileOffset = llFileOffset & 0xfffffffffffffffc;
                enStatus = enI106Ch10SetPos(iHandle, llFileOffset);
                if (enStatus != I106_OK)
                    return I106_SEEK_ERROR;
                } // end if not on 4 byte boundary
*/
            break;

        } // end switch on file state

    // Now we might be at the beginning of a header. Read what we think
    // is a header, check it, and keep reading if things don't look correct.
    bReadHeaderWasOK = bTRUE;
    while (bTRUE)
        {

        // Read the header
        iReadCnt = read(g_suI106Handle[iHandle].iFile, psuHeader, HEADER_SIZE);

        // Keep track of how much header we've read
        g_suI106Handle[iHandle].ulCurrHeaderBuffLen = HEADER_SIZE;

        // If there was an error reading, figure out why
        if (iReadCnt != HEADER_SIZE)
            {
            g_suI106Handle[iHandle].enFileState = enReadUnsynced;
            if (iReadCnt == -1)
                return I106_READ_ERROR;
            else
                return I106_EOF;
            } // end if read error

        // Setup a one time loop to make it easy to break out if
        // there is an error encountered
        do
            {
            // Read OK, check the sync field
            if (psuHeader->uSync != IRIG106_SYNC)
                {
                g_suI106Handle[iHandle].enFileState = enReadUnsynced;
                bReadHeaderWasOK = bFALSE;
                break;
                }

            // Always check the header checksum
            if (psuHeader->uChecksum != uCalcHeaderChecksum(psuHeader))
                {
                // If the header checksum was bad then set to unsynced state
                // and return the error. Next time we're called we'll go
                // through lots of heroics to find the next header.
                if (g_suI106Handle[iHandle].enFileState != enReadUnsynced)
                    {
                    g_suI106Handle[iHandle].enFileState = enReadUnsynced;
                    return I106_HEADER_CHKSUM_BAD;
                    }
                bReadHeaderWasOK = bFALSE;
                break;
                }

            // MIGHT NEED TO CHECK HEADER VERSION HERE

            // Header seems OK at this point

            // Figure out if there is a secondary header
            if ((psuHeader->ubyPacketFlags & I106CH10_PFLAGS_SEC_HEADER) != 0)
                {
                // Read the secondary header
                iReadCnt = read(g_suI106Handle[iHandle].iFile, &psuHeader->aulTime[0], SEC_HEADER_SIZE);

                // Keep track of how much header we've read
                g_suI106Handle[iHandle].ulCurrHeaderBuffLen += SEC_HEADER_SIZE;

                // If there was an error reading, figure out why
                if (iReadCnt != HEADER_SIZE)
                    {
                    g_suI106Handle[iHandle].enFileState = enReadUnsynced;
                    if (iReadCnt == -1)
                        return I106_READ_ERROR;
                    else
                        return I106_EOF;
                    } // end if read error

                // Always check the secondary header checksum now
                if (psuHeader->uChecksum != uCalcSecHeaderChecksum(psuHeader))
                    {
                    // If the header checksum was bad then set to unsynced state
                    // and return the error. Next time we're called we'll go
                    // through lots of heroics to find the next header.
                    if (g_suI106Handle[iHandle].enFileState != enReadUnsynced)
                        {
                        g_suI106Handle[iHandle].enFileState = enReadUnsynced;
                        return I106_HEADER_CHKSUM_BAD;
                        }
                    bReadHeaderWasOK = bFALSE;
                    break;
                    }

                } // end if secondary header

            } while (bFALSE); // end one time error testing loop

        // If read header was OK then break out
        if (bReadHeaderWasOK == bTRUE)
            break;

        // Read header was not OK so try again 4 bytes beyond previous read point
        enStatus = enI106Ch10GetPos(iHandle, &llFileOffset);
        if (enStatus != I106_OK)
            return I106_SEEK_ERROR;

        llFileOffset = llFileOffset - g_suI106Handle[iHandle].ulCurrHeaderBuffLen + 1;

        enStatus = enI106Ch10SetPos(iHandle, llFileOffset);
        if (enStatus != I106_OK)
            return I106_SEEK_ERROR;

        } // end while looping forever, looking for a good header
    // Save some data for later use
    g_suI106Handle[iHandle].ulCurrPacketLen       = psuHeader->ulPacketLen;
    g_suI106Handle[iHandle].ulCurrDataBuffLen     = uGetDataLen(psuHeader);
    g_suI106Handle[iHandle].ulCurrDataBuffReadPos = 0;
    g_suI106Handle[iHandle].enFileState           = enReadData;

    return I106_OK;
    } // end enI106Ch10ReadNextHeaderFile()



/* ----------------------------------------------------------------------- */

// Get the next header in time order from the file

EnI106Status I106_CALL_DECL 
    enI106Ch10ReadNextHeaderInOrder(int                iHandle,
                                    SuI106Ch10Header * psuHeader)
    {

    SuIndex           * psuIndex = &g_suI106Handle[iHandle].suIndex;
    EnI106Status        enStatus;
    int64_t             llOffset;
    EnFileState         enSavedFileState;

    // If we're at the end of the list then we are at the end of the file
    if (psuIndex->iArrayCurr == psuIndex->iArrayUsed)
        return I106_EOF;

    // Save the read state going in
    enSavedFileState = g_suI106Handle[iHandle].enFileState;

#ifdef LOOK_AHEAD
vCheckFillLookAheadBuffer(iHandle);
#endif

    // Move file pointer to the proper, er, point
    llOffset = psuIndex->asuIndex[psuIndex->iArrayCurr].llOffset;
    enStatus = enI106Ch10SetPos(iHandle, llOffset);

    // Go ahead and get the next header
    enStatus = enI106Ch10ReadNextHeaderFile(iHandle, psuHeader);

    // If the state was unsynced before but is synced now, figure out where in the
    // index we are
    if ((enSavedFileState == enReadUnsynced) && (g_suI106Handle[iHandle].enFileState != enReadUnsynced))
        {
        enI106Ch10GetPos(iHandle, &llOffset);
        llOffset -= iGetHeaderLen(psuHeader);
        psuIndex->iArrayCurr = 0;
        while (psuIndex->iArrayCurr < psuIndex->iArrayUsed)
            {
            if (llOffset == psuIndex->asuIndex[psuIndex->iArrayCurr].llOffset)
                break;
            psuIndex->iArrayCurr++;
            }
        // if psuIndex->iArrayCurr == psuIndex->iArrayUsed then bad things happened
        }

    // Move array index to the next element
    psuIndex->iArrayCurr++;

    return enStatus;
    } // end enI106Ch10ReadNextHeaderInOrder()



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106Ch10ReadPrevHeader(int                 iHandle,
                             SuI106Ch10Header  * psuHeader)
    {
    int                 bFound;
    int                 iReadCnt;
    int64_t             llSkipSize;
    int64_t             llCurrPos;
    EnI106Status        enStatus;

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
        {
        return I106_INVALID_HANDLE;
        }

// HANDLE THE READ IN ORDER MODE!!!!
//  if (I106_READ_IN_ORDER == enMode)
//      {
//      }

    // Check file mode
    switch (g_suI106Handle[iHandle].enFileState)
        {
        case enClosed :
            return I106_NOT_OPEN;
            break;

        case enWrite :
            return I106_WRONG_FILE_MODE;
            break;

        case enReadHeader :
        case enReadData   :
            // Backup to a point just before the most recently read header.
            // The amount to backup is the size of the previous header and the amount
            // of data already read.
            llSkipSize = g_suI106Handle[iHandle].ulCurrHeaderBuffLen +
                         g_suI106Handle[iHandle].ulCurrDataBuffReadPos;

            // Now to save some time backup more, at least the size of a header with no data
            llSkipSize += HEADER_SIZE;

            break;

        case enReadUnsynced :
            llSkipSize = 4;

            break;
        } // end switch file state


    // Figure out where we're at and where in the file we want to be next
    enI106Ch10GetPos(iHandle, &llCurrPos);

    // If unsynced then make sure we are on a 4 byte offset
    if (g_suI106Handle[iHandle].enFileState == enReadUnsynced)
        llSkipSize = llSkipSize - (llCurrPos % 4);

    llCurrPos -= llSkipSize;

    // Now loop forever looking for a valid packet or die trying
    bFound = bFALSE;
    while (bTRUE)
        {
        // Go to the new position and look for a legal header
        enStatus = enI106Ch10SetPos(iHandle, llCurrPos);
        if (enStatus != I106_OK)
            return I106_SEEK_ERROR;

        // Read and check the header sync
        iReadCnt = read(g_suI106Handle[iHandle].iFile, &(psuHeader->uSync), 2);
        if (iReadCnt != 2)
            return I106_SEEK_ERROR;
        if (psuHeader->uSync != IRIG106_SYNC)
            continue;

        // Sync pattern matched so check the header checksum
        iReadCnt = read(g_suI106Handle[iHandle].iFile, &(psuHeader->uChID), HEADER_SIZE-2);
        if (iReadCnt != HEADER_SIZE-2)
            return I106_SEEK_ERROR;
        if (psuHeader->uChecksum == uCalcHeaderChecksum(psuHeader))
            {
            bFound = bTRUE;
            break;
            }

        // No match, go back 4 more bytes and try again
        llCurrPos -= 4;

        // Check for begining of file
        if (llCurrPos < 0)
            {
            return I106_BOF;
            }

        } // end looping forever

    // If good header found then go back to just before the header
    // and call GetNextHeader() to let it do all the heavy lifting.
    if (bFound == bTRUE)
        {
        enStatus = enI106Ch10SetPos(iHandle, llCurrPos);
        if (enStatus != I106_OK)
            return I106_SEEK_ERROR;
        g_suI106Handle[iHandle].enFileState = enReadHeader;
        enStatus = enI106Ch10ReadNextHeader(iHandle, psuHeader);
        }

    return enStatus;
    } // end enI106Ch10ReadPrevHeader()



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106Ch10ReadData(int                iHandle,
                       unsigned long      ulBuffSize,
                       void             * pvBuff)
    {
    int             iReadCnt;
//    unsigned long   ulSkipSize;
    unsigned long   ulReadAmount;

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
        {
        return I106_INVALID_HANDLE;
        }

    // Check file state
    switch (g_suI106Handle[iHandle].enFileState)
        {
        case enClosed :
            return I106_NOT_OPEN;
            break;

        case enWrite :
            return I106_WRONG_FILE_MODE;
            break;

        case enReadData :
            break;

        default :
// MIGHT WANT TO SUPPORT THE "MORE DATA" METHOD INSTEAD
            g_suI106Handle[iHandle].enFileState = enReadUnsynced;
            return I106_READ_ERROR;
            break;
        } // end switch file state

    // Make sure there is enough room in the user buffer
// MIGHT WANT TO SUPPORT THE "MORE DATA" METHOD INSTEAD
    ulReadAmount = g_suI106Handle[iHandle].ulCurrDataBuffLen -
                   g_suI106Handle[iHandle].ulCurrDataBuffReadPos;
    if (ulBuffSize < ulReadAmount)
        return I106_BUFFER_TOO_SMALL;

    // Read the data, filler, and data checksum
    iReadCnt = read(g_suI106Handle[iHandle].iFile, pvBuff, ulReadAmount);

    // If there was an error reading, figure out why
    if ((unsigned long)iReadCnt != ulReadAmount)
        {
        g_suI106Handle[iHandle].enFileState = enReadUnsynced;
        if (iReadCnt == -1)
            return I106_READ_ERROR;
        else
            return I106_EOF;
        } // end if read error

    // Keep track of our read position in the current data buffer
    g_suI106Handle[iHandle].ulCurrDataBuffReadPos = ulReadAmount;

// MAY WANT TO DO CHECKSUM CHECKING SOMEDAY

    // Expect a header next read
    g_suI106Handle[iHandle].enFileState = enReadHeader;

    return I106_OK;
    } // end enI106Ch10ReadData()
    


/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106Ch10WriteMsg(int                iHandle,
                       SuI106Ch10Header * psuHeader,
                       void             * pvBuff)
    {
    int     iHeaderLen;
    int     iWriteCnt;

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
        {
        return I106_INVALID_HANDLE;
        }

    // Figure out header length
    iHeaderLen = iGetHeaderLen(psuHeader);

    // Write the header
    iWriteCnt = write(g_suI106Handle[iHandle].iFile, psuHeader, iHeaderLen);

    // If there was an error reading, figure out why
    if (iWriteCnt != iHeaderLen)
        {
        return I106_WRITE_ERROR;
        } // end if write error
    
    // Write the data
    iWriteCnt = write(g_suI106Handle[iHandle].iFile, pvBuff, psuHeader->ulPacketLen-iHeaderLen);

    // If there was an error reading, figure out why
    if ((unsigned long)iWriteCnt != (psuHeader->ulPacketLen-iHeaderLen))
        {
        return I106_WRITE_ERROR;
        } // end if write error
    
    return I106_OK;
    }



/* -----------------------------------------------------------------------
 * Move file pointer
 * ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106Ch10FirstMsg(int iHandle)
    {

    if (g_suI106Handle[iHandle].enFileMode == I106_READ_IN_ORDER)
        g_suI106Handle[iHandle].suIndex.iArrayCurr = 0;

    enI106Ch10SetPos(iHandle, 0L);
    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106Ch10LastMsg(int iHandle)
    {
    EnI106Status        enReturnStatus;
    EnI106Status        enStatus;
    int64_t             llPos;
    SuI106Ch10Header    suHeader;
    int                 iReadCnt;
    struct stat         suStatBuff;

    // If its opened for reading in order then just set the index pointer
    // to the last index.
    if (g_suI106Handle[iHandle].enFileMode == I106_READ_IN_ORDER)
        {
        g_suI106Handle[iHandle].suIndex.iArrayCurr = g_suI106Handle[iHandle].suIndex.iArrayUsed-1;
        enReturnStatus = I106_OK;
        }

    // If there is no index then do it the hard way
    else
        {

//      enReturnStatus = I106_SEEK_ERROR;

// MAYBE ALL WE NEED TO DO TO SEEK TO JUST PAST THE END, SET UNSYNC'ED STATE,
// AND THEN CALL enI106Ch10PrevMsg()

        // Figure out how big the file is and go to the end
//      llPos = filelength(_fileno(g_suI106Handle[iHandle].pFile)) - HEADER_SIZE;
        fstat(g_suI106Handle[iHandle].iFile, &suStatBuff);
        llPos = suStatBuff.st_size - HEADER_SIZE;

        //if ((llPos % 4) != 0)
        //    return I106_SEEK_ERROR;

        // Now loop forever looking for a valid packet or die trying
        while (1==1)
            {
            // Not at the beginning so go back 1 byte and try again
            llPos -= 1;

            // Go to the new position and look for a legal header
            enStatus = enI106Ch10SetPos(iHandle, llPos);
            if (enStatus != I106_OK)
                return I106_SEEK_ERROR;

            // Read and check the header
            iReadCnt = read(g_suI106Handle[iHandle].iFile, &suHeader, HEADER_SIZE);

            if (iReadCnt != HEADER_SIZE)
                continue;

            if (suHeader.uSync != IRIG106_SYNC)
                continue;

            // Sync pattern matched so check the header checksum
            if (suHeader.uChecksum == uCalcHeaderChecksum(&suHeader))
                {
                enReturnStatus = I106_OK;
                break;
                }

            // No match, check for begining of file
            if (llPos <= 0)
                {
                enReturnStatus = I106_SEEK_ERROR;
                break;
                }

            } // end looping forever
        } // end if not read in order mode

    // Go back to the good position
    enStatus = enI106Ch10SetPos(iHandle, llPos);

    return enReturnStatus;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106Ch10SetPos(int iHandle, int64_t llOffset)
    {

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
        {
        return I106_INVALID_HANDLE;
        }

    // Seek
#if defined(_MSC_VER)
    {
    __int64  llStatus;
    llStatus = _lseeki64(g_suI106Handle[iHandle].iFile, llOffset, SEEK_SET);
    }
#else
    lseek(g_suI106Handle[iHandle].iFile, llOffset, SEEK_SET);
#endif

    // Can't be sure we're on a message boundary so set unsync'ed
    g_suI106Handle[iHandle].enFileState = enReadUnsynced;

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106Ch10GetPos(int iHandle, int64_t *pllOffset)
    {

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
        {
        return I106_INVALID_HANDLE;
        }

    // Get position
#if defined(_MSC_VER)
    *pllOffset = _telli64(g_suI106Handle[iHandle].iFile);
#else
    *pllOffset = lseek(g_suI106Handle[iHandle].iFile, 0, SEEK_CUR);
#endif
    
    return I106_OK;
    }



/* -----------------------------------------------------------------------
 * Utilities
 * ----------------------------------------------------------------------- */

int I106_CALL_DECL 
    iHeaderInit(SuI106Ch10Header * psuHeader,
                unsigned int       uChanID,
                unsigned int       uDataType,
                unsigned int       uFlags,
                unsigned int       uSeqNum)
    {

    // Make a legal, valid header
    psuHeader->uSync          = IRIG106_SYNC;
    psuHeader->uChID          = uChanID;
    psuHeader->ulPacketLen    = HEADER_SIZE;
    psuHeader->ulDataLen      = 0;
    psuHeader->ubyHdrVer      = 0x02;
    psuHeader->ubySeqNum      = uSeqNum;
    psuHeader->ubyPacketFlags = uFlags;
    psuHeader->ubyDataType    = uDataType;
    memset(&(psuHeader->aubyRefTime), 0, 6);
    psuHeader->uChecksum      = uCalcHeaderChecksum(psuHeader);
    memset(&(psuHeader->aulTime), 0, 8);
    psuHeader->uReserved      = 0;
    psuHeader->uSecChecksum   = uCalcSecHeaderChecksum(psuHeader);

    return 0;
    }

/* ----------------------------------------------------------------------- */

// Figure out header length (might need to check header version at
// some point if I can ever figure out what the different header
// version mean.

int I106_CALL_DECL 
    iGetHeaderLen(SuI106Ch10Header * psuHeader)
    {
    int     iHeaderLen;

    if ((psuHeader->ubyPacketFlags & I106CH10_PFLAGS_SEC_HEADER) == 0)
        iHeaderLen = HEADER_SIZE;
    else
        iHeaderLen = HEADER_SIZE + SEC_HEADER_SIZE;

    return iHeaderLen;
    }




/* ----------------------------------------------------------------------- */

// Figure out data length including padding and any data checksum

uint32_t I106_CALL_DECL 
    uGetDataLen(SuI106Ch10Header * psuHeader)
    {
    int     iDataLen;

    iDataLen = psuHeader->ulPacketLen - iGetHeaderLen(psuHeader);

    return iDataLen;
    }



/* ----------------------------------------------------------------------- */

uint16_t I106_CALL_DECL 
    uCalcHeaderChecksum(SuI106Ch10Header * psuHeader)
    {
    int             iHdrIdx;
    uint16_t        uHdrSum;
    uint16_t      * aHdr = (uint16_t *)psuHeader;

    uHdrSum = 0;
    for (iHdrIdx=0; iHdrIdx<(HEADER_SIZE-2)/2; iHdrIdx++)
        uHdrSum += aHdr[iHdrIdx];

    return uHdrSum;
    }


/* ----------------------------------------------------------------------- */

uint16_t I106_CALL_DECL 
    uCalcSecHeaderChecksum(SuI106Ch10Header * psuHeader)
    {

    int             iByteIdx;
    uint16_t        uHdrSum;
// MAKE THIS 16 BIT UNSIGNEDS LIKE ABOVE
    unsigned char * auchHdrByte = (unsigned char *)psuHeader;

    uHdrSum = 0;
    for (iByteIdx=0; iByteIdx<SEC_HEADER_SIZE-2; iByteIdx++)
        uHdrSum += auchHdrByte[iByteIdx+HEADER_SIZE];

    return uHdrSum;
    }


/* ----------------------------------------------------------------------- */

// Calculate and return the required size of the data buffer portion of the
// packet including checksum and appropriate filler for 4 byte alignment.

uint32_t I106_CALL_DECL 
    uCalcDataBuffReqSize(uint32_t uDataLen, int iChecksumType)
    {
    uint32_t    uDataBuffLen;

    // Start with the length of the data
    uDataBuffLen = uDataLen;

    // Add in enough for the selected checksum
    switch (iChecksumType)
        {
        case I106CH10_PFLAGS_CHKSUM_NONE :
            break;
        case I106CH10_PFLAGS_CHKSUM_8    :
            uDataBuffLen += 1;
            break;
        case I106CH10_PFLAGS_CHKSUM_16   :
            uDataBuffLen += 2;
            break;
        case I106CH10_PFLAGS_CHKSUM_32   :
            uDataBuffLen += 4;
            break;
        default :
            uDataBuffLen = 0;
        } // end switch iChecksumType

    // Now add filler for 4 byte alignment
    uDataBuffLen += 3;
    uDataBuffLen &= 0xfffffffc;

    return uDataBuffLen;
    }


/* ----------------------------------------------------------------------- */

// Add the filler and appropriate checksum to the end of the data buffer
// It is assumed that the buffer is big enough to hold additional filler 
// and the checksum. Also fill in the header with the correct packet length.

EnI106Status I106_CALL_DECL 
    uAddDataFillerChecksum(SuI106Ch10Header * psuI106Hdr, unsigned char achData[])
    {
    uint32_t    uDataIdx;
    uint32_t    uDataBuffSize;
    uint32_t    uFillSize;
    int         iChecksumType;

    uint8_t    *puSum8;
    uint8_t    *puData8;
    uint16_t   *puSum16;
    uint16_t   *puData16;
    uint32_t   *puSum32;
    uint32_t   *puData32;

    // Extract the checksum type
    iChecksumType = psuI106Hdr->ubyPacketFlags & 0x03;

    // Figure out how big the final packet will be
    uDataBuffSize = uCalcDataBuffReqSize(psuI106Hdr->ulDataLen, iChecksumType);
    psuI106Hdr->ulPacketLen = HEADER_SIZE + uDataBuffSize;
    if ((psuI106Hdr->ubyPacketFlags & I106CH10_PFLAGS_SEC_HEADER) != 0)
        psuI106Hdr->ulPacketLen += SEC_HEADER_SIZE;

    // Figure out the filler/checksum size and zero fill it
    uFillSize = uDataBuffSize - psuI106Hdr->ulDataLen;
    memset(&achData[psuI106Hdr->ulDataLen], 0, uFillSize);

    // If no checksum then we're done
    if (iChecksumType == I106CH10_PFLAGS_CHKSUM_NONE)
        return I106_OK;

    // Calculate the checksum
    switch (iChecksumType)
        {
        case I106CH10_PFLAGS_CHKSUM_8    :
            // Checksum the data and filler
            puData8 = (uint8_t *)achData;
            puSum8  = (uint8_t *)&achData[psuI106Hdr->ulDataLen+uFillSize-1];
            for (uDataIdx=0; uDataIdx<uDataBuffSize-1; uDataIdx++)
                {
                *puSum8 += *puData8;
                puData8++;
                }
            break;

        case I106CH10_PFLAGS_CHKSUM_16   :
            puData16 = (uint16_t *)achData;
            puSum16  = (uint16_t *)&achData[psuI106Hdr->ulDataLen+uFillSize-2];
            for (uDataIdx=0; uDataIdx<(uDataBuffSize/2)-1; uDataIdx++)
                {
                *puSum16 += *puData16;
                puData16++;
                }
            break;

        case I106CH10_PFLAGS_CHKSUM_32   :
            puData32 = (uint32_t *)achData;
            puSum32  = (uint32_t *)&achData[psuI106Hdr->ulDataLen+uFillSize-4];
            for (uDataIdx=0; uDataIdx<(uDataBuffSize/4)-1; uDataIdx++)
                {
                *puSum32 += *puData32;
                puData32++;
                }
            break;
        default :
//            uDataBuffLen = 0;
            break;
        } // end switch iChecksumType

    return I106_OK;
    }



// -----------------------------------------------------------------------
// Generate an index from the data file
// -----------------------------------------------------------------------

/*  
Support for read back in time order is experimental.  Some 106-04 recorders 
recorder data *way* out of time order.  But most others don't.  And starting
with 106-05 the most out of order is 1 second.

The best way to support read back in order is to do it on the fly as the file
is being read.  But that's more than I'm willing to do right now.  This indexing
scheme does get the job done for now.
*/

// Read the index from a previously generated index file.

int I106_CALL_DECL 
    bReadInOrderIndex(int iHandle, char * szIdxFileName)
    {
    int                 iIdxFile;
    int                 iFlags;
    int                 iArrayReadStart;
    int                 iReadCnt;
    int                 bReadOK = bFALSE;
    SuIndex           * psuIndex = &g_suI106Handle[iHandle].suIndex;

    // Setup a one time loop to make it easy to break out on errors
    do
        {

        // Try opening and reading the index file
#if defined(_MSC_VER)
	    iFlags = O_RDONLY | O_BINARY;
#else
	    iFlags = O_RDONLY;
#endif
        iIdxFile = open(szIdxFileName, iFlags, 0);
        if (iIdxFile == -1)
            break;

        // Read the index data from the file
        while (bTRUE)
            {
            iArrayReadStart = psuIndex->iArraySize;
            psuIndex->iArraySize += 100;
            psuIndex->asuIndex = (SuFileIndex *)realloc(psuIndex->asuIndex, sizeof(SuFileIndex)*psuIndex->iArraySize);
            iReadCnt = read(iIdxFile, &(psuIndex->asuIndex[iArrayReadStart]), 100*sizeof(SuFileIndex));
            psuIndex->iArrayUsed += iReadCnt / sizeof(SuFileIndex);
            if (iReadCnt != 100*sizeof(SuFileIndex))
                break;
            } // end while reading data from file

        close(iIdxFile);

        // MIGHT WANT TO DO SOME SANITY CHECKS IN HERE

        psuIndex->enSortStatus = enSorted;
        bReadOK = bTRUE;
        } while (bFALSE); // end one time loop to read

    return bReadOK;
    }



// -----------------------------------------------------------------------

int I106_CALL_DECL 
    bWriteInOrderIndex(int iHandle, char * szIdxFileName)
    {
    int                 iFlags;
    int                 iIdxFile;
    int                 iWriteIdx;
    SuIndex           * psuIndex = &g_suI106Handle[iHandle].suIndex;

    // Write out an index file for use next time
#if defined(_MSC_VER)
	iFlags = O_WRONLY | O_CREAT | O_BINARY;
#else
	iFlags = O_WRONLY | O_CREAT;
#endif
    iIdxFile = open(szIdxFileName, iFlags, _S_IREAD | _S_IWRITE);
    if (iIdxFile != -1)
        {

        // Read the index data from the file
        for (iWriteIdx=0; iWriteIdx<psuIndex->iArrayUsed; iWriteIdx++)
            {
            write(iIdxFile, &(psuIndex->asuIndex[iWriteIdx]), sizeof(SuFileIndex));
            } // end for all index array elements

        close(iIdxFile);
        }

    return bFALSE;
    }



// -----------------------------------------------------------------------

// This is used in qsort in vMakeInOrderIndex() below

int FileTimeCompare(const void * psuIndex1, const void * psuIndex2)
    {
    if (((SuFileIndex *)psuIndex1)->llTime < ((SuFileIndex *)psuIndex2)->llTime) return -1;
    if (((SuFileIndex *)psuIndex1)->llTime > ((SuFileIndex *)psuIndex2)->llTime) return  1;
    return 0;
    }


// -----------------------------------------------------------------------

// Read all headers and make an index based on time

void I106_CALL_DECL 
    vMakeInOrderIndex(int iHandle)
    {

    EnI106Status        enStatus;
    int64_t             llStartPos;     // File position coming in
    int64_t             llCurrPos;      // Current file position
    SuI106Ch10Header    suHdr;          // Data packet header
    int64_t             llCurrTime;     // Current header time
    SuIndex           * psuIndex = &g_suI106Handle[iHandle].suIndex;

    // Remember the current file position
    enStatus = enI106Ch10GetPos(iHandle, &llStartPos);

    enStatus = enI106Ch10SetPos(iHandle, 0L);

    // Read headers, put time and file offset into index array
    while (bTRUE)
        {
        enStatus = enI106Ch10ReadNextHeaderFile(iHandle, &suHdr);

        // If EOF break out
        if (enStatus == I106_EOF)
            break;

        // If an error then clean up and get out
        if (enStatus != I106_OK)
            {
            free(psuIndex->asuIndex);
            psuIndex->asuIndex        = NULL;
            psuIndex->iArraySize      = 0;
            psuIndex->iArrayUsed      = 0;
            psuIndex->iNumSearchSteps = 0;
            psuIndex->enSortStatus    = enSortError;
            break;
            }

        // Get the time and position
        enStatus = enI106Ch10GetPos(iHandle, &llCurrPos);
        llCurrPos -= iGetHeaderLen(&suHdr);
        vTimeArray2LLInt(suHdr.aubyRefTime, &llCurrTime);

        // Check the array size, make it bigger if necessary
        if (psuIndex->iArrayUsed >= psuIndex->iArraySize)
            {
            psuIndex->iArraySize += 100;
            psuIndex->asuIndex = 
                (SuFileIndex *)realloc(psuIndex->asuIndex, sizeof(SuFileIndex)*psuIndex->iArraySize);
            }

        // Copy the info into the next array element
        psuIndex->asuIndex[psuIndex->iArrayUsed].llOffset = llCurrPos;
        psuIndex->asuIndex[psuIndex->iArrayUsed].llTime   = llCurrTime;
        psuIndex->iArrayUsed++;
        }

    // Sort the index array
    // It is required that TMATS is the first record and IRIG time is the
    // second record so don't include those in the sort
    qsort(&(psuIndex->asuIndex[2]), psuIndex->iArrayUsed-2, sizeof(SuFileIndex), FileTimeCompare);

    // Put the file point back where we started and find the current index
// THIS SHOULD REALLY BE DONE FOR THE FILE-READ-OK LOGIC PATH ALSO
    enStatus = enI106Ch10SetPos(iHandle, llStartPos);
    psuIndex->iArrayCurr = 0;
    while (psuIndex->iArrayCurr < psuIndex->iArrayUsed)
        {
        if (llStartPos == psuIndex->asuIndex[psuIndex->iArrayCurr].llOffset)
            break;
        psuIndex->iArrayCurr++;
        }

    // If we didn't find it then it's an error
    if (psuIndex->iArrayCurr == psuIndex->iArrayUsed)
        {
        free(psuIndex->asuIndex);
        psuIndex->asuIndex        = NULL;
        psuIndex->iArraySize      = 0;
        psuIndex->iArrayUsed      = 0;
        psuIndex->iNumSearchSteps = 0;
        psuIndex->enSortStatus    = enSortError;
        }
    else
        psuIndex->enSortStatus = enSorted;

    return;
    }


// -----------------------------------------------------------------------
// Routines to support look-ahead sort and reorder
// -----------------------------------------------------------------------

// THIS CODE IS *VERY* EXPERIMENTAL AND HASN'T REALLY BEEN TRIED YET.

#ifdef LOOK_AHEAD

#define LOOK_AHEAD_TIME     5.0


void vCheckFillLookAheadBuffer(int iHandle)
    {
    SuIndex           * psuIndex = &g_suI106Handle[iHandle].suIndex;
    EnI106Status        enStatus;
    SuI106Ch10Header    suHdr;
    int64_t             llLookAheadTime;
    int64_t             llCurrPos;
    int64_t             llCurrTime;

    // Return if look ahead index is full enough
    // Only check if index array has been populated
    if (psuIndex->iArrayUsed > 0)
        {
        if (psuIndex->asuIndex[psuIndex->iArrayCurr].llTime < 
            psuIndex->asuIndex[psuIndex->iArrayUsed-1].llTime + (LOOK_AHEAD_TIME * 10000000))
            return;
        } // end if index array has been populated

    // Shift the index down, discarding old indexes
    memmove(&(psuIndex->asuIndex[0]), 
            &(psuIndex->asuIndex[psuIndex->iArrayCurr]), 
            (psuIndex->iArrayUsed - psuIndex->iArrayCurr) * sizeof(SuFileIndex));

    // Find out the new look ahead time limit
    llLookAheadTime = psuIndex->asuIndex[0].llTime + ((int)(LOOK_AHEAD_TIME * 2.0) * 10000000);

    // Read ahead in file until look ahead time exceeded
    enI106Ch10SetPos(iHandle, psuIndex->llNextReadOffset);

    // Read headers, put time and file offset into index array
    while (bTRUE)
        {
        enStatus = enI106Ch10ReadNextHeaderFile(iHandle, &suHdr);

        // If EOF break out
        if (enStatus == I106_EOF)
            break;

/*
        // If an error then clean up and get out
        if (enStatus != I106_OK)
            {
            free(psuIndex->asuIndex);
            psuIndex->asuIndex        = NULL;
            psuIndex->iArraySize      = 0;
            psuIndex->iArrayUsed      = 0;
            psuIndex->iNumSearchSteps = 0;
            psuIndex->enSortStatus    = enSortError;
            break;
            }
*/

        // Get the time and position
        enStatus = enI106Ch10GetPos(iHandle, &llCurrPos);
        llCurrPos -= iGetHeaderLen(&suHdr);
        vTimeArray2LLInt(suHdr.aubyRefTime, &llCurrTime);

        // Check the array size, make it bigger if necessary
        if (psuIndex->iArrayUsed >= psuIndex->iArraySize)
            {
            psuIndex->iArraySize += 100;
            psuIndex->asuIndex = 
                realloc(psuIndex->asuIndex, sizeof(SuFileIndex)*psuIndex->iArraySize);
            }

        // Copy the info into the next array element
        psuIndex->asuIndex[psuIndex->iArrayUsed].llOffset = llCurrPos;
        psuIndex->asuIndex[psuIndex->iArrayUsed].llTime   = llCurrTime;
        psuIndex->iArrayUsed++;

        psuIndex->llNextReadOffset = llCurrPos + suHdr.ulPacketLen;

        if (llCurrTime > llLookAheadTime)
            break;
        }

    // Sort the index array
    // It is required that TMATS is the first record and IRIG time is the
    // second record so don't include those in the sort
    qsort(&(psuIndex->asuIndex[2]), psuIndex->iArrayUsed-2, sizeof(SuFileIndex), FileTimeCompare);


    // Sort the index

    return;
    }



#endif // LOOK_AHEAD

