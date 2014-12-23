/****************************************************************************

 i106_decode_analogf1.c -

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

 Author: Spencer Hatch, Dartmouth College, Hanover, NH, USA
 *STOLEN* from Hans-Gerhard Flohr's i106_decode_analogf1.c
 2014 NOV Initial Version 1.0


 ****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <assert.h>
#include <inttypes.h>

#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_decode_tmats.h"
#include "i106_decode_analogf1.h"

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

/*
 * Function Declaration
 * --------------------
 */

// Local functions
EnI106Status PrepareNextDecodingRun_AnalogF1(SuAnalogF1_CurrMsg * psuMsg);

/* ======================================================================= */
/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Setup_AnalogF1(SuI106Ch10Header     * psuHeader,
				  void               * pvBuff,
				  SuAnalogF1_CurrMsg * psuMsg)
{

    int                     iSubChanIdx;
    uint32_t                ulSubPacketLen;
    uint32_t                uRemainder;
    uint32_t                uTotChan;
    SuAnalogF1_Attributes * psuAttributes;

    // Check for attributes available
    if(psuMsg->psuAttributes == NULL)
        return I106_INVALID_PARAMETER;

    psuAttributes = psuMsg->psuAttributes;
    
    // Set pointers to the beginning of the Analog buffer
    psuMsg->psuHeader = psuHeader; 
    psuMsg->psuChanSpec = (SuAnalogF1_ChanSpec *)pvBuff; 

    //Set variables for reading
    psuMsg->ulBytesRead = 0;
    psuMsg->ulDataLen = psuHeader->ulDataLen;
    
    //Make sure we're using packed analog data (others unsupported!)
    if(psuMsg->psuChanSpec->uMode != ANALOG_PACKED)
        return I106_UNSUPPORTED;
    
    // Check whether number of subchannels reported by TMATS matches number reported by CSDW
    if(psuAttributes->iAnalogChansPerPkt != psuMsg->psuChanSpec->uTotChan)
    {
        fprintf(stderr,"idmpanalog: TMATS # of subchannels reported does not match CSDW total number of channels!\n");
        return I106_INVALID_DATA;
    }
    
    //Based on first CSDWs 'Same' bit, prepare to allocate additional CSDW structs
    uTotChan = psuMsg->psuChanSpec->uTotChan;
    iSubChanIdx = 0;
    
    do
    {
        //Allocate memory for _pointers_ to all CSDWs
        //and allocate memory for CSDWs themselves
        psuAttributes->apsuSubChan[iSubChanIdx] = malloc(sizeof(SuAnalogF1_SubChan));

	psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec = malloc(sizeof(SuAnalogF1_ChanSpec));

	//Open and set name of subchan outfile, allocate subchan buffer
	sprintf(psuAttributes->apsuSubChan[iSubChanIdx]->szSubChOutFile,"%s--Analog_Subchan%i.dmpanalog",psuAttributes->szDataSourceID,iSubChanIdx);
	printf("Opening subchannel output file %s...\n",psuAttributes->apsuSubChan[iSubChanIdx]->szSubChOutFile);

	if((psuAttributes->apsuSubChan[iSubChanIdx]->psuSubChOutFile = fopen(psuAttributes->apsuSubChan[iSubChanIdx]->szSubChOutFile,"w")) == NULL )
	{
	    //NOTE: NEED TO FREE ALL MALLOC'ed MEM IF THINGS GO AWRY
	    return(I106_OPEN_ERROR);
	}
	
	if((psuAttributes->apsuSubChan[iSubChanIdx]->pauSubData = calloc(1, psuMsg->ulDataLen)) == NULL)
        {
	    return(I106_BUFFER_TOO_SMALL);
	}
	
        //Copy CSDW into allocated mem for future reference
        //NOTE: I have not tested situations where bSame is bFALSE!
        if(psuMsg->psuChanSpec->bSame == bFALSE)
        {
     	    memcpy(psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec, &(psuMsg->psuChanSpec[iSubChanIdx]), sizeof(SuAnalogF1_ChanSpec));
        }
        else
        {
            psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec = malloc(sizeof(SuAnalogF1_ChanSpec));
	    
            //Copy CSDW into allocated mem for future reference
	    memcpy(psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec, &(psuMsg->psuChanSpec[0]), sizeof(SuAnalogF1_ChanSpec));
	}
 
	//update bytes read outside do loop
	if( psuMsg->psuChanSpec->bSame == bFALSE )
	{
	    psuMsg->ulBytesRead += sizeof(SuAnalogF1_ChanSpec) * psuAttributes->iAnalogChansPerPkt;
	}
	else
	{
	  psuMsg->ulBytesRead += sizeof(SuAnalogF1_ChanSpec);
	}
	PrintCSDW_AnalogF1(psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec);
	
	iSubChanIdx++;
    }while ( iSubChanIdx < uTotChan );

    //Bubble sort pointers to subchannels
    //This is necessary (for this implementation) in order to comply with the organization of analog data packets as described in Ch.10
    for (iSubChanIdx = 0 ; iSubChanIdx < uTotChan; iSubChanIdx++)
    {
        int32_t              iSubChanIdx2;
	SuAnalogF1_SubChan * psuSwapSubChan;
	for ( iSubChanIdx2 = 0 ;  iSubChanIdx2 < uTotChan - iSubChanIdx - 1; iSubChanIdx2++)
	{
	    int iCurrSubChan = psuAttributes->apsuSubChan[iSubChanIdx2]->psuChanSpec->uSubChan;
	    int iCurrSubChan2 = psuAttributes->apsuSubChan[iSubChanIdx2+1]->psuChanSpec->uSubChan;

	    if (iCurrSubChan > iCurrSubChan2)
	    {
	      	psuSwapSubChan = psuAttributes->apsuSubChan[iSubChanIdx2];
	      	psuAttributes->apsuSubChan[iSubChanIdx2] = psuAttributes->apsuSubChan[iSubChanIdx2+1];
		psuAttributes->apsuSubChan[iSubChanIdx2+1] = psuSwapSubChan;
	    }
        }
    }

    /* // Check for no (more) data */
    /* if (psuMsg->ulDataLen <= psuMsg->ulBytesRead) */
    /*     return I106_NO_MORE_DATA; */

    /* // Set the pointer to the Analog message data */
    /* psuMsg->pauData = (uint8_t *)((char *)(psuMsg->psuChanSpec) + psuMsg->ulBytesRead); */
    
    /* // Prepare the Analog buffers and load the first bits */
    /* if(psuAttributes->bPrepareNextDecodingRun) */
    /* { */
    /*     // Set up the data */
    /*     EnI106Status enStatus = PrepareNextDecodingRun_AnalogF1(psuMsg); */
    /*     if(enStatus != I106_OK) */
    /*         return enStatus; */
    /* } */

    return (I106_OK);

} // End enI106_Setup_AnalogF1

