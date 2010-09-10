/****************************************************************************

 i106_index.c - 

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

#include "stdint.h"
#include "irig106ch10.h"
#include "i106_decode_tmats.h"
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


/*
 * Module data
 * -----------
 */

/** The index table data **/
//uint64_t          * g_u64NodeIndexOffsetTable;
//uint32_t          * g_u32TimeIndexTable;

static int                  bIndexesInited = bFALSE;        /// Flag to see if index data has been init'ed yet

static uint32_t             uNodesUsed[MAX_HANDLES];        /// Number of index nodes actually used
static uint32_t             uNodesAvailable[MAX_HANDLES];   /// Number of index nodes available in the table
static SuIndexTableNode   * IndexTable[MAX_HANDLES];


/*
 * Function Declaration
 * --------------------
 */

void    InitIndexes(void);

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

            } while (false); // end one time breakout loop

        // Restore the file position
        enI106Ch10SetPos(iHandle, llFileOffset);

        return enStatus;
        } // end IndexPresent()



/* ----------------------------------------------------------------------- */

/**
* Read an open a Ch 10 file, read the various index packets, and build an 
* in-memory table of time and offsets.
* iHandle         : the handle of an IRIG file already opened for reading
* Return      : I106_OK if data valid
*/

EnI106Status I106_CALL_DECL enReadIndexes(const int iHandle)
    {
    EnI106Status        enStatus = I106_OK;
    int                 bFoundIndex;
    int64_t             llFileOffset;

    //int                 iReadCnt;
    //int64_t             i64RootIndPackOffset;
    //int64_t             llNewPos;

    // First, see if indexes have been init'ed
    if (bIndexesInited == bFALSE)
        InitIndexes();

    // Make sure indexes are in the file
    enStatus = enIndexPresent(iHandle, &bFoundIndex);
    if (enStatus != I106_OK)
        return enStatus;
    if (bFoundIndex == bFALSE)
        return I106_NO_INDEX;

    // Save current position
    enI106Ch10GetPos(iHandle, &llFileOffset);


#if 0

    // Initilize the global index table variables
    uiSize     = 0;
    uiCapacity = 50;
    IndexTable = (SuIndexTableNode *)malloc(sizeof(SuIndexTableNode));
    
    if ( enI106Ch10Open(&iHandle, strIrigFileName, enMode)!=I106_OK )
        {
        return I106_OPEN_ERROR;
        }

    // The reading mode must be I106_READ
    if ( enMode!=I106_READ )
        {
        return I106_WRONG_FILE_MODE;
        }

    // Place the reading pointer at the last packet which is the Root Index Packet
    enI106Ch10LastMsg(iHandle);
    
    enI106Ch10GetPos(iHandle, &i64RootIndPackOffset);
    if ((iReadCnt = read(g_suI106Handle[iHandle].iFile, &suHeader, HEADER_SIZE))!=HEADER_SIZE)
        {
        return 0;  // FIX
        }

        enI106Ch10GetPos(iHandle, &llNewPos);

    if ( suHeader.ubyDataType!=0x03 )
        {
        iReturnValue = 0;
        }

    else
        {
        ProcessRootIndexPackBody(iHandle, suHeader);
        }
    
    enI106Ch10Close(iHandle);

#endif
    return enStatus;
    }



/* ----------------------------------------------------------------------- */

#if 0
int I106_CALL_DECL SaveIndexTable(char* strFileName)
    {
    int             iReturnValue = 1;
    unsigned int    i; // FIX
    FILE          * pFile;
    struct tm     * newTime;
        
    pFile = fopen(strFileName, "w");
    fprintf(pFile, "%s\t\t%s\t\t%s\t\t%s\t%s\n", "Relative Time", "Seconds", 
        "Second Fraction", "Day/DMY", "File Offset");

    for( i=0; i<uiSize; i++ )
        {
        fprintf(pFile, "%llu\t\t", IndexTable[i].ui64RelTime);
        newTime = gmtime(&(IndexTable[i].suIrigTime.ulSecs));
        fprintf(pFile, "%d\t\t%d\t\t", IndexTable[i].suIrigTime.ulSecs, IndexTable[i].suIrigTime.ulFrac);
        if ( IndexTable[i].suIrigTime.enFmt==0 )
            fprintf(pFile, "%s\t", "Day");
        else
            fprintf(pFile, "%s\t", "DMY");
        fprintf(pFile, "%llu", IndexTable[i].ui64FileOffset);
        fprintf(pFile, "\t%s", asctime(newTime));
        }
    fprintf(pFile, "%s", strFileName);
    fclose(pFile);

    return iReturnValue;
    }


