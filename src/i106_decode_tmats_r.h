/****************************************************************************

 i106_decode_tmats_r.h - Decode TMATS R fields

 Copyright (c) 2018 Irig106.org

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

#ifndef _I106_DECODE_TMATS_R_H
#define _I106_DECODE_TMATS_R_H

#include "i106_decode_tmats_common.h"
//#include "i106_decode_tmats.h"

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */


/*
 * Data structures
 * ---------------
 */


// R Records
// =========

// R record data source structures
// -------------------------------

// PCM minor frame filter
typedef PUBLIC struct SuRPcmMinorFrameFilter_S
    {
    int                         iIndex;                     // n
    char                      * szFilteredMinorFrameNum;    // R-x\MFF\MFN-n-m
    struct SuRPcmMinorFrameFilter_S * psuNext;
    } SuRPcmMinorFrameFilter;


// PCM Overwrite definitions
typedef PUBLIC struct SuRPcmMeasurementOverwrite_S
    {
    int                         iIndex;                     // m
    char                      * szSelectedMeasurementName;  // R-x\SMF\SMN-n-m
    char                      * szOverwriteTag;             // R-x\SMF\MFOT-n-m
    struct SuRPcmMeasurementOverwrite_S * psuNext;
    } SuRPcmMeasurementOverwrite;

// 1553 message filters
typedef PUBLIC struct SuR1553MsgFilter_S
    {
    int                         iIndex;                 // m
    char                      * szMsgNumber;            // R-x\MFD\MID-n-m
    char                      * szMsgType;              // R-x\MFD\MT-n-m
    char                      * szCommandWordEntry;     // R-x\CWE-n-m
    char                      * szCommandWord;          // R-x\CMD-n-m
    char                      * szRTAddress;            // R-x\MFD\TRA-n-m
    char                      * szTR;                   // R-x\MFD\TRM-n-m
    char                      * szSubaddress;           // R-x\MFD\STA-n-m
    char                      * szWordCountModeCode;    // R-x\MFD\DWC-n-m
    char                      * szRcvCommandWordEntry;  // R-x\RCWE-n-m
    char                      * szRcvCommandWord;       // R-x\RCMD-n-m
    char                      * szRT2RTAddress;         // R-x\MFD\RTRA-n-m
    char                      * szRT2RTSubaddress;      // R-x\MFD\RSTA-n-m
    char                      * szRT2RTWordCount;       // R-x\MFD\RDWC-n-m
    struct SuR1553MsgFilter_S   * psuNext;
    } SuR1553MsgFilter;

typedef PUBLIC struct SuR1553Overwrite_S
    {
    int                         iIndex;                     // m
    char                      * szSelectedMeasurementName;  // R-x\BME\SMN-n-m
    char                      * szOverwriteTag;             // R-x\BME\MFOT-n-m
    struct SuR1553Overwrite_S   * psuNext;
    } SuR1553Overwrite;

// UART subchannels
typedef PUBLIC struct SuRUartSubchannel_S
    {
    int                         iIndex;                 // m
    char                      * szNumber;               // R-x\USCN-n-m
    char                      * szName;                 // R-x\UCNM-n-m
    char                      * szBaudRate;             // R-x\UCR-n-m
    char                      * szBitsPerWord;          // R-x\UCB-n-m
    char                      * szParity;               // R-x\UCP-n-m
    char                      * szStopBits;             // R-x\UCS-n-m
    char                      * szInterface;            // R-x\UCIN-n-m
    char                      * szBlockSize;            // R-x\UCBS-n-m
    char                      * szSyncWordLength;       // R-x\UCSL-n-m
    char                      * szBlockSyncValue;       // R-x\UCSV-n-m
    char                      * szBlockRate;            // R-x\UCBR-n-m
    struct SuRUartSubchannel_S  * psuNext;
    } SuRUartSubchannel;

// Message data packet subchannel
typedef PUBLIC struct SuRMsgSubchannel_S
    {
    int                         iIndex;                 // m
    char                      * szNumber;               // R-x\MSCN-n-m
    char                      * szName;                 // R-x\MCNM-n-m
    struct SuRMsgSubchannel_S * psuNext;
    } SuRMsgSubchannel;

