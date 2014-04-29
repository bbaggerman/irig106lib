/****************************************************************************

 i106_decode_pcmf1.c -

 Copyright (c) 2014 Irig106.org

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

 Author: Hans-Gerhard Flohr, Hasotec GmbH, www.hasotec.de
 2014/04/07 Initial Version 1.0
 2014/04/23 Version 1.1 
 Changes:   Inversing meaning of swap data bytes / words
            Correcting llintpkttime calculation if a new packet was received

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
#include "i106_time.h"
#include "i106_decode_tmats.h"
#include "i106_decode_pcmf1.h"

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

    // For test with the data of a known (special) file 
    #ifdef _DEBUG
        //#define DEBUG_OTHER_PCM_FILE
        #ifdef DEBUG_OTHER_PCM_FILE
            static FILE *FilePcmTest = NULL;
        #endif
    #endif


/*
 * Function Declaration
 * --------------------
 */

// Local functions
EnI106Status PrepareNextDecodingRun_PcmF1(SuPcmF1_CurrMsg * psuMsg);
void PrepareNewMinorFrameCollection_PcmF1(SuPcmF1_Attributes * psuAttributes);
void GetNextBit_PcmF1(SuPcmF1_CurrMsg * psuMsg, SuPcmF1_Attributes * psuAttributes);
int IsSyncWordFound_PcmF1(SuPcmF1_Attributes * psuAttributes);
void RenewSyncCounters_PcmF1(SuPcmF1_Attributes * psuAttributes, uint64_t ullSyncCount);



/* ======================================================================= */

// Note; This code is tested only with Pcm in throughput mode