/* ----------------------------------------------------------------------- */

int I106_CALL_DECL ProcessRootIndexPackBody( int iHandle, SuI106Ch10Header suHeader )
    {
    int iReturnValue = 1;
    int iReadCnt;
    uint32_t u32RootIndexOffsetCounter = 0, i;
    // Index Intra-Packet Data Header bit
    uint32_t u32IIPDH;
    // Byte offset variables
    uint64_t u64RootIndPackBodyOffset, u64FileSize, u64BytesRead = 0, llNewPos, llOldPos;
    // File Size variable
    uint32_t au32FileSize[2], au32NodeOffset[2];
    // The Index Intra-Packet Time Stamp and Index Instra-Packet Data Header
    uint64_t au64IIPTS[1], au64IIPDH[1];
    // Channel Specific Data Word variable
    SuChannel_Spec* psuChannelSpec;
    psuChannelSpec = (SuChannel_Spec*)malloc(sizeof(SuChannel_Spec));

    enI106Ch10GetPos(iHandle, &u64RootIndPackBodyOffset);
    if ( (iReadCnt = read(g_suI106Handle[iHandle].iFile, psuChannelSpec, sizeof(SuChannel_Spec))) != 
        sizeof(SuChannel_Spec))
        {
        return I106_NO_MORE_DATA;
        }

    u64BytesRead += sizeof(SuChannel_Spec);
    enI106Ch10GetPos(iHandle, &llNewPos);
    
    u32IIPDH = psuChannelSpec->uiIPDH;
    if ( psuChannelSpec->uiIT || !psuChannelSpec->uiIndexEntry )// uiIT==Node Index, uiIndexEntry==0
        {
        iReturnValue = I106_READ_ERROR;
        }

    // Continue processing the Root Index Packet
    else
        {
        // Allocate memory to build the table holding all Node Index Offsets
        g_u64NodeIndexOffsetTable = (uint64_t*)malloc(psuChannelSpec->uiIndexEntry*sizeof(uint64_t));
        if (psuChannelSpec->uiFSP)
            {
            enI106Ch10GetPos(iHandle, &llOldPos);
            // Read 8-byte FILE SIZE data
            iReadCnt = read(g_suI106Handle[iHandle].iFile, &au32FileSize[0], FILE_SIZE);
            iReadCnt = read(g_suI106Handle[iHandle].iFile, &au32FileSize[1], FILE_SIZE);
            enI106Ch10GetPos(iHandle, &llNewPos);
            u64BytesRead += 2 * FILE_SIZE;
            u64FileSize = (au32FileSize[1] << 32) + au32FileSize[0];
            }

        // Read all index entries including the Root Index Offset
        while ( u32RootIndexOffsetCounter < psuChannelSpec->uiIndexEntry )
            {
            // Read the Index Intra-Packet Time Stamp
            iReadCnt = read(g_suI106Handle[iHandle].iFile, &au64IIPTS[0], IIP_TimeStamp);
            u64BytesRead += IIP_TimeStamp;
            //if ( u32IIPDH )// If the Index Intra-Packet Data Header is present
            //{
                // Read the Index Intra-Packet Data Header
                iReadCnt = read(g_suI106Handle[iHandle].iFile, &au64IIPDH[0], IIP_DataHeader);
                u64BytesRead += IIP_DataHeader;
            //}
            iReadCnt = read(g_suI106Handle[iHandle].iFile, &au32NodeOffset[0], sizeof(uint32_t));
            iReadCnt = read(g_suI106Handle[iHandle].iFile, &au32NodeOffset[1], sizeof(uint32_t));
            g_u64NodeIndexOffsetTable[u32RootIndexOffsetCounter] = (au32NodeOffset[1]<<32) + au32NodeOffset[0];
            u64BytesRead += 2 * sizeof(uint32_t);
            u32RootIndexOffsetCounter++;
            }

        for( i=0; i<u32RootIndexOffsetCounter-1; i++ )
            {
            ProcessNodeIndexPack(iHandle, g_u64NodeIndexOffsetTable[i]);
            }

        if ( g_u64NodeIndexOffsetTable )
            free( g_u64NodeIndexOffsetTable );
        } // end else

    if ( psuChannelSpec )
        free(psuChannelSpec);

    return iReturnValue;
    }



