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

 Created by Bob Baggerman

 $RCSfile: irig106ch10.c,v $
 $Date: 2006-10-08 16:29:30 $
 $Revision: 1.9 $

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

//#include <io.h>
//#include <fcntl.h>
//#include <stdio.h>


#include "stdint.h"

#include "irig106ch10.h"

/*
 * Macros and definitions
 * ----------------------
 */




/*
 * Data structures
 * ---------------
 */



/*
 * Module data
 * -----------
 */

SuI106Ch10Handle  g_suI106Handle[MAX_HANDLES];


/*
 * Function Declaration
 * --------------------
 */




/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10Open(int             * piHandle,
                   const char        szFileName[],
                   EnI106Ch10Mode    enMode)
    {
    static int          bHandlesInited = bFALSE;
    int                 iIdx;
    int                 iReadCnt;
    uint16_t            uSignature;
    EnI106Status        enStatus;
    SuI106Ch10Header    suI106Hdr;

    // Initialize handle data if necessary
    if (bHandlesInited == bFALSE)
        {
        for (iIdx=0; iIdx<MAX_HANDLES; iIdx++)
            g_suI106Handle[iIdx].bInUse = bFALSE;
        bHandlesInited = bTRUE;
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

    // Get a copy of the file name
    strncpy (g_suI106Handle[*piHandle].szFileName, szFileName, sizeof(g_suI106Handle[*piHandle].szFileName));
    g_suI106Handle[*piHandle].szFileName[sizeof(g_suI106Handle[*piHandle].szFileName) - 1] = '\0';

    // Reset total bytes written
    g_suI106Handle[*piHandle].ulTotalBytesWritten = 0L;

/*** Read Mode ***/

    // Open for read
    if (I106_READ == enMode)
        {
        //// Try to open file

        g_suI106Handle[*piHandle].pFile = fopen(szFileName, "rb");
        if (NULL == g_suI106Handle[*piHandle].pFile)
            return I106_OPEN_ERROR;
    
        //// Check to make sure it is a valid IRIG 106 Ch 10 data file

        // Check for valid signature
        iReadCnt = fread(&uSignature, 2, 1, g_suI106Handle[*piHandle].pFile);

        // If we couldn't even read the first 2 bytes then return error
        if (iReadCnt != 1)
            {
            fclose(g_suI106Handle[*piHandle].pFile);
            return I106_OPEN_ERROR;
            }

        // If the first word isn't the sync value then return error
        if (uSignature != IRIG106_SYNC)
            {
            fclose(g_suI106Handle[*piHandle].pFile);
            return I106_OPEN_ERROR;
            }

        //// Reading data file looks OK so check some other stuff

        // Open OK and sync character OK so set read state to reflect this
        g_suI106Handle[*piHandle].enFileState = enReadHeader;

        // Make sure first packet is a config packet
        fseek(g_suI106Handle[*piHandle].pFile, 0L, SEEK_SET);
        enStatus = enI106Ch10ReadNextHeader(*piHandle, &suI106Hdr);
        if (enStatus != I106_OK)
            return I106_OPEN_WARNING;
        if (suI106Hdr.ubyDataType != I106CH10_DTYPE_COMPUTER_1)
            return I106_OPEN_WARNING;

        // Make sure first data packet is a time packet
// THERE MAY BE MULTIPLE COMPUTER GENERATED PACKETS AT THE BEGINNING
//        fseek(psuHandle->pFile, suI106Hdr.ulPacketLen, SEEK_SET);
        enStatus = enI106Ch10ReadNextHeader(*piHandle, &suI106Hdr);
        if (enStatus != I106_OK)
            return I106_OPEN_WARNING;
        if (suI106Hdr.ubyDataType != I106CH10_DTYPE_IRIG_TIME)
            return I106_OPEN_WARNING;

        // Everything OK so get time and reset back to the beginning
        fseek(g_suI106Handle[*piHandle].pFile, 0L, SEEK_SET);
        g_suI106Handle[*piHandle].enFileState = enReadHeader;

        } // end if read mode


/*** Overwrite Mode ***/

    // Open for overwrite
    else if (I106_OVERWRITE == enMode)
        {

        /// Try to open file
        g_suI106Handle[*piHandle].pFile = fopen(szFileName, "wb");
        if (NULL == g_suI106Handle[*piHandle].pFile)
            return I106_OPEN_ERROR;

        // Open OK and write state to reflect this
        g_suI106Handle[*piHandle].enFileState = enWrite;
    
        } // end if read mode


/*** Any other mode is an error ***/

    else
        {
        return I106_OPEN_ERROR;
        }

//    setvbuf (psuBioHandle->pFile, NULL, _IOFBF, IO_BUFF_SIZE);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10Close(int iHandle)
    {

    // Close the file
    fclose(g_suI106Handle[iHandle].pFile);

    // Reset some status variables
    g_suI106Handle[iHandle].pFile       = NULL;
    g_suI106Handle[iHandle].bInUse      = bFALSE;
    g_suI106Handle[iHandle].enFileState = enClosed;

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10ReadNextHeader(int                iHandle,
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
            // Make sure we are on a 4 byte offset
            enStatus = enI106Ch10GetPos(iHandle, &llFileOffset);
            if (enStatus != I106_OK)
                return I106_SEEK_ERROR;
            if ((llFileOffset % 4) != 0)
                {
                llFileOffset %= 4;
                enStatus = enI106Ch10SetPos(iHandle, llFileOffset);
                if (enStatus != I106_OK)
                    return I106_SEEK_ERROR;
                } // end if not on 4 byte boundary
            break;

        } // end switch on file state

    // Now we might be at the beginning of a header. Read what we think
    // is a header, check it, and keep reading if things don't look correct.
    bReadHeaderWasOK = bTRUE;
    while (bTRUE)
        {

        // Read the header
        iReadCnt = fread(psuHeader, HEADER_SIZE, 1, g_suI106Handle[iHandle].pFile);

        // Keep track of how much header we've read
        g_suI106Handle[iHandle].ulCurrHeaderBuffLen = HEADER_SIZE;

        // If there was an error reading, figure out why
        if (iReadCnt != 1)
            {
            g_suI106Handle[iHandle].enFileState = enReadUnsynced;
            if (feof(g_suI106Handle[iHandle].pFile) != 0)
                return I106_EOF;
            else
                return I106_READ_ERROR;
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

            // If we were unsynced double check the header checksum
            if (g_suI106Handle[iHandle].enFileState == enReadUnsynced)
                if (psuHeader->uChecksum != uCalcHeaderChecksum(psuHeader))
                    {
                    bReadHeaderWasOK = bFALSE;
                    break;
                    }

            // MIGHT NEED TO CHECK HEADER VERSION HERE

            // Header seems OK at this point

            // Figure out if there is a secondary header
            if ((psuHeader->ubyPacketFlags & I106CH10_PFLAGS_SEC_HEADER) != 0)
                {
                // Read the secondary header
                iReadCnt = fread(&psuHeader->aulTime[0], SEC_HEADER_SIZE, 1, g_suI106Handle[iHandle].pFile);

                // Keep track of how much header we've read
                g_suI106Handle[iHandle].ulCurrHeaderBuffLen += SEC_HEADER_SIZE;

                // If there was an error reading, figure out why
                if (iReadCnt != 1)
                    {
                    g_suI106Handle[iHandle].enFileState = enReadUnsynced;
                    if (feof(g_suI106Handle[iHandle].pFile) != 0)
                        return I106_EOF;
                    else
                        return I106_READ_ERROR;
                    } // end if read error

                // If we were unsynced double check the secondary header checksum
                if (g_suI106Handle[iHandle].enFileState == enReadUnsynced)
                    if (psuHeader->uChecksum != uCalcSecHeaderChecksum(psuHeader))
                        {
                        bReadHeaderWasOK = bFALSE;
                        break;
                        }

                } // end if secondary header

            } while (bFALSE); // end one time loop

        // If read header was OK then break out
        if (bReadHeaderWasOK == bTRUE)
            break;

        // Read header was not OK so try again 4 bytes beyond previous read point
        enStatus = enI106Ch10GetPos(iHandle, &llFileOffset);
        if (enStatus != I106_OK)
            return I106_SEEK_ERROR;

        llFileOffset -= g_suI106Handle[iHandle].ulCurrHeaderBuffLen + 4;

        enStatus = enI106Ch10SetPos(iHandle, llFileOffset);
        if (enStatus != I106_OK)
            return I106_SEEK_ERROR;

        } // end while looping forever, looking for a good header


    // Save some data for later use
    g_suI106Handle[iHandle].ulCurrPacketLen       = psuHeader->ulPacketLen;
    g_suI106Handle[iHandle].ulCurrDataBuffLen     = iGetDataLen(psuHeader);
    g_suI106Handle[iHandle].ulCurrDataBuffReadPos = 0;
    g_suI106Handle[iHandle].enFileState           = enReadData;

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
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
        iReadCnt = fread(&(psuHeader->uSync), 2, 1, g_suI106Handle[iHandle].pFile);
        if (iReadCnt != 1)
            return I106_SEEK_ERROR;
        if (psuHeader->uSync != IRIG106_SYNC)
            continue;

        // Sync pattern matched so check the header checksum
        iReadCnt = fread(&(psuHeader->uChID), HEADER_SIZE-2, 1, g_suI106Handle[iHandle].pFile);
        if (iReadCnt != 1)
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
    }



/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10ReadData(int                iHandle,
                       unsigned long    * pulBuffSize,
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
    if (*pulBuffSize < ulReadAmount)
        return I106_BUFFER_TOO_SMALL;

    // Read the data, filler, and data checksum
    iReadCnt = fread(pvBuff, ulReadAmount, 1, g_suI106Handle[iHandle].pFile);

    // If there was an error reading, figure out why
    if (iReadCnt != 1)
        {
        g_suI106Handle[iHandle].enFileState = enReadUnsynced;
        if (feof(g_suI106Handle[iHandle].pFile) != 0)
            return I106_EOF;
        else
            return I106_READ_ERROR;
        } // end if read error

    // Keep track of our read position in the current data buffer
    g_suI106Handle[iHandle].ulCurrDataBuffReadPos = ulReadAmount;

// MAY WANT TO DO CHECKSUM CHECKING SOMEDAY

    // Expect a header next read
    g_suI106Handle[iHandle].enFileState = enReadHeader;

    return I106_OK;
    }
    


/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
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
    iWriteCnt = fwrite(psuHeader, iHeaderLen, 1, g_suI106Handle[iHandle].pFile);

    // If there was an error reading, figure out why
    if (iWriteCnt != 1)
        {
        return I106_WRITE_ERROR;
        } // end if write error
    
    // Write the data
    iWriteCnt = fwrite(pvBuff, psuHeader->ulPacketLen-iHeaderLen, 1, g_suI106Handle[iHandle].pFile);

    // If there was an error reading, figure out why
    if (iWriteCnt != 1)
        {
        return I106_WRITE_ERROR;
        } // end if write error
    
    return I106_OK;
    }



/* -----------------------------------------------------------------------
 * Move file pointer
 * ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10FirstMsg(int iHandle)
    {
    enI106Ch10SetPos(iHandle, 0L);
    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10LastMsg(int iHandle)
    {
    EnI106Status        enReturnStatus;
    EnI106Status        enStatus;
//  __int64             llPos;
    int64_t             llPos;
    SuI106Ch10Header    suHeader;
    int                 iReadCnt;
    struct stat         suStatBuff;


//    enReturnStatus = I106_SEEK_ERROR;

// MAYBE ALL WE NEED TO DO TO SEEK TO JUST PAST THE END, SET UNSYNC'ED STATE,
// AND THEN CALL enI106Ch10PrevMsg()

    // Figure out how big the file is and go to the end
//    llPos = filelength(_fileno(g_suI106Handle[iHandle].pFile)) - HEADER_SIZE;
    fstat(_fileno(g_suI106Handle[iHandle].pFile), &suStatBuff);
    llPos = suStatBuff.st_size;
    if ((llPos % 4) != 0)
        return I106_SEEK_ERROR;

    // Now loop forever looking for a valid packet or die trying
    while (1==1)
        {
        // Go to the new position and look for a legal header
        enStatus = enI106Ch10SetPos(iHandle, llPos);
        if (enStatus != I106_OK)
            return I106_SEEK_ERROR;

        // Read and check the header
        iReadCnt = fread(&suHeader, HEADER_SIZE, 1, g_suI106Handle[iHandle].pFile);
        if (iReadCnt != 1)
            return I106_SEEK_ERROR;
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

        // Not at the beginning so go back 4 more bytes and try again
        llPos -= 4;

        } // end looping forever

    return enReturnStatus;
    }



/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10SetPos(int iHandle, int64_t llOffset)
    {
//    fpos_t      llPos;

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
        {
        return I106_INVALID_HANDLE;
        }

    // Seek
//    llPos = (fpos_t)llOffset;
//    fsetpos(g_suI106Handle[iHandle].pFile, &llPos);
    fseek(g_suI106Handle[iHandle].pFile, (long)llOffset, SEEK_SET);

    // Can't be sure we're on a message boundary so set unsync'ed
    g_suI106Handle[iHandle].enFileState = enReadUnsynced;

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10GetPos(int iHandle, int64_t *pllOffset)
    {
//    fpos_t      llPos;

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
        {
        return I106_INVALID_HANDLE;
        }

//    fgetpos(g_suI106Handle[iHandle].pFile, &llPos);
    *pllOffset = ftell(g_suI106Handle[iHandle].pFile);

    return I106_OK;
    }



/* -----------------------------------------------------------------------
 * Utilities
 * ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC int I106_CALL_DECL 
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
    memset(&(psuHeader->aulTime),     0, 8);
    psuHeader->uReserved      = 0;
    psuHeader->uSecChecksum   = uCalcSecHeaderChecksum(psuHeader);

    return 0;
    }

/* ----------------------------------------------------------------------- */

// Figure out header length (might need to check header version at
// some point if I can ever figure out what the different header
// version mean.

I106_DLL_DECLSPEC int I106_CALL_DECL 
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

I106_DLL_DECLSPEC int I106_CALL_DECL 
    iGetDataLen(SuI106Ch10Header * psuHeader)
    {
    int     iDataLen;

    iDataLen = psuHeader->ulPacketLen - iGetHeaderLen(psuHeader);

    return iDataLen;
    }



/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC uint16_t I106_CALL_DECL 
    uCalcHeaderChecksum(SuI106Ch10Header * psuHeader)
    {
    int             iByteIdx;
    uint16_t        uHdrSum;
    unsigned char * auchHdrByte = (unsigned char *)psuHeader;

    uHdrSum = 0;
    for (iByteIdx=0; iByteIdx<HEADER_SIZE-2; iByteIdx++)
        uHdrSum += auchHdrByte[iByteIdx];

    return uHdrSum;
    }


/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC uint16_t I106_CALL_DECL 
    uCalcSecHeaderChecksum(SuI106Ch10Header * psuHeader)
    {

    int             iByteIdx;
    uint16_t        uHdrSum;
    unsigned char * auchHdrByte = (unsigned char *)psuHeader;

    uHdrSum = 0;
    for (iByteIdx=0; iByteIdx<SEC_HEADER_SIZE-2; iByteIdx++)
        uHdrSum += auchHdrByte[iByteIdx+HEADER_SIZE];

    return uHdrSum;
    }


/* ----------------------------------------------------------------------- */

// Calculate and return the required size of the data buffer portion of the
// packet including checksum and appropriate filler for 4 byte alignment.

I106_DLL_DECLSPEC uint32_t I106_CALL_DECL 
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

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
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