/* ----------------------------------------------------------------------- */



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstPcmF1(SuI106Ch10Header     * psuHeader,
    void            * pvBuff,
    SuPcmF1_CurrMsg * psuMsg)
{
    uint32_t ulSubPacketLen;
    uint32_t uRemainder;

    // Check for attributes available
    if(psuMsg->psuAttributes == NULL)
        return I106_UNSUPPORTED;

    // Set pointers to the beginning of the Pcm buffer
    psuMsg->psuHeader = psuHeader; 
    psuMsg->psuChanSpec = (SuPcmF1_ChanSpec *)pvBuff; 

    psuMsg->uBytesRead = 0;
    psuMsg->ulDataLen = psuHeader->ulDataLen;
    psuMsg->uBytesRead += sizeof(SuPcmF1_ChanSpec);

    // Check for no (more) data
    if (psuMsg->ulDataLen <= psuMsg->uBytesRead)
        return I106_NO_MORE_DATA;

    // Save the time from the packet header
    vTimeArray2LLInt(psuHeader->aubyRefTime, &(psuMsg->llBaseIntPktTime));

    // Some precalculations inclusive time
    if(psuMsg->psuChanSpec->bThruMode)
    {
        // Throughput mode, no intra packet header present
        // -----------------------------------------------
        psuMsg->psuIntraPktHdr = NULL;

        // Take the whole remaining data buffer as packet len
        psuMsg->ulSubPacketLen = psuMsg->ulDataLen - psuMsg->uBytesRead;

        // The IntPktTime is recalculated later from the bit position
        psuMsg->llIntPktTime = psuMsg->llBaseIntPktTime;
    }
    else
    {
        // Not troughput mode, an intra packet header must be present
        // NOTE: UNTESTED
        // ----------------------------------------------------------

        psuMsg->psuIntraPktHdr = (SuPcmF1_IntraPktHeader *) ((char *)(psuMsg->psuChanSpec) + psuMsg->uBytesRead);
        psuMsg->uBytesRead += sizeof(SuPcmF1_IntraPktHeader);

        // If there is no space for data
        if (psuMsg->ulDataLen <= psuMsg->uBytesRead)
            return I106_NO_MORE_DATA;

        // Compute the padded framelen for the minor frame
        ulSubPacketLen = psuMsg->psuAttributes->ulBitsInMinorFrame;

        if( ! psuMsg->psuChanSpec->bAlignment) // 16 bit alignement
        {
            uRemainder = ulSubPacketLen & 0xf; // %16
            ulSubPacketLen >>= 4; // /= 16;

            if(uRemainder)
                ulSubPacketLen += 1;
            ulSubPacketLen <<= 1; // * 2
        }
        else // 32 bit alignement
        {
            uRemainder = ulSubPacketLen & 0x1f; // % 32
            ulSubPacketLen >>= 5; // / 32

            if(uRemainder)
                ulSubPacketLen += 1;
            ulSubPacketLen <<= 2; // * 4
        }
        psuMsg->ulSubPacketLen = ulSubPacketLen; 

        // Fetch the time from the intra packet header
        vFillInTimeStruct(psuHeader, (SuIntraPacketTS *)psuMsg->psuIntraPktHdr, &psuMsg->suTimeRef);
        // and publish it   
        psuMsg->llIntPktTime = psuMsg->suTimeRef.uRelTime;

    }

    // We continue with the throughput mode
    // ------------------------------------

    // Check for the amount of the remaining data including the length of the data
    if(psuMsg->ulDataLen < psuMsg->uBytesRead + psuMsg->ulSubPacketLen)
        return I106_NO_MORE_DATA;

    // Set the pointer to the Pcm message data
    psuMsg->pauData = (uint8_t *)((char *)(psuMsg->psuChanSpec) + psuMsg->uBytesRead);

    //for(int count= 0; count < 128; count++)
    //  TRACE(" %2.2X", psuMsg->pauData[count]);
    //TRACE("\n");

    psuMsg->uBytesRead += psuMsg->ulSubPacketLen;

    // For troughput mode
    psuMsg->ulSubPacketBits = psuMsg->ulSubPacketLen * 8;

    // Prepare the Pcm buffers and load the first bits
    if(psuMsg->psuAttributes->bPrepareNextDecodingRun)
    {

        #ifdef DEBUG_OTHER_PCM_FILE
        if(FilePcmTest == NULL)
        {
            FilePcmTest = fopen("d:\\projects\\winspo\\winspoca\\F1249\\PC101039.ftr", "rb");
            if(FilePcmTest == NULL)
                return I106_NO_MORE_DATA;
            fseek(FilePcmTest, 0x64, SEEK_SET);
        }
        #endif

        // Set up the data
        EnI106Status enStatus = PrepareNextDecodingRun_PcmF1(psuMsg);
        if(enStatus != I106_OK)
            return enStatus;
    }

    if( ! psuMsg->psuChanSpec->bThruMode)
    {
        // Intra-packet not tested, so return
        return I106_UNSUPPORTED;
    }

    if(psuMsg->psuChanSpec->bThruMode)
    {

        #ifdef DEBUG_OTHER_PCM_FILE
        int nBytes;
        if(FilePcmTest == NULL)
            return I106_NO_MORE_DATA;
        // Note: skip the datarec3 header, otherwise we will have about 45 sync errore
        fread(psuMsg->pauData, 1, 6, FilePcmTest); // skip the datarec3 header
        nBytes = fread(psuMsg->pauData, 1, psuMsg->ulSubPacketLen, FilePcmTest);
        //TRACE("nBytes %d, SubPacketLen %d, filepos %d\n",nBytes, psuMsg->ulSubPacketLen, ftell(FilePcmTest));
        //Sleep(200);
        if(nBytes < (int32_t)psuMsg->ulSubPacketLen)
        {
            fclose(FilePcmTest);
            FilePcmTest = NULL;
            return I106_NO_MORE_DATA;
        }
        #endif

        if( ! psuMsg->psuAttributes->bDontSwapRawData)
        {
            if(SwapBytes_PcmF1(psuMsg->pauData, psuMsg->ulSubPacketLen))
                return(I106_INVALID_DATA); 
            // Note: Untested 
            if(psuMsg->psuChanSpec->bAlignment)
            {
                if(SwapShortWords_PcmF1((uint16_t *)psuMsg->pauData, psuMsg->ulSubPacketLen))
                return(I106_INVALID_DATA); 
            }
        }

        // Now start the decode of this buffer
        psuMsg->psuAttributes->ulBitPosition = 0;

        return (DecodeMinorFrame_PcmF1(psuMsg));
    }

    return I106_OK;
}


