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
 $Date: 2005-12-28 14:41:41 $
 $Revision: 1.4 $

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdint.h"

#include "irig106ch10.h"

/*
 * Macros and definitions
 * ----------------------
 */

#define MAX_HANDLES         4

#define IRIG106_SYNC        0xEB25
#define HEADER_SIZE         24
#define SEC_HEADER_SIZE     12



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
    g_suI106Handle[*piHandle].enReadState = enClosed;

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
        g_suI106Handle[*piHandle].enReadState = enHeader;

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
        g_suI106Handle[*piHandle].enReadState = enHeader;

        } // end if read mode


/*** Overwrite Mode ***/

    // Open for overwrite
    else if (I106_OVERWRITE == enMode)
        {

        /// Try to open file
        g_suI106Handle[*piHandle].pFile = fopen(szFileName, "wb");
        if (NULL == g_suI106Handle[*piHandle].pFile)
            return I106_OPEN_ERROR;
    
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
    g_suI106Handle[iHandle].pFile  = NULL;
    g_suI106Handle[iHandle].bInUse = bFALSE;

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10ReadNextHeader(int                iHandle,
                             SuI106Ch10Header * psuHeader)
    {
    int                 iReadCnt;
    unsigned long       ulSkipSize;

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
    {
        return I106_INVALID_HANDLE;
    }

// HANDLE THE UNSYNCED CASE

    // If state is enData then skip the data and go to the next header
    if (g_suI106Handle[iHandle].enReadState == enData)
    {
        ulSkipSize = g_suI106Handle[iHandle].ulCurrPacketLen - g_suI106Handle[iHandle].ulCurrHdrLen;
        fseek(g_suI106Handle[iHandle].pFile, ulSkipSize, SEEK_CUR);
        g_suI106Handle[iHandle].enReadState = enHeader;
    }

    // Make sure we think we're positioned to the beginning of the header
    if (g_suI106Handle[iHandle].enReadState != enHeader)
    {
        g_suI106Handle[iHandle].enReadState = enUnsynced;
        return I106_READ_ERROR;
    }

    // Read the header
    iReadCnt = fread(psuHeader, HEADER_SIZE, 1, g_suI106Handle[iHandle].pFile);

    // If there was an error reading, figure out why
    if (iReadCnt != 1)
    {
        g_suI106Handle[iHandle].enReadState = enUnsynced;
        if (feof(g_suI106Handle[iHandle].pFile) != 0)
            return I106_EOF;
        else
            return I106_READ_ERROR;
    } // end if read error

    // Do some reasonableness tests
    if (psuHeader->uSync != IRIG106_SYNC)
    {
        g_suI106Handle[iHandle].enReadState = enUnsynced;
        return I106_READ_ERROR;
    }

    // MIGHT NEED TO CHECK HEADER VERSION HERE

    g_suI106Handle[iHandle].ulCurrHdrLen = HEADER_SIZE;

    // No error so figure out if there is a secondary header
    if ((psuHeader->ubyPacketFlags & I106CH10_PFLAGS_SEC_HEADER) != 0)
        {
        // Read the secondary header
        iReadCnt = fread(&psuHeader->aulTime[0], SEC_HEADER_SIZE, 1, g_suI106Handle[iHandle].pFile);

        // If there was an error reading, figure out why
        if (iReadCnt != 1)
            {
            g_suI106Handle[iHandle].enReadState = enUnsynced;
            if (feof(g_suI106Handle[iHandle].pFile) != 0)
                return I106_EOF;
            else
                return I106_READ_ERROR;
            } // end if read error

        g_suI106Handle[iHandle].ulCurrHdrLen += SEC_HEADER_SIZE;
        } // end if secondary header

    // Save some data for later use
    g_suI106Handle[iHandle].ulCurrPacketLen = psuHeader->ulPacketLen;
    g_suI106Handle[iHandle].ulCurrDataLen   = psuHeader->ulDataLen;
    g_suI106Handle[iHandle].enReadState     = enData;

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10ReadNextData(int                iHandle,
                           unsigned long    * pulBuffSize,
                           void             * pvBuff)
    {
    int             iReadCnt;
    unsigned long   ulSkipSize;

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
    {
        return I106_INVALID_HANDLE;
    }

    // Make sure we're positioned to the beginning of the data record
    if (g_suI106Handle[iHandle].enReadState != enData)
        {
        g_suI106Handle[iHandle].enReadState = enUnsynced;
        return I106_READ_ERROR;
        }

    // Make sure there is enough room in the user buffer
// MIGHT WANT TO SUPPORT THE "MORE DATA" METHOD INSTEAD
    if (*pulBuffSize < g_suI106Handle[iHandle].ulCurrDataLen)
        return I106_BUFFER_TOO_SMALL;

    // Read the data
    iReadCnt = fread(pvBuff, g_suI106Handle[iHandle].ulCurrDataLen, 1, g_suI106Handle[iHandle].pFile);

    // If there was an error reading, figure out why
    if (iReadCnt != 1)
        {
        g_suI106Handle[iHandle].enReadState = enUnsynced;
        if (feof(g_suI106Handle[iHandle].pFile) != 0)
            return I106_EOF;
        else
            return I106_READ_ERROR;
        } // end if read error

// MAY WANT TO DO CHECKSUM CHECKING SOMEDAY

    // Skip past data checksum and filler
    ulSkipSize = g_suI106Handle[iHandle].ulCurrPacketLen - 
                    (g_suI106Handle[iHandle].ulCurrHdrLen + g_suI106Handle[iHandle].ulCurrDataLen);
    fseek(g_suI106Handle[iHandle].pFile, ulSkipSize, SEEK_CUR);

    // Expect a header next read
    g_suI106Handle[iHandle].enReadState = enHeader;

    return I106_OK;
    }
    


/* ----------------------------------------------------------------------- */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10WriteNextMsg(int                iHandle,
                             SuI106Ch10Header * psuHeader)
    {
    int                 iReadCnt;
    unsigned long       ulSkipSize;

    // Check for a valid handle
    if ((iHandle < 0)           || 
        (iHandle > MAX_HANDLES) || 
        (g_suI106Handle[iHandle].bInUse == bFALSE))
        {
        return I106_INVALID_HANDLE;
        }


    return I106_OK;
    }