/* ======================================================================= */
/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstAnalogF1(SuI106Ch10Header     * psuHeader,
				  void               * pvBuff,
				  SuAnalogF1_CurrMsg * psuMsg)
{

    int                     iSubChanIdx;
    uint32_t                ulSubPacketLen;
    uint32_t                uRemainder;
    uint32_t                uTotChan;
    SuAnalogF1_Attributes * psuAttributes;
    
    // Check for attributes available
    if(psuMsg->psuAttributes == NULL)
        return I106_INVALID_PARAMETER;

    psuAttributes = psuMsg->psuAttributes;
    
    // Set pointers to the beginning of the Analog buffer
    psuMsg->psuHeader = psuHeader; 
    psuMsg->psuChanSpec = (SuAnalogF1_ChanSpec *)pvBuff; 

    //Set variables for reading
    psuMsg->ulBytesRead = 0;
    psuMsg->ulDataLen = psuHeader->ulDataLen;
    
    //Make sure we're using packed analog data (others unsupported!)
    if(psuMsg->psuChanSpec->uMode != ANALOG_PACKED)
    {
        fprintf(stderr,"Dump of unpacked analog data not supported!\n");
        return I106_UNSUPPORTED;
    }    

    // Check whether number of subchannels reported by TMATS matches number reported by CSDW
    if(psuAttributes->iAnalogChansPerPkt != psuMsg->psuChanSpec->uTotChan)
    {
        fprintf(stderr,"idmpanalog: TMATS # of subchannels reported does not match CSDW total number of channels!\n");
        return I106_INVALID_DATA;
    }

    uTotChan = psuMsg->psuChanSpec->uTotChan;
    iSubChanIdx = 0;
    
    do
    {
        //Check current CSDW against the first one received
        //NOTE: I have not tested situations where bSame is bFALSE!
        if(psuMsg->psuChanSpec->bSame == bFALSE)
        {

	  if( ( psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec->uMode != psuMsg->psuChanSpec[iSubChanIdx].uMode )
	      || ( psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec->uLength != psuMsg->psuChanSpec[iSubChanIdx].uLength )
	      || ( psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec->uTotChan != psuMsg->psuChanSpec[iSubChanIdx].uTotChan ) )
	  {
	    fprintf(stderr,"ERROR for subchannel %i on analog channel %s: Current CSDW does not match initial CSDW\n",psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec->uSubChan, psuAttributes->szDataSourceID);
	    return(I106_INVALID_DATA);
	  }
        }
        else
        {
	  if( ( psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec->uMode != psuMsg->psuChanSpec[iSubChanIdx].uMode )
	      || ( psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec->uLength != psuMsg->psuChanSpec[iSubChanIdx].uLength )
	      || ( psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec->uTotChan != psuMsg->psuChanSpec[iSubChanIdx].uTotChan ) )
	  {
	    fprintf(stderr,"ERROR for subchannel %i on analog channel %s: Current CSDW does not match initial CSDW\n",psuAttributes->apsuSubChan[iSubChanIdx]->psuChanSpec->uSubChan, psuAttributes->szDataSourceID);
	    return(I106_INVALID_DATA);
	  }
	}

	iSubChanIdx++;
    }while ( iSubChanIdx < uTotChan );

    //update bytes read outside do loop
    if( psuMsg->psuChanSpec->bSame == bFALSE )
    {
        psuMsg->ulBytesRead += sizeof(SuAnalogF1_ChanSpec) * psuAttributes->iAnalogChansPerPkt;
    }
    else
    {
        psuMsg->ulBytesRead += sizeof(SuAnalogF1_ChanSpec);
    }	

    // Check for no (more) data
    if (psuMsg->ulDataLen <= psuMsg->ulBytesRead)
        return I106_NO_MORE_DATA;

    // Set the pointer to the Analog message data
    psuMsg->pauData = (uint8_t *)((char *)(psuMsg->psuChanSpec) + psuMsg->ulBytesRead);
    
    // Prepare the Analog buffers and load the first bits
    if(psuAttributes->bPrepareNextDecodingRun)
    {
        // Set up the data
        EnI106Status enStatus = PrepareNextDecodingRun_AnalogF1(psuMsg);
        if(enStatus != I106_OK)
            return enStatus;
    }

    // Now start the decode of this buffer

    return (DecodeBuff_AnalogF1(psuMsg));
} // End enI106_Decode_FirstAnalogF1