/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextPcmF1(SuPcmF1_CurrMsg * psuMsg)
{

    if(psuMsg->psuChanSpec->bThruMode)
    {
        return (DecodeMinorFrame_PcmF1(psuMsg));
    }

    // Check for no (more) data
    if (psuMsg->ulDataLen < psuMsg->uBytesRead)
        return I106_NO_MORE_DATA;
    
    // If not thru mode, we must have an intrapacket header
    // NOTE: UNTESTED
    // May be, it points to outside ...
    psuMsg->psuIntraPktHdr = (SuPcmF1_IntraPktHeader *) ((char *)(psuMsg->psuChanSpec) + psuMsg->uBytesRead);
    psuMsg->uBytesRead += sizeof(SuPcmF1_IntraPktHeader);
    // ... so check, if it was successful
    if (psuMsg->ulDataLen <= psuMsg->uBytesRead)
        return I106_NO_MORE_DATA;
            
    // TODO: Check time stamp, alignment, compute the the sub packet len etc
            
    // Fetch the time from the intra packet header
    vFillInTimeStruct(psuMsg->psuHeader, (SuIntraPacketTS *)psuMsg->psuIntraPktHdr, &psuMsg->suTimeRef);
    // and publish it   
    psuMsg->llIntPktTime = psuMsg->suTimeRef.uRelTime;

  // Check for no more data (including the length of the minor frame)
  if(psuMsg->ulDataLen < psuMsg->uBytesRead + psuMsg->ulSubPacketLen)
      return I106_NO_MORE_DATA;
        
  // Set the pointer to the Pcm message data
  psuMsg->pauData = (uint8_t *)((char *)(psuMsg->psuChanSpec) + psuMsg->uBytesRead);

  psuMsg->uBytesRead += psuMsg->ulSubPacketLen;

  return I106_OK;

}


/* ----------------------------------------------------------------------- */