// Ethernet network subchannel
typedef PUBLIC struct SuREthernetNetwork_S
    {
    int                         iIndex;                 // m
    char                      * szNumber;               //ETHERNET NETWORK NUMBER (R-x\ENBR-n-m)
    char                      * szName;                 //ETHERNET NETWORK NAME (R-x\ENAM-n-m)
    struct SuREthernetNetwork_S  * psuNext;
    } SuREthernetNetwork;

// CAN Bus subchannel
typedef PUBLIC struct SuRCanBusSubchannel_S
    {
    int                         iIndex;                 // m
    char                      * szNumber;               //CAN BUS SUB-CHANNEL NUMBER (R-x\CBN-n-m)
    char                      * szName;                 //CAN BUS SUB-CHANNEL NAME (R-x\CBM-n-m)
    char                      * szBitRate;              //CAN BUS BIT RATE (R-x\CBBS-n-m)
    struct SuRCanBusSubchannel_S  * psuNext;
    } SuRCanBusSubchannel;

// Analog subchannel
typedef PUBLIC struct SuRAnalogSubchannel_S
    {
    int                         iIndex;                 // m
    char                      * szSubchannelEnabled;    // R-x\AMCE-n-m
    char                      * szSubchannelNumber;     // R-x\AMCN-n-m
    char                      * szMeasurementName;      // R-x\AMN-n-m
    char                      * szDataLength;           // R-x\ADL-n-m
    char                      * szBitMask;              // R-x\AMSK-n-m
    char                      * szMeasurementTransferOrder; // R-x\AMTO-n-m
    char                      * szSampleFactor;         // R-x\ASF-n-m
    char                      * szSampleFilter3dbBW;    // R-x\ASBW-n-m
    char                      * szACDCCoupling;         // R-x\ACP-n-m
    char                      * szInputImpedance;       // R-x\AII-n-m
    char                      * szChannelGain;          // R-x\AGI-n-m
    char                      * szFullScaleRange;       // R-x\AFSI-n-m
    char                      * szOffsetVoltage;        // R-x\AOVI-n-m
    char                      * szRecordedAnalogFormat; // R-x\AF-n-m
    char                      * szInputType;            // R-x\AIT-n-m
    char                      * szAudio;                // R-x\AV-n-m
    char                      * szAudioFormat;          // R-x\AVF-n-m
    struct SuRAnalogSubchannel_S  * psuNext;
    } SuRAnalogSubchannel;


// Discrete measurements
typedef PUBLIC struct SuRDiscreteMeasurement_S
    {
    int                         iIndex;                     // m
    char                      * szMeasurementName;          // R-x\DMN-n-m
    char                      * szBitMask;                  // R-x\DMSK-n-m
    char                      * szMeasurementTransferOrder; // R-x\DMTO-n-m
    struct SuRDiscreteMeasurement_S  * psuNext;
    } SuRDiscreteMeasurement;

// ARINC 429 subchannel
typedef PUBLIC struct SuRArinc429Subchannel_S
    {
    int                         iIndex;                 // m
    char                      * szNumber;               // R-x\ASN-n-m
    char                      * szName;                 // R-x\ANM-n-m
    struct SuRArinc429Subchannel_S * psuNext;
    } SuRArinc429Subchannel;