/* ----------------------------------------------------------------------- */

//SPENCE CHECK--Maybe this function is useless now...
EnI106Status I106_CALL_DECL 
    enI106_Decode_NextAnalogF1(SuAnalogF1_CurrMsg * psuMsg)
{
  return (DecodeBuff_AnalogF1(psuMsg));
}

/* ----------------------------------------------------------------------- */

// Fill the attributes from TMATS 

EnI106Status I106_CALL_DECL Set_Attributes_AnalogF1(SuRDataSource * psuRDataSrc, SuAnalogF1_Attributes * psuAnalogF1_Attributes)
{

    uint32_t            uBitCount;

    if(psuAnalogF1_Attributes == NULL) return I106_INVALID_PARAMETER; // Set Attributes

    memset(psuAnalogF1_Attributes, 0, sizeof(SuAnalogF1_Attributes));

    // Collect the TMATS values
    // ------------------------

    psuAnalogF1_Attributes->psuRDataSrc                = psuRDataSrc; // May be, we need it in the future

    psuAnalogF1_Attributes->szDataSourceID             = psuRDataSrc->szDataSourceID;

    psuAnalogF1_Attributes->iDataSourceNum             = psuRDataSrc->iDataSourceNum; // R-x

    //Get number of chans per packet
    if(psuRDataSrc->szAnalogChansPerPkt != NULL)
      psuAnalogF1_Attributes->iAnalogChansPerPkt = atoi(psuRDataSrc->szAnalogChansPerPkt);

    //Get sample rate
    if(psuRDataSrc->szAnalogSampleRate != NULL)
       psuAnalogF1_Attributes->ullAnalogSampleRate = strtoull(psuRDataSrc->szAnalogSampleRate, NULL, 10);
    
    //Get whether data is packed
    if(psuRDataSrc->szAnalogIsDataPacked != NULL)
        psuAnalogF1_Attributes->bAnalogIsDataPacked = psuRDataSrc->bAnalogIsDataPacked;

    //Get size of a data sample on this channel
    if(psuRDataSrc->szAnalogDataLength != NULL)
       psuAnalogF1_Attributes->ulAnalogDataLength = strtoul(psuRDataSrc->szAnalogDataLength, NULL, 10);

    if(psuRDataSrc->szAnalogMeasTransfOrd != NULL)    // R-x\AMTO-n-m most significant bit "M", least significant bit "L". default: M
    {
        /* Measurement Transfer Order. Which bit is being transferred first is specified as – Most Significant Bit (M), 
        Least Significant Bit (L), or Default (D).
        D-1\MN3-1-1:M;
        */
        if(psuRDataSrc->szAnalogMeasTransfOrd[0] == 'L')
        {
            psuAnalogF1_Attributes->ulAnalogMeasTransfOrd = ANALOG_LSB_FIRST;
            return(I106_UNSUPPORTED);
        }
    }

    //Get Sample Filter 3dB Bandwidth (in Hz)
    if(psuRDataSrc->szAnalogSampleFilter != NULL)
       psuAnalogF1_Attributes->ullAnalogSampleFilter = strtoull(psuRDataSrc->szAnalogSampleFilter, NULL, 10);

    //Get whether AC/DC Coupling
    if(psuRDataSrc->szAnalogIsDCCoupled != NULL)
      psuAnalogF1_Attributes->bAnalogIsDCCoupled = psuRDataSrc->bAnalogIsDCCoupled;

    //Get Recorder Input Impedance 
    if(psuRDataSrc->szAnalogRecImpedance != NULL)
       psuAnalogF1_Attributes->ulAnalogRecImpedance = strtoul(psuRDataSrc->szAnalogRecImpedance, NULL, 10);

    //Get Channel Gain in milli units (10x = 010000)
    if(psuRDataSrc->szAnalogChanGain != NULL)
       psuAnalogF1_Attributes->ulAnalogChanGain = strtoul(psuRDataSrc->szAnalogChanGain, NULL, 10);

    //Get Full-Scale Range (in milliVolts)
    if(psuRDataSrc->szAnalogFullScaleRange != NULL)
       psuAnalogF1_Attributes->ulAnalogFullScaleRange = strtoul(psuRDataSrc->szAnalogFullScaleRange, NULL, 10);

    //Get Offset Voltage (in milliVolts)
    if(psuRDataSrc->szAnalogOffsetVoltage != NULL)
       psuAnalogF1_Attributes->lAnalogOffsetVoltage = strtoul(psuRDataSrc->szAnalogOffsetVoltage, NULL, 10);

    //Get LSB Value
    if(psuRDataSrc->szAnalogLSBValue != NULL)
       psuAnalogF1_Attributes->lAnalogLSBValue = strtoul(psuRDataSrc->szAnalogLSBValue, NULL, 10);

    //Get Analog Format 
    //"1" = One's comp. 
    //"2" = Two's comp.
    //"3" = Sign and magnitude binary [+=0]
    //"4" = Sign and magnitude binary [+=1]
    //"B" = Offset binary
    //"U" = Unsigned binary
    //"F" = IEEE 754 single-precision [IEEE 32] floating point
    if(psuRDataSrc->szAnalogFormat != NULL)
      switch ( psuRDataSrc->szAnalogFormat[0] )
      {
      case '1':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_ONES;
	break;
      case '2':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_TWOS; 
	break;
      case '3':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_SIGNMAG_0;
	break;
      case '4':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_SIGNMAG_1;
	break;
      case 'B':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_OFFSET_BIN;
	break;
      case 'U':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_UNSIGNED_BIN;
	break;
      case 'F':
	psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_SINGLE_FLOAT;
	break;
      default:
	return(I106_UNSUPPORTED);
        break;
      }

    //Get analog input type; 'D' = differential, 'S' = single-ended
    if(psuRDataSrc->szAnalogDifferentialInp != NULL)
       psuAnalogF1_Attributes->bAnalogDifferentialInp = psuRDataSrc->bAnalogDifferentialInp;
 
    //Get whether audio
    if(psuRDataSrc->szAnalogIsAudio != NULL)
       psuAnalogF1_Attributes->bAnalogIsAudio = psuRDataSrc->bAnalogIsAudio;
        
    psuAnalogF1_Attributes->bPrepareNextDecodingRun = 1; // Set_Attributes_AnalogF1
        
    return I106_OK;
} // End Set_Attributes _AnalogF1