// Fill the attributes from TMATS 
// ToDo: Check if all needed definitions found
EnI106Status I106_CALL_DECL Set_Attributes_PcmF1(SuRDataSource * psuRDataSrc, SuPcmF1_Attributes * psuPcmF1_Attributes)
{

    SuPRecord           * psuPRecord;
    uint32_t            uBitCount;

    if(psuPcmF1_Attributes == NULL) return I106_INVALID_PARAMETER; // Set Attributes

    memset(psuPcmF1_Attributes, 0, sizeof(SuPcmF1_Attributes));
    psuPRecord = psuRDataSrc->psuPRecord;

    if(psuPRecord == NULL) return I106_INVALID_PARAMETER; // Set Attributes

    // Collect the TMATS values
    // ------------------------
    // Essential values for throughput mode are: 
    //      szBitsPerSec / ulBitsPerSec // P-x\D2
    //      szCommonWordLen / ulCommonWordLen // P-x\F1
    //      szWordsInMinorFrame / ulWordsInMinorFrame // P-x\MF1
    //      szBitsInMinorFrame / ulBitsInMinorFrame // P-x\MF2
    //          if not existent, it is calculated from  ulCommonWordLen * (ulWordsInMinorFrame - 1) + ulMinorFrameSyncPatLen;
    //          Note: May be over-determined, no check for this
    //      szMinorFrameSyncPatLen / ulMinorFrameSyncPatLen // P-x\MF4
    //          if not existent, it is calculated from the number of bits in ulMinorFrameSyncPat
    //          Note: May be over-determined, no check for this
    //      szMinorFrameSyncPat / ulMinorFrameSyncPa) // P-x\MF5

    // Default values for throughput are taken for  
    //      szWordTransferOrder: "M" P-x\F2
    //      szParityType: "NO" P-x\F3
    //      szParityTransferOrder not "L" P-x/F4
    //      szMinorFrameSyncType P-x\MF3: "FPT"
    // Unneeded values for throughput are

    //      iRecordNum / iRecordNum // P-x
    //      szNumMinorFrames / ulNumMinorFrames  P-x\MF\N

    psuPcmF1_Attributes->psuRDataSrc                = psuRDataSrc; // May be, we need it in the future

    psuPcmF1_Attributes->iRecordNum                 = psuPRecord->iRecordNum; // P-x

    if(psuPRecord->szBitsPerSec != NULL)
        psuPcmF1_Attributes->ulBitsPerSec           = atol(psuPRecord->szBitsPerSec); // P-x\D2
    if(psuPRecord->szCommonWordLen != NULL)
        psuPcmF1_Attributes->ulCommonWordLen        = atol(psuPRecord->szCommonWordLen); // P-x\F1

    if(psuPRecord->szWordTransferOrder != NULL)    // P-x\F2 most significant bit "M", least significant bit "L". default: M
    {
        /*
        Measurement Transfer Order. Which bit is being transferred first is specified as – Most Significant Bit (M), 
        Least Significant Bit (L), or Default (D). The default is specified in the P-Group - (P-x\F2:M).
        D-1\MN3-1-1:M;
        */
        if(psuPRecord->szWordTransferOrder[0] == 'L')
        {
            psuPcmF1_Attributes->ulWordTransferOrder = PCM_LSB_FIRST;
            return(I106_UNSUPPORTED);
        }
    }
    if(psuPRecord->szParityType != NULL)  // P-x/F3
    {
        //even "EV", odd "OD", or none "NO", default: none
        if (strncasecmp(psuPRecord->szParityType, "EV", 2) == 0) 
            psuPcmF1_Attributes->ulParityType = PCM_PARITY_EVEN;
        else if (strncasecmp(psuPRecord->szParityType, "OD", 2) == 0) 
            psuPcmF1_Attributes->ulParityType = PCM_PARITY_EVEN; 
        else
            psuPcmF1_Attributes->ulParityType = PCM_PARITY_NONE;
    }
    if(psuPRecord->szParityTransferOrder != NULL)
    {
        if (strncasecmp(psuPRecord->szParityType, "L", 1) == 0)    // P-x/F4
            psuPcmF1_Attributes->ulParityTransferOrder = 1;
        else
            psuPcmF1_Attributes->ulParityTransferOrder = 0;
    }
    if(psuPRecord->szNumMinorFrames != NULL)
        psuPcmF1_Attributes->ulNumMinorFrames       = atol(psuPRecord->szNumMinorFrames); // P-x\MF\N

    if(psuPRecord->szWordsInMinorFrame != NULL)
        psuPcmF1_Attributes->ulWordsInMinorFrame    = atol(psuPRecord->szWordsInMinorFrame); // P-x\MF1

    if(psuPRecord->szBitsInMinorFrame != NULL)
        psuPcmF1_Attributes->ulBitsInMinorFrame     = atol(psuPRecord->szBitsInMinorFrame); // P-x\MF2

    if(psuPRecord->szMinorFrameSyncType != NULL)
    {
        // if not "FPT" : Error
        if (strncasecmp(psuPRecord->szMinorFrameSyncType, "FPT", 3) != 0) // P-x\MF3
            return(I106_UNSUPPORTED);
        psuPcmF1_Attributes->ulMinorFrameSyncType = 0;
    }

    if(psuPRecord->szMinorFrameSyncPatLen != NULL)
        psuPcmF1_Attributes->ulMinorFrameSyncPatLen = atol(psuPRecord->szMinorFrameSyncPatLen); // P-x\MF4

    if(psuPRecord->szInSyncCrit != NULL)
    {
        // to declare that the system is in sync – First good sync (0), Check (1 or greater), or Not specified (NS).
        psuPcmF1_Attributes->ulMinSyncs = 0; // Minimal number of syncs P-x\SYNC1;
    }
        
    if(psuPRecord->szMinorFrameSyncPat != NULL) // P-x\MF5
    {
        uint64_t ullSyncPat = 0;
        uint64_t ullSyncMask = 0;
        uint32_t ulMinorFrameSyncPatLen = 0;
        char *pChar = psuPRecord->szMinorFrameSyncPat;
        //Example: 0xFE6B2840
        //static char *xx = "11111110011010110010100001000000";
        //pChar = xx;
            
        // Skip leading blanks
        while(*pChar == ' ')
            pChar++;
        // Transfer the sync bits
        while((*pChar == '0') || (*pChar == '1'))
        {
            ulMinorFrameSyncPatLen++;
            ullSyncMask <<= 1;
            ullSyncMask |= 1;
            
            ullSyncPat <<= 1;
            if(*pChar == '1') 
                ullSyncPat |= 1;
            pChar++;
        }
        psuPcmF1_Attributes->ullMinorFrameSyncPat = ullSyncPat;
        psuPcmF1_Attributes->ullMinorFrameSyncMask = ullSyncMask;
        if(psuPcmF1_Attributes->ulMinorFrameSyncPatLen == 0)
            psuPcmF1_Attributes->ulMinorFrameSyncPatLen = ulMinorFrameSyncPatLen;
    } // minor frame sync pat
        
    // Some post processing
    if(psuPcmF1_Attributes->ulBitsInMinorFrame == 0)
    {
        psuPcmF1_Attributes->ulBitsInMinorFrame = psuPcmF1_Attributes->ulCommonWordLen * (psuPcmF1_Attributes->ulWordsInMinorFrame - 1) +
            psuPcmF1_Attributes->ulMinorFrameSyncPatLen;
    }
    for(uBitCount = 0; uBitCount < psuPcmF1_Attributes->ulCommonWordLen; uBitCount++)
    {
        psuPcmF1_Attributes->ullCommonWordMask <<= 1;
        psuPcmF1_Attributes->ullCommonWordMask |= 1;
    }
        
    psuPcmF1_Attributes->dDelta100NanoSeconds = d100NANOSECONDS / psuPcmF1_Attributes->ulBitsPerSec;
        
    psuPcmF1_Attributes->bPrepareNextDecodingRun = 1; // Set_Attributes_PcmF1
        
    return I106_OK;
} // End Set_Attributes _PcmF1