// R record data source
typedef PUBLIC struct SuRDataSource_S
    {
    int                         iIndex;                 // n
    char                      * szDataSourceID;         // R-x\DSI-n
    char                      * szDataSourceType;       // R-x\DST-n (-04 only)
    char                      * szChannelDataType;      // R-x\CDT-n
    char                      * szTrackNumber;          // R-x\TK1-n
    char                      * szRecordingTechnique;   // R-x\TK2-n
    char                      * szDerandomization;      // R-x\IDDR-n
    char                      * szDataDirection;        // R-x\TK3-n
    char                      * szPhysicalChannelNumber;// R-x\TK4-n
    char                      * szEnabled;              // R-x\CHE-n
    char                      * szPcmDataLinkName;      // R-x\PDLN-n (-04, -05)
    char                      * szBusDataLinkName;      // R-x\BDLN-n (-04, -05)
    char                      * szChanDataLinkName;     // R-x\CDLN-n ( => -07)
    char                      * szSecondaryHeaderTimeFormat;    // R-x\SHTF-n

    // PCM channel attributes
    struct
        {
        char                  * szDataTypeFormat;       // R-x\PDTF-n
        char                  * szDataPacking;          // R-x\PDP-n
        char                  * szRecorderPolarity;     // R-x\RPS-n
        char                  * szInputClockEdge;       // R-x\ICE-n
        char                  * szInputSignalType;      // R-x\IST-n
        char                  * szInputThreshold;       // R-x\ITH-n
        char                  * szInputTermination;     // R-x\ITM-n
        char                  * szVideoTypeFormat;      // R-x\PTF-n
        char                  * szMinorFrameFilterEnabled;  // R-x\MFF\E-n
        char                  * szPPOverwriteFilterEnabled; // R-x\POF\E-n
        char                  * szPPOverwriteFilterType;    // R-x\POF\T-n
        char                  * szMinorFrameFilterDefType;  // R-x\MFF\FDT-n
        char                       * szNumOfMinorFrameFilterDefs;   // R-x\MFF\N-n
        SuRPcmMinorFrameFilter     * psuFirstPcmMinorFrameFilter;
        char                       * szNumOfMeasurementOverwriteDefs;   // R-x\SMF\N-n
        SuRPcmMeasurementOverwrite * psuFirstPcmMeasurementOverwrite;
        } suPCM;

    // 1553 channel attributes
    struct
        {
        char                  * szDataTypeFormat;               // R-x\BTF-n
        char                  * szFilteringEnabled;             // R-x\MRF\E-n
        char                  * szPostProcessOverFiltEnabled;   // R-x\MOF\T-n
        char                  * szMsgFilterDefinitionType;      // R-x\MFD\FDT-n
        char                  * szNumberOfMessageFilterDefs;    // R-x\MFD\N-n
        SuR1553MsgFilter      * psuFirstMsgFilter;
        char                  * szNumOfMeasurementOverwriteDefs;  //R-x\BME\N-n
        SuR1553Overwrite      * psuFirst1553OverwriteDef;
        } su1553;

    // Analog channel attributes
    struct 
        {
        char                  * szDataTypeFormat;       // R-x\ATF-n
        char                  * szChansPerPkt;          // R-1\ACH\N-n
        char                  * szDataPacking;          // R-1\ADP-n
        char                  * szSampleRate;           // R-1\ASR-n
        SuRAnalogSubchannel   * psuFirstAnalogSubchannel;
        } suAnalog;

    // Discrete channel attributes
    struct
        {
        char                  * szDataTypeFormat;       // R-x\DTF-n
        char                  * szMode;                 // R-x\DMOD-n
        char                  * szSampleRate;           // R-x\DSR-n
        char                  * szNumberOfMeasurements; // R-x\NDM\N-n
        SuRDiscreteMeasurement  * psuFirstDiscreteMeasurement;
        } suDiscrete;

    // ARINC 429 channel attributes
    struct
        {
        char                  * szDataTypeFormat;       // R-x\ABTF-n
        char                  * szNumberOfSubchannels;  // R-x\NAS\N-n
        SuRArinc429Subchannel * psuFirstSubchannel;
        } suArinc429;

    // Video channel attributes
    struct
        {
        char                  * szDataTypeFormat;       // R-x\VTF-n
        char                  * szEncodeType;           // R-x\VXF-n
        char                  * szSignalType;           // R-x\VST-n
        char                  * szSignalFormat;         // R-x\VSF-n
        char                  * szConstBitRate;         // R-x\CBR-n
        char                  * szVarPeakBitRate;       // R-x\VBR-n
        char                  * szEncodingDelay;        // R-x\VED-n
        char                  * szOverlayEnabled;       // R-x\VCO\OE-n
        char                  * szOverlayPosX;          // R-x\VCO\X-n
        char                  * szOverlayPosY;          // R-x\VCO\Y-n
        char                  * szOverlayEventEnabled;  // R-x\VCO\OET-n
        char                  * szOverlayFormat;        // R-x\VCO\OLF-n
        char                  * szOverlayBackground;    // R-x\VCO\OBG-n
        char                  * szAudioInputLeft;       // R-x\ASI\ASL-n
        char                  * szAudioInputRight;      // R-x\ASI\ASR-n
        char                  * szDataAlignment;        // R-x\VDA-n
        } suVideo;

    // Time channel attributes
    struct 
        {
        char                  * szDataTypeFormat;       // R-x\TTF-n
        char                  * szTimeFormat;           // R-x\TFMT-n
        char                  * szTimeSource;           // R-x\TSRC-n
        } suTime;

    // Image channel attributes
    struct
        {
        char                  * szDataTypeFormat;       // R-x\ITF-n
        char                  * szStillImageType;       // R-x\SIT-n
        char                  * szDynamicImageFormat;   // R-x\DIF-n
        char                  * szTimeStampMode;        // R-x\ITSM-n
        char                  * szDynamicImageAcqMode;  // R-x\DIAM-n
        char                  * szFrameRate;            // R-x\IFR-n
        char                  * szPreTriggerFrames;     // R-x\PTG-n
        char                  * szTotalFrames;          // R-x\TOTF-n
        char                  * szExposureTime;         // R-x\EXP-n
        char                  * szSensorRotation;       // R-x\ROT-n
        char                  * szSensorGainValue;      // R-x\SGV-n
        char                  * szSensorAutoGain;       // R-x\SAG-n
        char                  * szSensorWidth;          // R-x\ISW-n
        char                  * szSensorHeight;         // R-x\ISH-n
        char                  * szMaxImageWidth;        // R-x\MIW-n
        char                  * szMaxImageHeight;       // R-x\MIH-n
        char                  * szImageWidth;           // R-x\IW-n
        char                  * szImageHeight;          // R-x\IH-n
        char                  * szImageOffsetX;         // R-x\IOX-n
        char                  * szImageOffsetY;         // R-x\IOY-n
        char                  * szLinePitch;            // R-x\ILP-n
        char                  * szBinningHorizontal;    // R-x\IBH-n
        char                  * szBinningVertical;      // R-x\IBV-n
        char                  * szDecimationHorizontal; // R-x\IDH-n
        char                  * szDecimationVertical;   // R-x\IDV-n
        char                  * szReverseX;             // R-x\IRX-n
        char                  * szReverseY;             // R-x\IRY-n
        char                  * szPixelDynRangeMin;     // R-x\IPMN-n
        char                  * szPixelDynRangeMax;     // R-x\IPMX-n
        char                  * szTestImageType;        // R-x\TIT-n
        } suImage;

    // UART channel attributes
    struct
        {
        char                  * szDataTypeFormat;       // R-x\UTF-n
        char                  * szNumberOfSubchannels;  // R-x\NUS\N-n
        SuRUartSubchannel     * psuFirstUartSubchannel;         
        } suUART;

    // Message channel attributes
    struct
        {
        char                  * szDataTypeFormat;       //MESSAGE DATA TYPE FORMAT (R-x\MTF-n)
        char                  * szNumberofSubchannels;  //NUMBER OF MESSAGE SUBCHANNELS (R-x\NMS\N-n)
        SuRMsgSubchannel      * psuFirstMsgSubchannel;  
        } suMessage;

    // IEEE-1394 channel attributes
    struct
        {
        char                  * szDataTypeFormat;       //IEEE-1394 DATA TYPE FORMAT (R-x\IETF-n)
        } su1394;

    // Parallel channel atttributes
    struct
        {
        char                  * szDataTypeFormat;       //PARALLEL DATA TYPE FORMAT (R-x\PLTF-n)
        } suParallel;

    // Ethernet channel attributes
    struct
        {
        char                  * szDataTypeFormat;       //ETHERNET DATA TYPE FORMAT (R-x\ENTF-n)
        char                  * szNumberOfNetworks;     //NUMBER OF ETHERNET NETWORKS (R-x\NNET\N-n)
        SuREthernetNetwork    * psuFirstNetwork;
        } suEthernet;

    // TSPI/CTS channel attribute
    struct
        {
        char                  * szDataTypeFormat;       // R-x\TDTF-n
        } suTSPI;

    // CAN Bus channel attributes
    struct
        {
        char                  * szDataTypeFormat;       //CAN BUS DATA TYPE FORMAT (R-x\CBTF-n)
        char                  * szNumberofSubchannels;  //NUMBER OF CAN BUS SUBCHANNELS (R-x\NCB\N-n)
        SuRCanBusSubchannel   * psuFirstCanBusSubchannel;
        } suCAN;

    // Fibre Channnel channel attributes
    struct
        {
        char                  * szDataTypeFormat;       // R-x\FCTF-n
        char                  * szChannelSpeed;         // R-x\FCSP-n
        } suFibre;

    // Telemetry Output channnel attributes
    struct
        {
        char                  * szStreamName;           // R-x\OSNM-n
        char                  * szStreamId;             // R-x\SID-n
        char                  * szConfigHashRate;       // R-x\HRATE-n
        char                  * szConfigPacketRate;     // R-x\CRATE-n
        } suTelemetryOut;

    SuComment                 * psuFirstComment;

    struct SuMRecord_S        * psuMRecord;             // Corresponding M record
    struct SuPRecord_S        * psuPRecord;             // Corresponding P record
    struct SuBRecord_S        * psuBRecord;             // Corresponding B record

    struct SuRDataSource_S    * psuNext;
    } SuRDataSource;    