/* ----------------------------------------------------------------------- */

// Create the output buffers (data and error flags)
EnI106Status I106_CALL_DECL 
  CreateOutputBuffers_AnalogF1(SuAnalogF1_Attributes * psuAttributes, uint32_t ulDataLen)
{

    // Allocate the Analog output buffer
    psuAttributes->ulOutBufSize = ulDataLen;
    psuAttributes->paullOutBuf = (uint8_t *)calloc(sizeof(uint8_t), ulDataLen);
    if(psuAttributes->paullOutBuf == NULL)
        return I106_BUFFER_TOO_SMALL;
    
    psuAttributes->pauOutBufErr = (uint8_t *)calloc(sizeof(uint8_t), ulDataLen);
    if(psuAttributes->pauOutBufErr == NULL)
    {
        free(psuAttributes->paullOutBuf); psuAttributes->paullOutBuf = NULL;
        return I106_BUFFER_TOO_SMALL;
    }
    return(I106_OK);
} // End CreateOutputBuffers

/* ----------------------------------------------------------------------- */

// Free the output buffers
EnI106Status I106_CALL_DECL FreeOutputBuffers_AnalogF1(SuAnalogF1_Attributes * psuAttributes)
{

    if(psuAttributes->paullOutBuf)
    {
        free(psuAttributes->paullOutBuf);
        psuAttributes->paullOutBuf = NULL;
    }
    if(psuAttributes->pauOutBufErr)
    {
        free(psuAttributes->pauOutBufErr);
        psuAttributes->pauOutBufErr = NULL;
    }
    psuAttributes->bPrepareNextDecodingRun = 1; 

    return(I106_OK);
} // End FreeOutputBuffers

