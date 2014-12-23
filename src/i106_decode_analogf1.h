/****************************************************************************

 i106_decode_analogf1.h - 

 Copyright (c) 2008 Irig106.org

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
 Brought to life by Spencer Hatch in Andøya, Norge, NOV 2014

 ****************************************************************************/

/*
TO DO:
  
*For each data packet, write relevant samples to subchan buff
----measure location using SubChBytesRead, i.e., you are currently at
    (uint8_t *)pauSubData + SubChBytesRead

*At end of data packet, write "SubChBytesRead" to SubChOutFile, flush SubChBuffer, set BytesRead to zero, continue along

 */


#ifndef _I106_DECODE_ANALOGF1_H
#define _I106_DECODE_ANALOGF1_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif


/*
 * Macros and definitions
 * ----------------------
 */

#define ANALOG_MAX_SUBCHANS 256

typedef enum
{
    ANALOG_PACKED                 = 0,
    ANALOG_UNPACKED_LSB_PADDED    = 1,
    ANALOG_RESERVED               = 2,
    ANALOG_UNPACKED_MSB_PADDED    = 3,
} ANALOG_MODE;

typedef enum 
{
    ANALOG_MSB_FIRST       = 0,
    ANALOG_LSB_FIRST       = 1,
} ANALOG_BIT_TRANSFER_ORDER;

typedef enum
{
  ANALOG_FMT_ONES          = 0,
  ANALOG_FMT_TWOS          = 1,
  ANALOG_FMT_SIGNMAG_0     = 2,
  ANALOG_FMT_SIGNMAG_1     = 3,
  ANALOG_FMT_OFFSET_BIN    = 4,
  ANALOG_FMT_UNSIGNED_BIN  = 5,
  ANALOG_FMT_SINGLE_FLOAT  = 6,
} ANALOG_FORMAT;   // R-x\AF-n-m

/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Channel-specific data word
// --------------------------

typedef PUBLIC struct AnalogF1_ChanSpec_S
    {
    uint32_t    uMode           :  2;      // 
    uint32_t    uLength         :  6;      // Bits in A/D value
    uint32_t    uSubChan        :  8;      // Subchannel number
    uint32_t    uTotChan        :  8;      // Total number of subchannels
    uint32_t    uFactor         :  4;      // Sample rate exponent
    uint32_t    bSame           :  1;      // One/multiple Channel Specific
    uint32_t    iReserved       :  3;      //
#if !defined(__GNUC__)
    } SuAnalogF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuAnalogF1_ChanSpec;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

// Subchannel information structure
// --------------------------------
  
typedef struct AnalogF1_SubChan_S
    {
    uint32_t             uChanID;        // Overall channel ID

    SuAnalogF1_ChanSpec *psuChanSpec;    // CSDW corresponding to subchan

    unsigned int         uSubBytesRead;// Number of bytes read for subchan
    uint8_t             *pauSubData;     // Pointer to the start of the data
      //    uint32_t             ulSubDataLen;      // Overall number of subchan bytes in current packet

    char                 szSubChOutFile[256]; // Subchan output filename
    FILE                *psuSubChOutFile;// Subchan output file handle
      
#if !defined(__GNUC__)
    } SuAnalogF1_SubChan;
#else
    } __attribute__ ((packed)) SuAnalogF1_SubChan;
#endif  