/* ----------------------------------------------------------------------- */
// Fill the attributes from an external source
// Replace the correspondent TMATS values, if the argument value is >= 0
EnI106Status I106_CALL_DECL 
    Set_Attributes_Ext_PcmF1(SuRDataSource * psuRDataSrc, SuPcmF1_Attributes * psuPcmF1_Attributes,
    //      P-x                 P-x\D2               P-x\F1                   P-x\F2
    int32_t lRecordNum, int32_t lBitsPerSec, int32_t lCommonWordLen, int32_t lWordTransferOrder,
    //       P-x\F3               P-x\F4
    int32_t lParityType, int32_t lParityTransferOrder,
    //      P-x\MF\N                 P-x\MF1                     P-x\MF2            P-x\MF3
    int32_t lNumMinorFrames, int32_t lWordsInMinorFrame, int32_t lBitsInMinorFrame, int32_t lMinorFrameSyncType,
    //      P-x\MF4                        P-x\MF5                      P-x\SYNC1 
    int32_t lMinorFrameSyncPatLen, int64_t llMinorFrameSyncPat, int32_t lMinSyncs, 
    //      External                      External
    int64_t llMinorFrameSyncMask, int32_t lNoByteSwap)
{
    uint32_t BitCount;
    if(psuRDataSrc == NULL) return I106_INVALID_PARAMETER; // Set Attributes Ext

    if(psuPcmF1_Attributes == NULL) return I106_INVALID_PARAMETER; // Set Attributes Ext

    // Transfer the external data
    if(lRecordNum != -1)
        psuPcmF1_Attributes->iRecordNum = lRecordNum;
    if(lBitsPerSec != -1)
        psuPcmF1_Attributes->ulBitsPerSec = lBitsPerSec;
    if(lCommonWordLen != -1)
        psuPcmF1_Attributes->ulCommonWordLen = lCommonWordLen;
    if(lWordTransferOrder != -1)
        psuPcmF1_Attributes->ulWordTransferOrder = lWordTransferOrder;
    if(lParityType != -1)
        psuPcmF1_Attributes->ulParityType = lParityType;
    if(lParityTransferOrder != -1)
        psuPcmF1_Attributes->ulParityTransferOrder = lParityTransferOrder;
    if(lNumMinorFrames != -1)
        psuPcmF1_Attributes->ulNumMinorFrames = lNumMinorFrames;
    if(lWordsInMinorFrame != -1)
        psuPcmF1_Attributes->ulWordsInMinorFrame = lWordsInMinorFrame;
    if(lBitsInMinorFrame != -1)
        psuPcmF1_Attributes->ulBitsInMinorFrame = lBitsInMinorFrame;
    if(lMinorFrameSyncType != -1)
        psuPcmF1_Attributes->ulMinorFrameSyncType = lMinorFrameSyncType;
    if(lMinorFrameSyncPatLen != -1)
        psuPcmF1_Attributes->ulMinorFrameSyncPatLen = lMinorFrameSyncPatLen;
    if(llMinorFrameSyncPat != -1)
        psuPcmF1_Attributes->ullMinorFrameSyncPat = llMinorFrameSyncPat;
    if(llMinorFrameSyncMask != -1)
        psuPcmF1_Attributes->ullMinorFrameSyncMask = llMinorFrameSyncMask;
    if(lMinSyncs != -1)
        psuPcmF1_Attributes->ulMinSyncs = lMinSyncs;
    if(lNoByteSwap != -1)
        psuPcmF1_Attributes->bDontSwapRawData = lNoByteSwap;

    psuPcmF1_Attributes->ullCommonWordMask = 0;
    for(BitCount = 0; BitCount < psuPcmF1_Attributes->ulCommonWordLen; BitCount++)
    {
         psuPcmF1_Attributes->ullCommonWordMask <<= 1;
         psuPcmF1_Attributes->ullCommonWordMask |= 1;
    }

    psuPcmF1_Attributes->ullCommonWordMask &= psuPcmF1_Attributes->ullMinorFrameSyncMask;

    psuPcmF1_Attributes->dDelta100NanoSeconds = d100NANOSECONDS / psuPcmF1_Attributes->ulBitsPerSec;

    psuPcmF1_Attributes->bPrepareNextDecodingRun = 1; // Set_Attributes_Ext_PcmF1

  return I106_OK;

} // End Set_Attributes_Ext_ PcmF1

