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
    if(psuMsg->psuAttributes->bFirstRun)
    {

        #ifdef DEBUG_OTHER_PCM_FILE
        psuMsg->psuAttributes->lByteSwap = 0; // debug other pcm file
        if(FilePcmTest == NULL)
        {
            FilePcmTest = fopen("d:\\projects\\winspo\\winspoca\\F1249\\PC101039.ftr", "rb");
            if(FilePcmTest == NULL)
                return I106_NO_MORE_DATA;
            fseek(FilePcmTest, 0x64, SEEK_SET);
        }
        #endif

        // Set up the data
        EnI106Status enStatus = PcmF1FirstRun(psuMsg);
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

        if(psuMsg->psuAttributes->lByteSwap)
        {
            if(SwapBytes_PcmF1(psuMsg->pauData, psuMsg->ulSubPacketLen))
                return(I106_INVALID_DATA); 
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
        //psuMsg->psuAttributes->ulBitPosition = 0;
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
EnI106Status Set_PcmF1_Attributes(SuRDataSource * psuRDataSrc, SuPcmF1_Attributes * psuPcmF1_Attributes)
{

    SuPRecord           * psuPRecord;
    uint32_t uBitCount;

    //SuPcmF1_Attributes  * psuPcmF1_Attributes;

    if(psuPcmF1_Attributes == NULL) return I106_INVALID_PARAMETER; // Set Attributes

    memset(psuPcmF1_Attributes, 0, sizeof(SuPcmF1_Attributes));
    psuPRecord = psuRDataSrc->psuPRecord;

    if(psuPRecord == NULL) return I106_INVALID_PARAMETER; // Set Attributes

        // Take TMATS values
        // -----------------
        // TODO: Check for essential values
        psuPcmF1_Attributes->iRecordNum                 = psuPRecord->iRecordNum; // P-x
        if(psuPRecord->szBitsPerSec != NULL)
            psuPcmF1_Attributes->uBitsPerSec            = atol(psuPRecord->szBitsPerSec); // P-x\D2
        if(psuPRecord->szCommonWordLen != NULL)
            psuPcmF1_Attributes->ulCommonWordLen        = atol(psuPRecord->szCommonWordLen); // P-x\F1

        if(psuPRecord->szParityType != NULL)
        {
            psuPcmF1_Attributes->lParityType = 0;   // P-x/F3
        }
        if(psuPRecord->szParityTransferOrder != NULL)
        {
            psuPcmF1_Attributes->lParityTransferOrder = 0;   // P-x/F4
        }
        if(psuPRecord->szNumMinorFrames != NULL)
            psuPcmF1_Attributes->ulNumMinorFrames       = atol(psuPRecord->szNumMinorFrames); // P-x\MF\N
        if(psuPRecord->szWordsInMinorFrame != NULL)
            psuPcmF1_Attributes->ulWordsInMinorFrame    = atol(psuPRecord->szWordsInMinorFrame); // P-x\MF1
        if(psuPRecord->szBitsInMinorFrame != NULL)
            psuPcmF1_Attributes->ulBitsInMinorFrame     = atol(psuPRecord->szBitsInMinorFrame); // P-x\MF2
        if(psuPRecord->szMinorFrameSyncPatLen != NULL)
            psuPcmF1_Attributes->ulMinorFrameSyncPatLen = atol(psuPRecord->szMinorFrameSyncPatLen); // P-x\MF4
        if(psuPRecord->szInSyncCrit != NULL)
            psuPcmF1_Attributes->ulMinSyncs = 0; // Minimal number of syncs P-x\SYNC1;
        
        if(psuPRecord->szMinorFrameSyncPat != NULL) // P-x\MF5
        {
            uint64_t uSyncPat = 0;
            uint64_t uSyncMask = 0;
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
                uSyncMask <<= 1;
                uSyncMask |= 1;
                
                uSyncPat <<= 1;
                if(*pChar == '1') 
                    uSyncPat |= 1;
                pChar++;
            }
            psuPcmF1_Attributes->uMinorFrameSyncPat = uSyncPat;
            psuPcmF1_Attributes->uMinorFrameSyncMask = uSyncMask;
            if(psuPcmF1_Attributes->ulMinorFrameSyncPatLen == 0)
                psuPcmF1_Attributes->ulMinorFrameSyncPatLen = ulMinorFrameSyncPatLen;
        } // minor frame sync pat
        
        if(psuPcmF1_Attributes->ulBitsInMinorFrame == 0)
        {
            psuPcmF1_Attributes->ulBitsInMinorFrame = psuPcmF1_Attributes->ulCommonWordLen * (psuPcmF1_Attributes->ulWordsInMinorFrame - 1) +
                psuPcmF1_Attributes->ulMinorFrameSyncPatLen;
        }
        for(uBitCount = 0; uBitCount < psuPcmF1_Attributes->ulCommonWordLen; uBitCount++)
        {
            psuPcmF1_Attributes->ullWordMask <<= 1;
            psuPcmF1_Attributes->ullWordMask |= 1;
        }
        
        psuPcmF1_Attributes->dDelta100NanoSeconds = d100NANOSECONDS / psuPcmF1_Attributes->uBitsPerSec;
        
        psuPcmF1_Attributes->bFirstRun = 1;
        
        return I106_OK;

    psuPcmF1_Attributes->dDelta100NanoSeconds = d100NANOSECONDS / psuPcmF1_Attributes->uBitsPerSec;

    psuPcmF1_Attributes->bFirstRun = 1;

    return I106_OK;
} // End Set_PcmF1_Attributes

/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    PcmF1FirstRun(SuPcmF1_CurrMsg * psuMsg)
{

    SuPcmF1_Attributes * psuAttributes = psuMsg->psuAttributes;

    // Allocate the Pcm output buffer
    psuAttributes->ulOutBufSize = psuAttributes->ulWordsInMinorFrame;
    psuAttributes->paullOutBuf =    (uint64_t *)calloc(sizeof(uint64_t), psuAttributes->ulOutBufSize);
    if(psuAttributes->paullOutBuf == NULL)
        return I106_BUFFER_TOO_SMALL;
    
    psuAttributes->pauOutBufErr = (uint8_t *)calloc(sizeof(uint8_t), psuAttributes->ulOutBufSize);
    if(psuAttributes->pauOutBufErr == NULL)
        return I106_BUFFER_TOO_SMALL;

    psuAttributes->bFirstRun = 0;
    
    // If not throughput mode, the work is done 
    // ----------------------------------------
    if( ! psuMsg->psuChanSpec->bThruMode)
        return I106_OK;

    // Prepare the variables for bit decodng in throughput mode
    // --------------------------------------------------------
    psuAttributes->ullSyncCount = -1; // -1 sets all bits to 1
    psuAttributes->ullSyncErrors = 0;
    psuAttributes->ullTestWord = 0; 
    psuAttributes->ulBitPosition = 0; 
    psuAttributes->ullBitsLoaded = 0;
    psuAttributes->ulMinorFrameBitCount = 0;
    psuAttributes->ulMinorFrameWordCount = 0;
    psuAttributes->ulDataWordBitCount = 0;
    psuAttributes->lSaveData = 0;

  return I106_OK;

}

/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    DecodeMinorFrame_PcmF1(SuPcmF1_CurrMsg * psuMsg)
{

    SuPcmF1_Attributes * psuAttributes = psuMsg->psuAttributes;

    while(psuAttributes->ulBitPosition < psuMsg->ulSubPacketBits)
    {

        GetNextBit(psuMsg, psuAttributes);

        // Check for a sync word

        if(IsSyncWordFound(psuAttributes))
        {   
            // Prevent an overflow after a terabyte of bits
            if(psuAttributes->ullBitsLoaded > 1000000000000)
                psuAttributes->ullBitsLoaded = 1000000000;

            psuAttributes->ullSyncCount++;

            //TRACE("Sync word found at BitPos %6d, MFBitCnt %5d, 0x%08X, SyncCnt %6d\n", 
            //  psuAttributes->ulBitPosition, psuAttributes->ulMinorFrameBitCount, (int32_t)psuAttributes->ullTestWord, psuAttributes->ullSyncCount);

            if(psuAttributes->ulMinorFrameBitCount == psuAttributes->ulBitsInMinorFrame)
            {
                // A sync word at the correct offset to the previous one

                RenewSyncCounters(psuAttributes, psuAttributes->ullSyncCount); // with the current sync counter
                
                // If there are enough syncs, release the previous filled outbuf
                // Note: a minor frame is released only, if it is followed by a sync word at the correct offset. 
                // i.e. the sync word are used as brackets
                if((psuAttributes->ullSyncCount >= psuAttributes->ulMinSyncs) && (psuAttributes->lSaveData > 1)) 
                {

                    // Compute the intrapacket time of the start sync bit position in the current buffer
                    int64_t lulBitPosition = (int64_t)psuAttributes->ulBitPosition - (int64_t)psuAttributes->ulBitsInMinorFrame /*-                                                                                                                         (int64_t)psuAttributes->ulMinorFrameSyncPatLen*/;

                    double dOffsetIntPktTime = (double)lulBitPosition * psuAttributes->dDelta100NanoSeconds;   

                    psuMsg->llIntPktTime = psuMsg->llBaseIntPktTime + (int64_t)dOffsetIntPktTime; // Relative time, omit rounding

                    // Prepare for the next run
                    PrepareNewMinorFrameCollection(psuAttributes);
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

                // RenewSyncCounters with a sync counter of zero
                RenewSyncCounters(psuAttributes, 0);
            }

            PrepareNewMinorFrameCollection(psuAttributes);
            continue;

        } // if sync found

        // Collect the data

        if(psuAttributes->lSaveData == 1)
        {
            psuAttributes->ulDataWordBitCount++;
            if(psuAttributes->ulDataWordBitCount >= psuAttributes->ulCommonWordLen)
            {
                psuAttributes->paullOutBuf[psuAttributes->ulMinorFrameWordCount - 1] = psuAttributes->ullTestWord ;
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

    }

    // Preset for the next run
    psuAttributes->ulBitPosition = 0;

  return I106_NO_MORE_DATA;
}

/////////////////////////////////////////////////////////////////////////////
// Returns I106_OK on success, I106_INVALID_DATA on error
// Assumes the parity at the last position
EnI106Status PcmCheckParity(uint64_t ullTestWord, int iWordLen, int iParityType, int iParityTransferOrder)
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
// Prepare a new minor frame collection
 void  PrepareNewMinorFrameCollection(SuPcmF1_Attributes * psuAttributes)
{
    psuAttributes->ulDataWordBitCount = 0;
    psuAttributes->lSaveData = 1;
}


/* ----------------------------------------------------------------------- */
// Get the next bit
void GetNextBit(SuPcmF1_CurrMsg * psuMsg, SuPcmF1_Attributes * psuAttributes)
{
    psuAttributes->ullTestWord <<= 1;
    if(IsBitSetL2R(psuMsg->pauData, psuAttributes->ulBitPosition))
        psuAttributes->ullTestWord |= 1;
    psuAttributes->ullBitsLoaded++;
    psuAttributes->ulMinorFrameBitCount++;
    psuAttributes->ulBitPosition++;

}

/* ----------------------------------------------------------------------- */
// Check for a sync word
int IsSyncWordFound(SuPcmF1_Attributes * psuAttributes)
{
    return((psuAttributes->ullBitsLoaded >= psuAttributes->ulMinorFrameSyncPatLen) && 
                (psuAttributes->ullTestWord & psuAttributes->uMinorFrameSyncMask) == psuAttributes->uMinorFrameSyncPat);
}

/* ----------------------------------------------------------------------- */
// RenewSyncCounters
void RenewSyncCounters(SuPcmF1_Attributes * psuAttributes, uint64_t ullSyncCount)
{
    psuAttributes->ulMinorFrameBitCount = 0; 
    psuAttributes->ulMinorFrameWordCount = 1; 
    psuAttributes->ulDataWordBitCount = 0;
    psuAttributes->ullSyncCount = ullSyncCount;
}

/* ----------------------------------------------------------------------- */

EnI106Status SwapBytes_PcmF1(uint8_t *Buffer,long nBytes)
          // Swaps nBytes in the Buffer
{

  uint8_t ct;
  while((nBytes -= 2) >= 0)
  {
    ct = *Buffer;
    *Buffer = *(Buffer+1);
    *(Buffer+1) = ct;
    Buffer +=2;
  }
    if(nBytes & 1)
    {
        return(I106_INVALID_DATA); // This is an error ....
    }
    return(I106_OK);
}

#ifdef __cplusplus
} // end namespace
#endif