// R record Structures
// -------------------

typedef PUBLIC struct SuRModuleInfo_S
    {
    int                         iIndex;                 // n
    char                      * szModuleID;             // R-x\RIMI-n
    char                      * szModuleSerialNum;      // R-x\RIMS-n
    char                      * szModuleFirmwareVer;    // R-x\RIMF-n
    struct SuRModuleInfo_S    * psuNext;
    } SuRModuleInfo;


typedef PUBLIC struct SuRRmmInfo_S
    {
    int                         iIndex;                 // n
    char                      * szRmmID;                // R-x\RMMID-n
    char                      * szRmmSerialNum;         // R-x\RMMS-n
    char                      * szRmmFirmwareVer;       // R-x\RMMF-n
    struct SuRRmmInfo_S       * psuNext;
    } SuRRmmInfo;


typedef PUBLIC struct SuREthernetInfo_S
    {
    int                         iIndex;                 // n
    char                      * szInterfaceName;        // R-x\EINM-n
    char                      * szPhysicalInterface;    // R-x\PEIN-n
    char                      * szLinkSpeed;            // R-x\EILS-n
    char                      * szInterfaceType;        // R-x\EIT-n
    char                      * szIPAddress;            // R-x\EIIP-n
    char                      * szNumPorts;             // R-x\EIIP\N-n
    struct SuREthernetPortInfo_S * psuFirstPortInfo;
    struct SuREthernetInfo_S     * psuNext;
    } SuREthernetInfo;