/* ----------------------------------------------------------------------- */

// Create the output buffers for a minor frame (data and error flags)
EnI106Status I106_CALL_DECL 
    CreateOutputBuffers_PcmF1(SuPcmF1_Attributes * psuAttributes)
{

    // Allocate the Pcm output buffer for a minor frame
    psuAttributes->ulOutBufSize = psuAttributes->ulWordsInMinorFrame;
    psuAttributes->paullOutBuf = (uint64_t *)calloc(sizeof(uint64_t), psuAttributes->ulOutBufSize);
    if(psuAttributes->paullOutBuf == NULL)
        return I106_BUFFER_TOO_SMALL;
    
    psuAttributes->pauOutBufErr = (uint8_t *)calloc(sizeof(uint8_t), psuAttributes->ulOutBufSize);
    if(psuAttributes->pauOutBufErr == NULL)
    {
        free(psuAttributes->paullOutBuf); psuAttributes->paullOutBuf = NULL;
        return I106_BUFFER_TOO_SMALL;
    }
    return(I106_OK);
} // End CreateOutputBuffers

/* ----------------------------------------------------------------------- */

// Free the output buffers for a minor frame
EnI106Status I106_CALL_DECL FreeOutputBuffers_PcmF1(SuPcmF1_Attributes * psuPcmAttributes)
{

    if(psuPcmAttributes->paullOutBuf)
    {
        free(psuPcmAttributes->paullOutBuf);
        psuPcmAttributes->paullOutBuf = NULL;
    }
    if(psuPcmAttributes->pauOutBufErr)
    {
        free(psuPcmAttributes->pauOutBufErr);
        psuPcmAttributes->pauOutBufErr = NULL;
    }
    psuPcmAttributes->bPrepareNextDecodingRun = 1; 

    return(I106_OK);
} // End FreeOutputBuffers

/* ----------------------------------------------------------------------- */

// Prepare a new decoding run 
// Creates the output buffers and resets values and counters
EnI106Status PrepareNextDecodingRun_PcmF1(SuPcmF1_CurrMsg * psuMsg)
{
    SuPcmF1_Attributes * psuAttributes = psuMsg->psuAttributes;

    EnI106Status enStatus = CreateOutputBuffers_PcmF1(psuAttributes);
    if(enStatus != I106_OK)
        return(enStatus);

    psuAttributes->bPrepareNextDecodingRun = 0;
    
    // If not throughput mode, the work is done 
    // ----------------------------------------
    if( ! psuMsg->psuChanSpec->bThruMode)
        return I106_OK;

    // Prepare the variables for bit decoding in throughput mode
    // --------------------------------------------------------
    psuAttributes->ullSyncCount = -1; // -1 sets all bits to 1
    psuAttributes->ullSyncErrors = 0;
    psuAttributes->ullTestWord = 0; 
    psuAttributes->ulBitPosition = 0; 
    psuAttributes->ullBitsLoaded = 0;
    // Nearly the same as in RenewSyncCounter...
    psuAttributes->ulMinorFrameBitCount = 0;
    psuAttributes->ulMinorFrameWordCount = 0;
    psuAttributes->ulDataWordBitCount = 0;
    psuAttributes->lSaveData = 0;

    return I106_OK;

} // End PrepareNextDecodingRun