/* ----------------------------------------------------------------------- */

// Prepare a new decoding run 
// Creates the output buffers and resets values and counters
EnI106Status PrepareNextDecodingRun_AnalogF1(SuAnalogF1_CurrMsg * psuMsg)
{
    SuAnalogF1_Attributes * psuAttributes = psuMsg->psuAttributes;

    EnI106Status enStatus = CreateOutputBuffers_AnalogF1(psuAttributes, psuMsg->ulDataLen);
    if(enStatus != I106_OK)
        return(enStatus);

    psuAttributes->bPrepareNextDecodingRun = 0;
    
    psuAttributes->lSaveData = 0;

    return I106_OK;

} // End PrepareNextDecodingRun


//TO DO: Implement reading of MSB- or LSB-padded packets
//Currently only supports packed data
/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    DecodeBuff_AnalogF1(SuAnalogF1_CurrMsg * psuMsg)
{

    //Use these arrays to save on time
    SuAnalogF1_Attributes * psuAttributes;
    uint32_t                aulSubChanSampFactor[ANALOG_MAX_SUBCHANS];
    uint32_t                aulSubChanSampSize[ANALOG_MAX_SUBCHANS];
    uint32_t                ulNumSubChans;
    
    psuAttributes = psuMsg->psuAttributes;
    ulNumSubChans = psuAttributes->iAnalogChansPerPkt;

    SuAnalogF1_SubChan * psuCurrSubChan;
    int32_t iSubChanIdx = 0;
    int32_t iMaxSimulSamps = 0;


    if ( psuAttributes->bAnalogIsDataPacked == bTRUE ) {
        //Need to distinguish between subchannels if not identical
        //NOTE: I have not tested code where bSame == bFALSE!
        if ( psuMsg->psuChanSpec->bSame == bFALSE ){
        
            //Calculate all factors for each channel (done here for speed)
            //Also get max number of simultaneous samples (See pp 56-57 in IRIG-106 Ch10 June 2013 rev.)
            //Also ensure all sample sizes an integer factor of 8
            for ( iSubChanIdx = 0; iSubChanIdx < ulNumSubChans; iSubChanIdx++ )
            {
                psuCurrSubChan = psuMsg->psuAttributes->apsuSubChan[iSubChanIdx];
            	int iPowIdx = 0;
            	aulSubChanSampFactor[iSubChanIdx] = 0;
            	if ( psuCurrSubChan->psuChanSpec->uFactor > 0 ){
            	    while ( iPowIdx < psuCurrSubChan->psuChanSpec->uFactor ){
            	        aulSubChanSampFactor[iSubChanIdx] *= 2;
            		iPowIdx++;
            	    }
            	    if ( iMaxSimulSamps < aulSubChanSampFactor[iSubChanIdx] )
            	        iMaxSimulSamps = aulSubChanSampFactor[iSubChanIdx];
            	}
            	//Now get sample sizes for each subchannel
                aulSubChanSampSize[iSubChanIdx] = psuCurrSubChan->psuChanSpec->uLength;
            	if ( aulSubChanSampSize[iSubChanIdx] % 8 != 0 )
                        return I106_UNSUPPORTED;
            }
            
            
            int32_t iSimulSamp;
            while ( psuMsg->ulBytesRead < psuMsg->ulDataLen )
            {
                for ( iSimulSamp = 1; iSimulSamp < iMaxSimulSamps; iSimulSamp++ )
                {
                    int iSubChanIdx;
                    for ( iSubChanIdx = 0; iSubChanIdx < ulNumSubChans; iSubChanIdx++ )
                    {
            	        if( iSimulSamp == aulSubChanSampFactor[iSubChanIdx] )
                        {
                          //code to write data to sampbuff
            		  /*     if(psuAttributes->lSaveData == 1) */
            		  /*     { */
            		  /*     } */
            		  
            		    psuCurrSubChan = psuMsg->psuAttributes->apsuSubChan[iSubChanIdx];
            		    psuCurrSubChan->uSubBytesRead += aulSubChanSampSize[iSubChanIdx];
                	    psuMsg->ulBytesRead += aulSubChanSampSize[iSubChanIdx];
                        }
                
                    }
                    
                }
            }
        } //end psuMsg->psuChanSpec->bSame == bFALSE
        else 
        {
    
            if( ulNumSubChans > 1 ) {
            int32_t iSimulSamp;
                while ( psuMsg->ulBytesRead < psuMsg->ulDataLen )
                {
                    int iSubChanIdx;
                    for ( iSubChanIdx = 0; iSubChanIdx < ulNumSubChans; iSubChanIdx++ )
                    {
                        //code to write data to sampbuff
                	/*     if(psuAttributes->lSaveData == 1) */
                	/*     { */
                	/*     } */
    	          
    	            psuCurrSubChan = psuMsg->psuAttributes->apsuSubChan[iSubChanIdx];
    		    psuCurrSubChan->uSubBytesRead += aulSubChanSampSize[iSubChanIdx];
    		    psuMsg->ulBytesRead += aulSubChanSampSize[iSubChanIdx];
    	        }
    	    }      
    	}
    	else //handle the simplest case--only one subchannel
    	{
    	    psuCurrSubChan = psuMsg->psuAttributes->apsuSubChan[iSubChanIdx];
    	    //Code to write all data to sampbuff
    	    if ( ( ( psuMsg->ulDataLen - psuMsg->ulBytesRead ) % ( psuCurrSubChan->psuChanSpec->uLength/8) ) != 0 )
    	    {
    	    printf("What the!?!? Remaining databuff doesn't allow for clean copy!!!!\n");
    	    return I106_INVALID_DATA;
    	    }
    	    
    	    int bytesR = fwrite(psuMsg->pauData + psuMsg->ulBytesRead,1, psuMsg->ulDataLen - psuMsg->ulBytesRead,psuCurrSubChan->psuSubChOutFile);
    	    printf("Wrote %i bytes to %s\n",bytesR,psuCurrSubChan->szSubChOutFile);
    	    
    	    psuMsg->ulBytesRead = psuMsg->ulDataLen;
    	}
        }
    
    } //end if AnalogIsDataPacked == bTRUE
    else
    {
      fprintf(stderr,"idmpanalog: Dumping of unpacked analog data is currently unsupported!\nEnding...\n");
      return I106_UNSUPPORTED;
    }

  return I106_NO_MORE_DATA;
} //End DecodeBuff_AnalogF1