typedef PUBLIC struct SuREthernetPortInfo_S
    {
    int                         iIndex;                 // m
    char                      * szPortAddress;          // R-x\EI\PA-n-m
    char                      * szPortType;             // R-x\EI\PT-n-m
    struct SuREthernetPortInfo_S * psuNext;
    } SuREthernetPortInfo;


typedef PUBLIC struct SuRChannelGroup_S
    {
    int                         iIndex;                 // n
    char                      * szName;                 // R-x\CGNM-n
    char                      * szStreamNumber;         // R-x\CGSN-n
    char                      * szNumGroupChannels;     // R-x\CGCN\N-n
    struct SuRChannelGroupChannels_S * psuFirstChanGroupChans; 
    struct SuRChannelGroup_S  * psuNext;
    } SuRChannelGroup;


typedef PUBLIC struct SuRChannelGroupChannels_S
    {
    int                         iIndex;                 // m
    char                      * szGroupChannelNumber;   // R-x\CGCN-m-n
    struct SuRChannelGroupChannels_S * psuNext;
    } SuRChannelGroupChannels;


typedef PUBLIC struct SuRDriveVolume_S
    {
    int                         iIndex;                 // m
    char                      * szVolumeName;           // R-x\VLNM-n-m
    char                      * szVolumeNumber;         // R-x\VLN-n-m
    char                      * szBlocksToAllocate;     // R-x\VLBA-n-m
    char                      * szNumberOfBlocks;       // R-x\VLNB-n-m
    struct SuRDriveVolume_S   * psuNext;
    } SuRDriveVolume;