// Channel attributes
typedef struct AnalogF1_Attributes_S
    {
    SuRDataSource * psuRDataSrc;            // Pointer to the corresponding RDataSource
    int             iDataSourceNum;             // R-x

    char          * szDataSourceID;         //

    int             iTrackNumber;           // Only valid if szTrackNumber != NULL
    int             iPhysicalChanNumber;
    int             bEnabled;               // Only valid if szEnabled != NULL
    char          * szBusDataLinkName;      // R-x\BDLN-n (-04, -05)
    char          * szChanDataLinkName;     // R-x\CDLN-n (-07, -09)
    int             iAnalogDataTypeFormat;  // (R-x\ATF-n)
    int             iAnalogChansPerPkt;     // (R-1\ACH\N-n) //ORIG
    uint64_t        ullAnalogSampleRate;    // (R-1\ASR-n)   //ORIG

    uint32_t        bAnalogIsDataPacked;    // (R-1\ADP-n)   //ORIG
    char          * szAnalogMeasurementNam; // (R-x\AMN-n-m)
    uint32_t        ulAnalogDataLength;     // (R-x\ADL-n-m)
    char          * szAnalogBitMask;        // (R-x\AMSK-n-m)
    //    char          * szAnalogMeasTransfOrd;  // 
    uint32_t        ulAnalogMeasTransfOrd;  // Msb (0)/ LSB (1, unsupported) R-x\AMTO-n-m
    uint32_t        ulAnalogSampleFactor;   // (R-x\ASF-n-m)
    uint64_t        ullAnalogSampleFilter;  // (R-x\ASBW-n-m)
    uint32_t        bAnalogIsDCCoupled;     // D (0) / A (1)  R-x\ACP-n-m
    uint32_t        ulAnalogRecImpedance;   // (R-x\AII-n-m)
    int32_t         ulAnalogChanGain;        // (R-x\AGI-n-m)
    uint32_t        ulAnalogFullScaleRange; // (R-x\AFSI-n-m)
    int32_t         lAnalogOffsetVoltage;   // (R-x\AOVI-n-m)
    int32_t         lAnalogLSBValue;        // (R-x\ALSV-n-m)
    /* char          * szAnalogEUCSlope;       // (R-x\AECS-n-m) */
    /* char          * szAnalogEUCOffset;      // (R-x\AECO-n-m) */
    /* char          * szAnalogEUCUnits;       // (R-x\AECU-n-m) */
    uint32_t        ulAnalogFormat;         // (R-x\AF-n-m)
    uint32_t        bAnalogDifferentialInp;      // (R-x\AIT-n-m)
    uint32_t        bAnalogIsAudio;         // (R-x\AV-n-m)
    uint32_t        ulAnalogAudioFormat;    // (R-x\AVF-n-m)      
      
    // Stuff from SuRRecord
    uint32_t        ulNumDataSources;       // R-x\N
    uint32_t        bIndexEnabled;          // R-x\ID
    uint32_t        bEventsEnabled;         //R-x\EV\E

    // Computed values 

    int32_t         bPrepareNextDecodingRun;            // First bit flag for a complete decoding run: preload a minor frame sync word to the test word

      //The possibility exists for multiple CSDWs and we want to keep a running copy of them, which we do with a subchannel structure
    SuAnalogF1_SubChan * apsuSubChan[256]; //256 is max number of subchannels


    // The buffer consists of two parts: A data buffer and an error buffer
    int32_t     ulOutBufSize;               // Size of the output buffer in bytes
    uint8_t     * paullOutBuf;              // Contains the data
    uint8_t     * pauOutBufErr;             // Contains aberrant data

    // Variables for bit decoding

    int32_t     lSaveData;                  // Save the data (0: do nothing, 1 save, 2: save terminated)


#if !defined(__GNUC__)
    } SuAnalogF1_Attributes;
#else
    } __attribute__ ((packed)) SuAnalogF1_Attributes;
#endif


// Current Analog message 
typedef struct
    {
        SuI106Ch10Header       * psuHeader;        // The overall packet header
        SuAnalogF1_ChanSpec    * psuChanSpec;      // Header(s) in the data stream
        SuAnalogF1_Attributes  * psuAttributes;    // Pointer to analog-channel attributes structure, with most (all?) values imported from TMATS


        uint32_t                 ulBytesRead;            // Number of bytes read in this message
        uint32_t                 ulDataLen;             // Overall data packet length (in bytes)

        uint8_t                * pauData;             // Pointer to the start of the data
        SuTimeRef                suTimeRef;

#if !defined(__GNUC__)
    } SuAnalogF1_CurrMsg;
#else
    } __attribute__ ((packed)) SuAnalogF1_CurrMsg;
#endif


/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Setup_AnalogF1(SuI106Ch10Header * psuHeader, void * pvBuff, SuAnalogF1_CurrMsg * psuMsg);

  EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstAnalogF1(SuI106Ch10Header * psuHeader, void * pvBuff, SuAnalogF1_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextAnalogF1(SuAnalogF1_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    DecodeBuff_AnalogF1(SuAnalogF1_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    Set_Attributes_AnalogF1(SuRDataSource * psuRDataSrc, SuAnalogF1_Attributes * psuAttributes);

EnI106Status I106_CALL_DECL 
    CreateOutputBuffers_AnalogF1(SuAnalogF1_Attributes * psuAttributes, uint32_t ulDataLen);

EnI106Status  I106_CALL_DECL
    FreeOutputBuffers_AnalogF1(SuAnalogF1_Attributes * psuAttributes);

// Help functions
EnI106Status I106_CALL_DECL
    SwapShortWords_AnalogF1(uint16_t *puBuffer, long nBytes);

EnI106Status I106_CALL_DECL
    PrintCSDW_AnalogF1(SuAnalogF1_ChanSpec *psuChanSpec);

EnI106Status I106_CALL_DECL
PrintAttributesfromTMATS_AnalogF1(SuRDataSource * psuRDataSource, SuAnalogF1_Attributes *psuAttributes, FILE * psuOutFile);

#ifdef __cplusplus
} // end namespace
} // end extern c  
#endif

#endif // _I106_DECODE_ANALOGF1_H