/* ----------------------------------------------------------------------- */

int I106_CALL_DECL ProcessNodeIndexPack(int iHandle, uint64_t u64NodeIndexOffset)
    {
    int                 iReturnValue = 1;
    // I106 Ch10 Header variable
    SuI106Ch10Header    suHeader;
    uint64_t            llNewPos;
    
    enI106Ch10SetPos(iHandle, u64NodeIndexOffset);
    if (read(g_suI106Handle[iHandle].iFile, &suHeader, HEADER_SIZE)!=HEADER_SIZE)
        {
        return I106_NO_MORE_DATA;
        }
    enI106Ch10GetPos(iHandle, &llNewPos);

    ProcessNodeIndexPackBody(iHandle, suHeader);
    
    return iReturnValue;
    }



/* ----------------------------------------------------------------------- */

int I106_CALL_DECL ProcessNodeIndexPackBody(int iHandle, SuI106Ch10Header suHeader)
    {
    int iReturnValue = 1;
    int iReadCnt;
    
    uint32_t            u32NodeEntryCounter = 0;// The number of Node Entries in this Node Index Packet
    uint32_t            u32IIPDH;               // Index Intra-Packet Data Header bit
    uint64_t            u64FileSize;            // Byte offset variables
    uint64_t            u64BytesRead = 0;
    uint64_t            llNewPos;
    uint64_t            llOldPos;
    uint64_t            u64NodeEntryOffset;
    uint64_t            u64NodeIndPackBodyOffset;
    uint64_t            u64NextNodeEntryOffset;
    uint32_t            au32FileSize[2];        // File Size variable
    uint64_t            au64IIPTS[1],           // The Index Intra-Packet Time Stamp and Index Instra-Packet Data Header 
    uint64_t            au64IIPDH[1];
    SuChannel_Spec    * psuChannelSpec;         // Channel Specific Data Word variable
    SuNodeIndexEntry  * psuNodeIndexEntry;      // Node Index Entry variable
    SuI106Ch10Header    suTimeDataHeader;
    void              * pvReadBuff;             // Reading buffer variables
    unsigned long       ulBuffSize = 0, 
    unsigned long       ulReadSize;

    // IRIG time and Relative Time
    SuIrig106Time       suIrigTime;
    uint64_t            u64RelTime;
    
    psuChannelSpec    = (SuChannel_Spec   *)malloc(sizeof(SuChannel_Spec));
    psuNodeIndexEntry = (SuNodeIndexEntry *)malloc(sizeof(SuNodeIndexEntry));

    enI106Ch10GetPos(iHandle, &u64NodeIndPackBodyOffset);

    // Read the Channel Specific Data of the recording node index packet
    if ( (iReadCnt = read(g_suI106Handle[iHandle].iFile, psuChannelSpec, sizeof(SuChannel_Spec))) != 
        sizeof(SuChannel_Spec))
        {
        return I106_NO_MORE_DATA;
        }

    u64BytesRead += sizeof(SuChannel_Spec);
    enI106Ch10GetPos(iHandle, &llNewPos);
    
    u32IIPDH = psuChannelSpec->uiIPDH;
    if ( !psuChannelSpec->uiIT || !psuChannelSpec->uiIndexEntry )// uiIT==Root Index, uiIndexEntry==0
        {
        iReturnValue = 0;
        }

    else// Continue processing the Node Index Packet
        {
        // Read the FILE SIZE data if it is present
        //g_u64NodeIndexOffsetTable = (uint64_t*)malloc(psuChannelSpec->uiIndexEntry*sizeof(uint64_t));
        if (psuChannelSpec->uiFSP)
            {
            enI106Ch10GetPos(iHandle, &llOldPos);
            // Read 8-byte FILE SIZE data
            read(g_suI106Handle[iHandle].iFile, &au32FileSize[0], FILE_SIZE);
            read(g_suI106Handle[iHandle].iFile, &au32FileSize[1], FILE_SIZE);
            //enI106Ch10GetPos(iHandle, &llNewPos);
            u64BytesRead += 2 * FILE_SIZE;
            u64FileSize = (au32FileSize[1] << 32) + au32FileSize[0];
            }

        // Read all node entries
        while ( u32NodeEntryCounter < psuChannelSpec->uiIndexEntry )
            {
            // Read the Intra-Packet Time Stamp of the current node index in the node index packet
            read(g_suI106Handle[iHandle].iFile, &au64IIPTS[0], IIP_TimeStamp);
            u64BytesRead += IIP_TimeStamp;
            //if ( u32IIPDH )// If the Intra-Packet Data Header of the recording node index packet is present
            //{
                // Read the Intra-Packet Data Header of the current node index in the node index packet
                read(g_suI106Handle[iHandle].iFile, &au64IIPDH[0], IIP_DataHeader);
                u64BytesRead += IIP_DataHeader;
            //}
            // Read the Recording Node Index of the node index 'u32NodeEntryCounter'
            read(g_suI106Handle[iHandle].iFile, psuNodeIndexEntry, sizeof(SuNodeIndexEntry));
            // Get the file offset of the next Recording Node Index
            enI106Ch10GetPos(iHandle, &u64NextNodeEntryOffset);

            u64BytesRead += sizeof(SuNodeIndexEntry);
            u32NodeEntryCounter++;
            // Get the file offset of the current Recording Node Index
            u64NodeEntryOffset = psuNodeIndexEntry->u64Offset;

            // Read the packet header of the Node Entry at the file offset 'u64NodeEntryOffset'
            enI106Ch10SetPos(iHandle, u64NodeEntryOffset);
            g_suI106Handle[iHandle].enFileState = enReadData;
            if (read(g_suI106Handle[iHandle].iFile, &suTimeDataHeader, HEADER_SIZE)!=HEADER_SIZE)
                {
                return I106_NO_MORE_DATA;
                }

            else
                {
                // Process if the Node Entry is a Time Data Packet
                if ( suTimeDataHeader.ubyDataType==0x11 )
                    {
                    if ( ulBuffSize < suHeader.ulPacketLen )
                        {
                        pvReadBuff = (void*)malloc(suTimeDataHeader.ulPacketLen);
                        ulBuffSize = suTimeDataHeader.ulPacketLen;
                        }
                    // Read the data buffer (ulBuffSize-HEADER_SIZE) since the header was already read above
                    ulReadSize = read(g_suI106Handle[iHandle].iFile, pvReadBuff, ulBuffSize-HEADER_SIZE);
                    if (ulReadSize != (ulBuffSize-HEADER_SIZE))
                        {
                        iReturnValue = 0;
                        break;
                        }
                    // Get the IRIG time and the Relative time
                    enI106_Decode_TimeF1(&suTimeDataHeader, pvReadBuff, &suIrigTime);
                    u64RelTime = (suTimeDataHeader.aubyRefTime[5]<<40) + (suTimeDataHeader.aubyRefTime[4]<<32) +
                        (suTimeDataHeader.aubyRefTime[3]<<24) + (suTimeDataHeader.aubyRefTime[2]<<16) +
                        (suTimeDataHeader.aubyRefTime[1]<<8) + suTimeDataHeader.aubyRefTime[0];
                    if ( uiSize==uiCapacity )
                        ReallocIndexTable();
                    IndexTable[uiSize].ui64RelTime = u64RelTime;
                    memcpy(&(IndexTable[uiSize].suIrigTime), &suIrigTime, sizeof(SuIrig106Time));
                    IndexTable[uiSize].ui64FileOffset = u64NodeEntryOffset;
                    uiSize++;
                    } // end if time data packet
                } // end else still reading
            // Set the file pointer to read the next Recording Node Index
            enI106Ch10SetPos(iHandle, u64NextNodeEntryOffset);
            } // end while reading all node entries
        } // end process Node Index Packet

    if ( u64BytesRead!=suHeader.ulDataLen )
        {
        iReturnValue = 0;
        printf("The Node Index Packet data body read incorrectly\n");
        }

    if ( psuChannelSpec )
        free(psuChannelSpec);

    if ( psuNodeIndexEntry )
        free(psuNodeIndexEntry);

    return iReturnValue;
    }



/* ----------------------------------------------------------------------- */

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
* Initialize index table data for the first time
*/

void InitIndexes(void)
    {
    int     iHandleIdx;

    for (iHandleIdx=0; iHandleIdx<MAX_HANDLES; iHandleIdx++)
        {
        uNodesUsed[iHandleIdx]      = 0L;
        uNodesAvailable[iHandleIdx] = 0L;
        IndexTable[iHandleIdx]      = NULL;
        } 

    return;
    }

#ifdef __cplusplus
}
#endif