typedef PUBLIC struct SuRDrivesVolumes_S
    {
    int                         iIndex;                 // n
    char                      * szDriveName;            // R-x\DRNM-n
    char                      * szDriveNumber;          // R-x\DRN-n
    char                      * szBlockSize;            // R-x\DRBS-n
    char                      * szNumOfDriveVolumes;    // R-x\DRVL\N-n
    SuRDriveVolume            * psuFirstDriveVolume;
    struct SuRDrivesVolumes_S * psuNext;
    } SuRDrivesVolumes;


typedef PUBLIC struct SuRDriveVolumeLinks_S
    {
    int                         iIndex;                 // n
    char                      * szLinkName;             // R-x\LNM-n
    char                      * szSrcStreamName;        // R-x\LSNM-n
    char                      * szSrcStreamNumber;      // R-x\LSSN-n
    char                      * szDstDriveNumber;       // R-x\LDDN-n
    char                      * szDstVolumeNumber;      // R-x\LDVN-n
    struct SuRDriveVolumeLinks_S * psuNext;
    } SuRDriveVolumeLinks;


typedef PUBLIC struct SuREthernetPubLinks_S
    {
    int                         iIndex;                 // n
    char                      * szLinkName;             // R-x\EPL\LNM-n
    char                      * szSrcStreamName;        // R-x\EPL\LSNM-n
    char                      * szSrcStreamNumber;      // R-x\EPL\LSSN-n
    char                      * szDstIpAddress;         // R-x\EPL\LDEIP-n
    char                      * szDstPortAddress;       // R-x\EPL\LDEPA-n
    struct SuREthernetPubLinks_S * psuNext;
    } SuREthernetPubLinks;

// Recording Events

typedef PUBLIC struct SuREventMeasurement_S
    {
    int                         iIndex;                 // m
    char                      * szName;                 // R-x\EV\PM\MN-n-m
    char                      * szPreEventDuration;     // R-x\EV\PM\PRE-n-m
    char                      * szPostEventDuration;    // R-x\EV\PM\PST-n-m
    struct SuREventMeasurement_S * psuNext;
    } SuREventMeasurement;


typedef PUBLIC struct SuRRecordingEvent_S
    {
    int                         iIndex;                 // n
    char                      * szID;                   // R-x\EV\ID-n
    char                      * szDescription;          // R-x\EV\D-n
    char                      * szDataProcessingEnabled;  // R-x\EV\EDP-n
    char                      * szType;                 // R-x\EV\T-n
    char                      * szPriority;             // R-x\EV\P-n
    char                      * szCaptureMode;          // R-x\EV\CM-n
    char                      * szInitialCapture;       // R-x\EV\IC-n
    char                      * szLimitCount;           // R-x\EV\LC-n
    char                      * szTriggerMeasurementSource; // R-x\EV\MS-n
    char                      * szTriggerMeasurementName;   // R-x\EV\MN-n
    char                      * szProcessingMeasurementDataLinkName; // R-x\EV\DLN-n
    char                      * szNumberOfMeasurements; // R-x\EV\PM\N-n
    struct SuREventMeasurement_S * psuFirstEventMeasurement;
    struct SuRRecordingEvent_S * psuNext;
    } SuRRecordingEvent;


// Reference Tracks
typedef PUBLIC struct SuRRefTracks_S
    {
    int                         iIndex;                 // n
    char                      * szTrackNumber;          // R-x\RT1-n
    char                      * szReferenceFrequency;   // R-x\RT2-n
    struct SuRRefTracks_S     * psuNext;
    } SuRRefTracks;