/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    DecodeMinorFrame_PcmF1(SuPcmF1_CurrMsg * psuMsg)
{

    SuPcmF1_Attributes * psuAttributes = psuMsg->psuAttributes;

    while(psuAttributes->ulBitPosition < psuMsg->ulSubPacketBits)
    {

        GetNextBit_PcmF1(psuMsg, psuAttributes);

        // Check for a sync word

        if(IsSyncWordFound_PcmF1(psuAttributes))
        {   
            // Prevent an overflow after a terabyte of bits
            if(psuAttributes->ullBitsLoaded > 1000000000000)
                psuAttributes->ullBitsLoaded = 1000000000;

            psuAttributes->ullSyncCount++;

            //TRACE("Sync word found at BitPos %6d, MFBitCnt %5d, 0x%08X, SyncCnt %6d\n", 
            //  psuAttributes->ulBitPosition, psuAttributes->ulMinorFrameBitCount, (int32_t)psuAttributes->ullTestWord, psuAttributes->ullSyncCount);

            if(psuAttributes->ulMinorFrameBitCount == psuAttributes->ulBitsInMinorFrame)
            {
                // A sync word found at the correct offset to the previous one

                RenewSyncCounters_PcmF1(psuAttributes, psuAttributes->ullSyncCount); // with the current sync counter
                
                // If there are enough syncs, release the previous filled outbuf
                // Note: a minor frame is released only, if it is followed by a sync word at the correct offset. 
                // i.e. the sync word are used as brackets
                if((psuAttributes->ullSyncCount >= psuAttributes->ulMinSyncs) && (psuAttributes->lSaveData > 1)) 
                {

                    // Compute the intrapacket time of the start sync bit position in the current buffer
                    int64_t llBitPosition = (int64_t)psuAttributes->ulBitPosition - (int64_t)psuAttributes->ulBitsInMinorFrame /*- (int64_t)psuAttributes->ulMinorFrameSyncPatLen*/;

                    double dOffsetIntPktTime = (double)llBitPosition * psuAttributes->dDelta100NanoSeconds;   

                    psuMsg->llIntPktTime = psuMsg->llBaseIntPktTime + (int64_t)dOffsetIntPktTime; // Relative time, omit rounding

                    // Prepare for the next run
                    PrepareNewMinorFrameCollection_PcmF1(psuAttributes);
                    return I106_OK;

                }

            }
            else
            {
                // A sync word at the wrong offset, throw away all
                // Note: a wrong offset is also at the first sync in the whole decoding run

                // Save the sync error for statistics
                if(psuAttributes->ullSyncCount > 0)
                    psuAttributes->ullSyncErrors++;

                // RenewSyncCounters_PcmF1 with a sync counter of zero
                RenewSyncCounters_PcmF1(psuAttributes, 0);
            }

            PrepareNewMinorFrameCollection_PcmF1(psuAttributes);
            continue;

        } // if sync found

        // Collect the data

        if(psuAttributes->lSaveData == 1)
        {
            psuAttributes->ulDataWordBitCount++;
            if(psuAttributes->ulDataWordBitCount >= psuAttributes->ulCommonWordLen)
            {
                psuAttributes->paullOutBuf[psuAttributes->ulMinorFrameWordCount - 1] = psuAttributes->ullTestWord;
                psuAttributes->ulDataWordBitCount = 0;
                //TRACE("MFWC %d 0x%I64x\n", psuAttributes->ulMinorFrameWordCount - 1, psuAttributes->paullOutBuf[psuAttributes->ulMinorFrameWordCount - 1]);
                psuAttributes->ulMinorFrameWordCount++;
            }
        }
        if(psuAttributes->ulMinorFrameWordCount >= psuAttributes->ulWordsInMinorFrame)
        {
            psuAttributes->lSaveData = 2;

            // Don't release the data here but wait for a trailing sync word. 
        }
    } // end while

    // Preset for the next run
    psuAttributes->ulBitPosition = 0;

  return I106_NO_MORE_DATA;
}