/* ----------------------------------------------------------------------- */
// Swaps nBytes in place
EnI106Status I106_CALL_DECL SwapBytes_AnalogF1(uint8_t *pubBuffer, long nBytes)
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
    SwapShortWords_AnalogF1((uint16_t *)&idata, 4);

    return(I106_OK);
}

/* ----------------------------------------------------------------------- */
// Swaps nbytes of 16 bit words in place
EnI106Status I106_CALL_DECL SwapShortWords_AnalogF1(uint16_t *puBuffer, long nBytes)
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

EnI106Status I106_CALL_DECL PrintCSDW_AnalogF1(SuAnalogF1_ChanSpec *psuChanSpec)
{

  printf("Subchannel number:\t\t %" PRIu32 "\n", psuChanSpec->uSubChan);
  printf("Mode:\t\t\t\t %" PRIu32 "\n", psuChanSpec->uMode);
  printf("Total number of subchannels:\t %" PRIu32 "\n", psuChanSpec->uTotChan);
  printf("Sampling factor:\t\t %" PRIu32 "\n", psuChanSpec->uFactor);
  printf("Same bit:\t\t\t %" PRIu32 "\n", psuChanSpec->bSame);
  printf("Reserved:\t\t\t %" PRIu32 "\n", psuChanSpec->iReserved);
  

}

 EnI106Status I106_CALL_DECL PrintAttributesfromTMATS_AnalogF1(SuRDataSource * psuRDataSource, SuAnalogF1_Attributes *psuAttributes, FILE * psuOutFile)
{
  
  if( ( psuRDataSource == NULL )  || ( psuAttributes == NULL ) )
      return I106_INVALID_PARAMETER;

  printf("\n");
  printf("========================================\n");  
  printf("TMATS Attributes, Data Source %s\n", psuRDataSource->szDataSourceID);
  printf("========================================\n");
  printf("\n");
  //  printf("Data source ID\t\t\t:\t%s\n", psuRDataSource->szDataSourceID);
  printf("Data source number\t\t:\t%i\n",psuRDataSource->iDataSourceNum);
  printf("Channel Data type\t\t:\t%s\n",psuRDataSource->szChannelDataType);
  printf("Channel Enabled\t\t\t:\t%i\n",psuRDataSource->bEnabled);
  printf("\n");
  if(psuRDataSource->szAnalogChansPerPkt != NULL)
    printf("Analog Channels/Packet\t\t:\t%i\n",psuAttributes->iAnalogChansPerPkt);
  if(psuRDataSource->szAnalogSampleRate != NULL)
      printf("Analog Sample Rate\t\t:\t%" PRIu64 "\tHz\n",psuAttributes->ullAnalogSampleRate);
  if(psuRDataSource->szAnalogIsDataPacked != NULL)
      printf("Analog Data Packed\t\t:\t%s\n",psuRDataSource->szAnalogIsDataPacked);
  if(psuRDataSource->szAnalogDataLength != NULL)
      printf("Analog Data Length\t\t:\t%" PRIu32 "-bit\n",psuAttributes->ulAnalogDataLength);
  if(psuRDataSource->szAnalogMeasTransfOrd != NULL)    // R-x\AMTO-n-m most significant bit "M", least significant bit "L". default: M
      printf("Analog Meas Transfer Order\t:\t%c\n",psuRDataSource->szAnalogMeasTransfOrd[0]);
  if(psuRDataSource->szAnalogSampleFactor != NULL)
      printf("Analog Sample Factor\t\t:\t%" PRIu32 "\n",psuAttributes->ulAnalogSampleFactor);
  if(psuRDataSource->szAnalogSampleFilter != NULL)
      printf("Analog 3dB Sample Filter\t:\t%" PRIu64 "\tHz\n",psuAttributes->ullAnalogSampleFilter);
  if(psuRDataSource->szAnalogIsDCCoupled != NULL)
      printf("Analog AC/DC Coupling\t\t:\t%cC\n",psuRDataSource->szAnalogIsDCCoupled[0]);
  if(psuRDataSource->szAnalogRecImpedance != NULL)
      printf("Analog Recorder Input Impedance\t:\t%" PRIu32 "\t\tOhms\n",psuAttributes->ulAnalogRecImpedance);
  if(psuRDataSource->szAnalogChanGain != NULL)
      printf("Analog Channel Gain\t\t:\t%" PRIu32 "\t\tmilliunits\n",psuAttributes->ulAnalogChanGain);
  if(psuRDataSource->szAnalogFullScaleRange != NULL)
      printf("Analog Full-Scale Range\t\t:\t%" PRIu32 "\t\tmV\n",psuAttributes->ulAnalogFullScaleRange);
  if(psuRDataSource->szAnalogOffsetVoltage != NULL)
      printf("Analog Offset Voltage\t\t:\t%" PRIi32 "\t\tmV\n",psuAttributes->lAnalogOffsetVoltage);
  if(psuRDataSource->szAnalogLSBValue != NULL)
      printf("Analog LSB Value\t\t:\t%" PRIi32 "\n",psuAttributes->lAnalogLSBValue);
  if(psuRDataSource->szAnalogFormat != NULL)
      printf("Analog Data Format\t\t:\t%c\n",psuRDataSource->szAnalogFormat[0]);
  if(psuRDataSource->szAnalogDifferentialInp != NULL)
      printf("Analog Data Format\t\t:\t%c\n",psuRDataSource->szAnalogDifferentialInp[0]);
  if(psuRDataSource->szAnalogIsAudio != NULL)
      printf("Analog Chan Is Audio\t\t:\t%" PRIi32 "\n",psuAttributes->bAnalogIsAudio);

  return(I106_OK);
}

 
#ifdef __cplusplus
} // end namespace
#endif