// R record
typedef PUBLIC struct SuRRecord_S
    {
    int                         iIndex;                 // R-x
    char                      * szDataSourceID;         // R-x\ID
    char                      * szRecorderID;           // R-x\RID
    char                      * szRecorderDescription;  // R-x\R1

    // Media Characteristics
    char                      * szMediaType;            // R-x\TC1
    char                      * szMediaMfg;             // R-x\TC2
    char                      * szMediaCode;            // R-x\TC3
    char                      * szMediaLocation;        // R-x\RML
    char                      * szExRmmBusSpeed;        // R-x\ERBS
    char                      * szTapeWidth;            // R-x\TC4
    char                      * szTapeHousing;          // R-x\TC5
    char                      * szTypeOfTracks;         // R-x\TT
    char                      * szNumDataSources;       // R-x\N
    char                      * szRecordSpeed;          // R-x\TC6
    char                      * szDataPackingDensity;   // R-x\TC7
    char                      * szTapeRewound;          // R-x\TC8
    char                      * szNumSourceBits;        // R-x\NSB

    // Recorder Information
    char                      * szManufacturer;         // R-x\RI1
    char                      * szRecorderModel;        // R-x\RI2
    char                      * szOriginalRecording;    // R-x\RI3
    char                      * szRecordingDateTime;    // R-x\RI4

    SuPointOfContact            suCreatingOrg;          // R-x\POC
    SuPointOfContact            suCopyingOrg;           // R-x\DPOC
    SuPointOfContact            suModifyingOrg;         // R-x\MPOC

    char                      * szContRecordEnable;     // R-x\CRE
    char                      * szRecorderSetupSource;  // R-x\RSS
    char                      * szRecorderSerialNum;    // R-x\RI9
    char                      * szRecorderFirmwareVer;  // R-x\RI10
    char                      * szNumModules;           // R-x\RIM\N
    SuRModuleInfo             * psuFirstModuleInfo;
    char                      * szNumRmms;              // R-x\RMM\N
    SuRRmmInfo                * psuFirstRmmInfo;
    char                      * szNumEthernet;          // R-x\EI\N
    SuREthernetInfo           * psuFirstEthernetInfo;
    char                      * szNumChannelGroups;     // R-x\CG\N
    SuRChannelGroup           * psuFirstChannelGroup;
    char                      * szNumDrivesVolumes;     // R-x\DR\N
    SuRDrivesVolumes          * psuFirstDrivesVolumes;
    char                      * szNumDriveVolumeLinks;  // R-x\L\N
    SuRDriveVolumeLinks       * psuFirstDriveVolumeLinks;
    char                      * szNumEthernetLinks;     // R-x\EPL\N
    SuREthernetPubLinks       * psuFirstEthernetLink;

    // Recording Event Definitions
    struct
        {
        char                  * szEnabled;              // R-x\EV\E
        char                  * szChannelID;            // R-x\EV\TK1
        char                  * szNumberOfEvents;       // R-x\EV\N
        char                  * szInternalEnabled;      // R-x\EV\IEE
        SuRRecordingEvent     * psuFirstRecordingEvent;
        } suRecordingEvents;

    // Recording Index
    struct
        {
        char                  * szEnabled;              // R-x\IDX\E
        char                  * szChannelID;            // R-x\IDX\TK1
        char                  * szType;                 // R-x\IDX\IT
        char                  * szTimeValue;            // R-x\IDX\ITV
        char                  * szCountValue;           // R-x\IDX\ICV
        } suRecordingIndex;

    // 1553 Recorder Control
    struct
        {
        char                  * szEnabled;              // R-x\MRC\E
        char                  * szChannelID;            // R-x\MRC\ID
        char                  * szType;                 // R-x\MRC\RCT
        char                  * szStopPauseCmdWord;     // R-x\MRC\SPM
        char                  * szStartResumeCmdWord;   // R-x\MRC\SRM
        } su1553Control;

    // Data sources
    SuRDataSource             * psuFirstDataSource;

    // Reference track
    char                      * szNumOfReferenceTracks; // R-x\RT\N
    SuRRefTracks              * psuFirstRefTrack;

    struct SuRRecord_S        * psuNext;                // Next record in linked list
    } SuRRecord;

/*
 * Function Declaration
 * --------------------
 */

int bDecodeRLine(char * szCodeName, char * szDataItem, SuRRecord ** ppsuFirstRRec);

#ifdef __cplusplus
}
}
#endif

#endif