/* ----------------------------------------------------------------------- */
// Prepare a new minor frame collection
 void PrepareNewMinorFrameCollection_PcmF1(SuPcmF1_Attributes * psuAttributes)
{
    psuAttributes->ulDataWordBitCount = 0;
    psuAttributes->lSaveData = 1;
}


/* ----------------------------------------------------------------------- */
// Get the next bit
void GetNextBit_PcmF1(SuPcmF1_CurrMsg * psuMsg, SuPcmF1_Attributes * psuAttributes)
{
    psuAttributes->ullTestWord <<= 1;
    //TRACE("%d\n", psuAttributes->ulBitPosition);
    if(IsBitSetL2R(psuMsg->pauData, psuAttributes->ulBitPosition))
    {
        psuAttributes->ullTestWord |= 1;
    }
    psuAttributes->ullBitsLoaded++;
    psuAttributes->ulMinorFrameBitCount++;
    psuAttributes->ulBitPosition++;

}

/* ----------------------------------------------------------------------- */
// Check for a sync word
int IsSyncWordFound_PcmF1(SuPcmF1_Attributes * psuAttributes)
{
    return((psuAttributes->ullBitsLoaded >= psuAttributes->ulMinorFrameSyncPatLen) && 
                (psuAttributes->ullTestWord & psuAttributes->ullMinorFrameSyncMask) == psuAttributes->ullMinorFrameSyncPat);
}

/* ----------------------------------------------------------------------- */
// RenewSyncCounters_PcmF1
void RenewSyncCounters_PcmF1(SuPcmF1_Attributes * psuAttributes, uint64_t ullSyncCount)
{
    psuAttributes->ulMinorFrameBitCount = 0; 
    psuAttributes->ulMinorFrameWordCount = 1; // Note the 1: this is the sync word
    psuAttributes->ulDataWordBitCount = 0;
    psuAttributes->ullSyncCount = ullSyncCount;
}

/* ----------------------------------------------------------------------- */
// Returns I106_OK on success, I106_INVALID_DATA on error
EnI106Status I106_CALL_DECL
    CheckParity_PcmF1(uint64_t ullTestWord, int iWordLen, int iParityType, int iParityTransferOrder)
          // check the parity of a word
{
    uint64_t ullTestBit = 1;
    unsigned int uBitSum = 0;

    switch(iParityType)
    {
    case PCM_PARITY_NONE:
        break;
    case PCM_PARITY_EVEN:
        while(iWordLen-- > 0)
        {
            if(ullTestWord & ullTestBit) uBitSum++;
            ullTestBit <<= 1;
        }
        if(uBitSum & 1) return(I106_INVALID_DATA);
        break;
    case PCM_PARITY_ODD:
        while(iWordLen-- > 0)
        {
            if(ullTestWord & ullTestBit) uBitSum++;
            ullTestBit <<= 1;
        }
        if( ! (uBitSum & 1)) return(I106_INVALID_DATA);
        break;
    default: // none
        break;
    }
    return(I106_OK);
}

/* ----------------------------------------------------------------------- */
// Swaps nBytes in place
EnI106Status I106_CALL_DECL SwapBytes_PcmF1(uint8_t *pubBuffer, long nBytes)
{
    uint32_t idata = 0x03020100;
    uint8_t ubTemp;
    if(nBytes & 1)
        return(I106_BUFFER_OVERRUN); // May be also an underrun ...
    while((nBytes -= 2) >= 0)
    {
        ubTemp = *pubBuffer;
        *pubBuffer = *(pubBuffer + 1);
        *++pubBuffer = ubTemp;
        pubBuffer++;
    }
    SwapShortWords_PcmF1((uint16_t *)&idata, 4);

    return(I106_OK);
}

/* ----------------------------------------------------------------------- */
// Swaps nbytes of 16 bit words in place
EnI106Status I106_CALL_DECL SwapShortWords_PcmF1(uint16_t *puBuffer, long nBytes)
{
    long Counter = nBytes;
    uint16_t ubTemp;
    if(nBytes & 3)
        return(I106_BUFFER_OVERRUN); // May be also an underrun ...
    Counter >>= 1;
    while((Counter -= 2) >= 0)
    {
        ubTemp = *puBuffer;
        *puBuffer = *(puBuffer + 1);
        *++puBuffer = ubTemp;
        puBuffer++;
    }
    return(I106_OK);
}

#ifdef __cplusplus
} // end namespace
#endif

