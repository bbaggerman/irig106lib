/****************************************************************************

 i106_decode_tmats_r.c - Decode TMATS R fields

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <assert.h>

#include "config.h"
#include "i106_stdint.h"

#include "irig106ch10.h"
#include "i106_decode_tmats_r.h"
//#include "i106_decode_tmats_common.h"
//#include "i106_decode_tmats.h"

//#include "sha-256.h"

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

// Make the routines for looking up records by index number
MAKE_GetRecordByIndex(SuRRecord)
MAKE_GetRecordByIndex(SuRDataSource)
MAKE_GetRecordByIndex(SuRModuleInfo)
MAKE_GetRecordByIndex(SuRRmmInfo)
MAKE_GetRecordByIndex(SuREthernetInfo)
MAKE_GetRecordByIndex(SuREthernetPortInfo)
MAKE_GetRecordByIndex(SuRChannelGroup)
MAKE_GetRecordByIndex(SuRChannelGroupChannels)
MAKE_GetRecordByIndex(SuRDrivesVolumes)
MAKE_GetRecordByIndex(SuRDriveVolume)
MAKE_GetRecordByIndex(SuRDriveVolumeLinks)
MAKE_GetRecordByIndex(SuREthernetPubLinks)
MAKE_GetRecordByIndex(SuRRecordingEvent)
MAKE_GetRecordByIndex(SuREventMeasurement)
MAKE_GetRecordByIndex(SuRPcmMinorFrameFilter)
MAKE_GetRecordByIndex(SuRPcmMeasurementOverwrite)
MAKE_GetRecordByIndex(SuR1553MsgFilter)
MAKE_GetRecordByIndex(SuR1553Overwrite)
MAKE_GetRecordByIndex(SuRAnalogSubchannel)
MAKE_GetRecordByIndex(SuRDiscreteMeasurement)
MAKE_GetRecordByIndex(SuRArinc429Subchannel)
MAKE_GetRecordByIndex(SuRUartSubchannel)
MAKE_GetRecordByIndex(SuRMsgSubchannel)
MAKE_GetRecordByIndex(SuREthernetNetwork)
MAKE_GetRecordByIndex(SuRCanBusSubchannel)
MAKE_GetRecordByIndex(SuRRefTracks)


 /* -----------------------------------------------------------------------
 * R Records
 * ----------------------------------------------------------------------- 
 */

// R record specific decode macros
// -------------------------------

// Decode an R record
#define DECODE_R(pattern, field)                                                \
    DECODE(pattern, psuRRec->field)

// Data Source linked list items
#define DECODE_R_DS(pattern, field)                                             \
    DECODE_1(pattern, field, psuRRec->psuFirstDataSource, SuRDataSource)

// Module linked list items
#define DECODE_R_MODULE(pattern, field)                                         \
    DECODE_1(pattern, field, psuRRec->psuFirstModuleInfo, SuRModuleInfo)

// RMM linked list items
#define DECODE_R_RMM(pattern, field)                                            \
    DECODE_1(pattern, field, psuRRec->psuFirstRmmInfo, SuRRmmInfo)

// Ethernet Interface linked list items
#define DECODE_R_ETHERNET(pattern, field)                                       \
    DECODE_1(pattern, field, psuRRec->psuFirstEthernetInfo, SuREthernetInfo)

// Channel Grup Streams linked list items
#define DECODE_R_CHANNEL_GROUP(pattern, field)                                  \
    DECODE_1(pattern, field, psuRRec->psuFirstChannelGroup, SuRChannelGroup)

// Drives and volumes linked list items
#define DECODE_R_DRIVES_VOLUMES(pattern, field)                                 \
    DECODE_1(pattern, field, psuRRec->psuFirstDrivesVolumes, SuRDrivesVolumes)

// Drives and volumes links linked list items
#define DECODE_R_DRIVES_VOLUMES_LINKS(pattern, field)                           \
    DECODE_1(pattern, field, psuRRec->psuFirstDriveVolumeLinks, SuRDriveVolumeLinks)

// Drives and volumes links linked list items
#define DECODE_R_ETHERNET_PUB_LINKS(pattern, field)                             \
    DECODE_1(pattern, field, psuRRec->psuFirstEthernetLink, SuREthernetPubLinks)

// Drives and volumes links linked list items
#define DECODE_R_RECORDING_EVENT(pattern, field)                                \
    DECODE_1(pattern, field, psuRRec->suRecordingEvents.psuFirstRecordingEvent, SuRRecordingEvent)

// 1553 filters
#define DECODE_R_1553_FILTER(pattern, field)                                    \
    DECODE_2(pattern, field, psuRRec->psuFirstDataSource, SuRDataSource, su1553.psuFirstMsgFilter, SuR1553MsgFilter)

// 1553 overwrite
#define DECODE_R_1553_OVERWRITE(pattern, field)                                 \
    DECODE_2(pattern, field, psuRRec->psuFirstDataSource, SuRDataSource, su1553.psuFirst1553OverwriteDef, SuR1553Overwrite)

// Analog subchannels
#define DECODE_R_ANALOG_SUBCHAN(pattern, field)                                 \
    DECODE_2(pattern, field, psuRRec->psuFirstDataSource, SuRDataSource, suAnalog.psuFirstAnalogSubchannel, SuRAnalogSubchannel)

// Discrete measurements
#define DECODE_R_DISC_MESS(pattern, field)                                      \
    DECODE_2(pattern, field, psuRRec->psuFirstDataSource, SuRDataSource, suDiscrete.psuFirstDiscreteMeasurement, SuRDiscreteMeasurement)

// ARINC 429 subchannels
#define DECODE_R_429_SUB(pattern, field)                                        \
    DECODE_2(pattern, field, psuRRec->psuFirstDataSource, SuRDataSource, suArinc429.psuFirstSubchannel, SuRArinc429Subchannel)

// UART subchannels
#define DECODE_R_UART_SUBCHAN(pattern, field)                                   \
    DECODE_2(pattern, field, psuRRec->psuFirstDataSource, SuRDataSource, suUART.psuFirstUartSubchannel, SuRUartSubchannel)

// Message packet subchannels
#define DECODE_R_MSG_SUBCHAN(pattern, field)                                    \
    DECODE_2(pattern, field, psuRRec->psuFirstDataSource, SuRDataSource, suMessage.psuFirstMsgSubchannel, SuRMsgSubchannel)

// Ethernet network subchannels
#define DECODE_R_ETH_SUBCHAN(pattern, field)                                    \
    DECODE_2(pattern, field, psuRRec->psuFirstDataSource, SuRDataSource, suEthernet.psuFirstNetwork, SuREthernetNetwork)

// Ethernet network subchannels
#define DECODE_R_CAN_SUBCHAN(pattern, field)                                    \
    DECODE_2(pattern, field, psuRRec->psuFirstDataSource, SuRDataSource, suCAN.psuFirstCanBusSubchannel, SuRCanBusSubchannel)


/* ----------------------------------------------------------------------- */

int bDecodeRLine(char * szCodeName, char * szDataItem, SuRRecord ** ppsuFirstRRecord)
    {
    SuRRecord     * psuRRec;

    char            szCodeName2[2000];
    char            szCodeField[2000];
    char          * pcChar;
    int             iRIdx;
    int             iTokens;
    int             iIndex1, iIndex2, iIndex3, iIndex4;

    // Parse to get the R record index number, the R record, and the rest of the line
    iTokens = sscanf(szCodeName, "%*1c-%i\\%s", &iRIdx, szCodeName2);
    if (iTokens == 2)
        {
        psuRRec = psuGetRecordByIndex_SuRRecord(ppsuFirstRRecord, iRIdx, bTRUE);
        assert(psuRRec != NULL);
        }
    else
        return 1;

    // Break the rest of the line apart for further analysis
    for (pcChar=szCodeName2; *pcChar!='\0'; pcChar++)
        if (*pcChar == '-') *pcChar = ' ';
    iTokens = sscanf(szCodeName2, "%s %d %d %d %d", szCodeField, &iIndex1, &iIndex2, &iIndex3, &iIndex4);

    if (bFALSE) {}                              // Keep macro logic happy
    
    DECODE_R(ID, szDataSourceID)                // ID - Data source ID
    DECODE_R(RID, szRecorderID)                 // RID - Recorder ID
    DECODE_R(R1, szRecorderDescription)         // R1 - Recorder description

    // Media Characteristics
    DECODE_R(TC1, szMediaType)                  // TC1 - Media type
    DECODE_R(TC2, szMediaMfg)                   // TC2 - Media manufacturer
    DECODE_R(TC3, szMediaCode)                  // TC3 - Media code
    DECODE_R(RML, szMediaLocation)              // RML - Media location
    DECODE_R(ERBS, szExRmmBusSpeed)             // ERBS - External RMM bus speed
    DECODE_R(TC4, szTapeWidth)                  // TC4 - Tape width
    DECODE_R(TC5, szTapeHousing)                // TC5 - Tape housing
    DECODE_R(TT, szTypeOfTracks)                // TT - Type of tracks
    DECODE_R(N, szNumDataSources)               // N - Number of data sources
    DECODE_R(TC6, szRecordSpeed)                // TC6 - Record speed
    DECODE_R(TC7, szDataPackingDensity)         // TC7 - Data packing density
    DECODE_R(TC8, szTapeRewound)                // TC8 - Tape rewound
    DECODE_R(NSB, szNumSourceBits)              // NSB - Number of source bits

    // Recorder Information
    DECODE_R(RI1, szManufacturer)               // RI1 - Recorder manufacturer
    DECODE_R(RI2, szRecorderModel)              // RI2 - Recorder model
    DECODE_R(RI3, szOriginalRecording)          // RI3 - Original recording flag
    DECODE_R(RI4, szRecordingDateTime)          // RI4 - Original recording date and time

    DECODE_R(POC1, suCreatingOrg.szName)        // POC1 - Creating organization name
    DECODE_R(POC1, suCreatingOrg.szAgency)      // POC2 - Creating organization agency
    DECODE_R(POC1, suCreatingOrg.szAddress)     // POC3 - Creating organization address
    DECODE_R(POC1, suCreatingOrg.szTelephone)   // POC4 - Creating organization phone

    DECODE_R(POC1, suCopyingOrg.szName)         // DPOC1 - Copying organization name
    DECODE_R(POC1, suCopyingOrg.szAgency)       // DPOC2 - Copying organization agency
    DECODE_R(POC1, suCopyingOrg.szAddress)      // DPOC3 - Copying organization address
    DECODE_R(POC1, suCopyingOrg.szTelephone)    // DPOC4 - Copying organization phone

    DECODE_R(POC1, suModifyingOrg.szName)       // MPOC1 - Modifying organization name
    DECODE_R(POC1, suModifyingOrg.szAgency)     // MPOC2 - Modifying organization agency
    DECODE_R(POC1, suModifyingOrg.szAddress)    // MPOC3 - Modifying organization address
    DECODE_R(POC1, suModifyingOrg.szTelephone)  // MPOC4 - Modifying organization phone

    DECODE_R(CRE, szContRecordEnable)           // CRE - Continuous record enable
    DECODE_R(RSS, szRecorderSetupSource)        // RSS - Recorder setup source
    DECODE_R(RI9, szRecorderSerialNum)          // RI9 - Recorder serial number
    DECODE_R(RI10, szRecorderFirmwareVer)       // RI10 - Recorder firmware version

    // Recorder modules
    DECODE_R(RIM\\N, szNumModules)
    DECODE_R_MODULE(RIMI, szModuleID)
    DECODE_R_MODULE(RIMS, szModuleSerialNum)
    DECODE_R_MODULE(RIMF, szModuleFirmwareVer)

    // Recorder RMMs
    DECODE_R(RMM\\N, szNumRmms)
    DECODE_R_RMM(RMMID, szRmmID)
    DECODE_R_RMM(RMMS,  szRmmSerialNum)
    DECODE_R_RMM(RMMF,  szRmmFirmwareVer)

    // Ethernet interfaces
    DECODE_R(EI\\N, szNumEthernet)
    DECODE_R_ETHERNET(EINM, szInterfaceName)
    DECODE_R_ETHERNET(PEIN, szPhysicalInterface)
    DECODE_R_ETHERNET(EILS, szLinkSpeed)
    DECODE_R_ETHERNET(EIT, szInterfaceType)
    DECODE_R_ETHERNET(EIIP, szIPAddress)
    DECODE_R_ETHERNET(EIIP\\N, szNumPorts)
    DECODE_2(EI\\PA, szPortAddress, psuRRec->psuFirstEthernetInfo, SuREthernetInfo, psuFirstPortInfo, SuREthernetPortInfo)
    DECODE_2(EI\\PT, szPortType,    psuRRec->psuFirstEthernetInfo, SuREthernetInfo, psuFirstPortInfo, SuREthernetPortInfo)

    // Channel Groups Streams
    DECODE_R(CG\\N, szNumChannelGroups)
    DECODE_R_CHANNEL_GROUP(CGNM, szName)
    DECODE_R_CHANNEL_GROUP(CGSN, szStreamNumber)
    DECODE_R_CHANNEL_GROUP(CGCH\\N, szNumGroupChannels)
    DECODE_2(CGCN, szGroupChannelNumber, psuRRec->psuFirstChannelGroup, SuRChannelGroup, psuFirstChanGroupChans, SuRChannelGroupChannels)

    // Drives and volumes
    DECODE_R(DR\\N, szNumDrivesVolumes)
    DECODE_R_DRIVES_VOLUMES(DRNM, szDriveName)
    DECODE_R_DRIVES_VOLUMES(DRN, szDriveNumber)
    DECODE_R_DRIVES_VOLUMES(DRBS, szBlockSize)
    DECODE_R_DRIVES_VOLUMES(DRVL\\N, szNumOfDriveVolumes)
    DECODE_2(VLNM, szVolumeName, psuRRec->psuFirstDrivesVolumes, SuRDrivesVolumes, psuFirstDriveVolume, SuRDriveVolume)
    DECODE_2(VLN, szVolumeNumber, psuRRec->psuFirstDrivesVolumes, SuRDrivesVolumes, psuFirstDriveVolume, SuRDriveVolume)
    DECODE_2(VLBA, szBlocksToAllocate, psuRRec->psuFirstDrivesVolumes, SuRDrivesVolumes, psuFirstDriveVolume, SuRDriveVolume)
    DECODE_2(VLNB, szNumberOfBlocks, psuRRec->psuFirstDrivesVolumes, SuRDrivesVolumes, psuFirstDriveVolume, SuRDriveVolume)

    // Drive and volume links
    DECODE_R(L\\N, szNumDriveVolumeLinks)
    DECODE_R_DRIVES_VOLUMES_LINKS(LNM,  szLinkName)
    DECODE_R_DRIVES_VOLUMES_LINKS(LSNM, szSrcStreamName)
    DECODE_R_DRIVES_VOLUMES_LINKS(LSSN, szSrcStreamNumber)
    DECODE_R_DRIVES_VOLUMES_LINKS(LDDN, szDstDriveNumber)
    DECODE_R_DRIVES_VOLUMES_LINKS(LDVN, szDstVolumeNumber)

    // Ethernet publishing links
    DECODE_R(EPL\\N, szNumEthernetLinks)
    DECODE_R_ETHERNET_PUB_LINKS(EPL\\LNM,   szLinkName)
    DECODE_R_ETHERNET_PUB_LINKS(EPL\\LSNM,  szSrcStreamName)
    DECODE_R_ETHERNET_PUB_LINKS(EPL\\LSSN,  szSrcStreamNumber)
    DECODE_R_ETHERNET_PUB_LINKS(EPL\\LDEIP, szDstIpAddress)
    DECODE_R_ETHERNET_PUB_LINKS(EPL\\LDEPA, szDstPortAddress)

    // Recording Event Definitions
    DECODE_R(EV\\E,   suRecordingEvents.szEnabled)
    DECODE_R(EV\\TK1, suRecordingEvents.szChannelID)
    DECODE_R(EV\\N,   suRecordingEvents.szNumberOfEvents)
    DECODE_R(EV\\IEE, suRecordingEvents.szInternalEnabled)
    DECODE_R_RECORDING_EVENT(EV\\ID,    szID)
    DECODE_R_RECORDING_EVENT(EV\\D,     szDescription)
    DECODE_R_RECORDING_EVENT(EV\\EDP,   szDataProcessingEnabled)
    DECODE_R_RECORDING_EVENT(EV\\T,     szType)
    DECODE_R_RECORDING_EVENT(EV\\P,     szPriority)
    DECODE_R_RECORDING_EVENT(EV\\CM,    szCaptureMode)
    DECODE_R_RECORDING_EVENT(EV\\IC,    szInitialCapture)
    DECODE_R_RECORDING_EVENT(EV\\LC,    szLimitCount)
    DECODE_R_RECORDING_EVENT(EV\\MS,    szTriggerMeasurementSource)
    DECODE_R_RECORDING_EVENT(EV\\MN,    szTriggerMeasurementName)
    DECODE_R_RECORDING_EVENT(EV\\DLN,   szProcessingMeasurementDataLinkName)
    DECODE_R_RECORDING_EVENT(EV\\PM\\N, szNumberOfMeasurements)
    DECODE_2(EV\\PM\\MN,  szName,              psuRRec->suRecordingEvents.psuFirstRecordingEvent, SuRRecordingEvent, psuFirstEventMeasurement, SuREventMeasurement)
    DECODE_2(EV\\PM\\PRE, szPreEventDuration,  psuRRec->suRecordingEvents.psuFirstRecordingEvent, SuRRecordingEvent, psuFirstEventMeasurement, SuREventMeasurement)
    DECODE_2(EV\\PM\\PST, szPostEventDuration, psuRRec->suRecordingEvents.psuFirstRecordingEvent, SuRRecordingEvent, psuFirstEventMeasurement, SuREventMeasurement)

    // Recording Index
    DECODE_R(IDX\\E,   suRecordingIndex.szEnabled)
    DECODE_R(IDX\\TK1, suRecordingIndex.szChannelID)
    DECODE_R(IDX\\IT,  suRecordingIndex.szType)
    DECODE_R(IDX\\ITV, suRecordingIndex.szTimeValue)
    DECODE_R(IDX\\ICV, suRecordingIndex.szCountValue)

    // 1553 Recorder Control
    DECODE_R(MCR\\E,   su1553Control.szEnabled)
    DECODE_R(MCR\\ID,  su1553Control.szChannelID)
    DECODE_R(MCR\\RCT, su1553Control.szType)
    DECODE_R(MCR\\SPM, su1553Control.szStopPauseCmdWord)
    DECODE_R(MCR\\SRM, su1553Control.szStartResumeCmdWord)

    // Data source attributes
    DECODE_R_DS(DSI,  szDataSourceID)           // DSI-n - Data source identifier

    // Data Source Type (DST) is kind of special because it is only in 106-04. After that
    // for some reason things switched to Channel Data Type (CDT). So for convenience if
    // DST is encountered go ahead and put a link to the type value in szChannelDataType
    // so that the application program doesn't have to check TMATS version and figure this
    // all out.
    // DECODE_R_DS(DST,  szDataSourceType)         // DST-n - Data source type (-04)
    if (strcasecmp(szCodeField, "DST") == 0)
        {
        SuRDataSource     * psuCurr;
        if (iTokens == 2)
            {
            psuCurr = psuGetRecordByIndex_SuRDataSource(&(psuRRec->psuFirstDataSource), iIndex1, bTRUE);
            assert(psuCurr != NULL);
            psuCurr->szDataSourceType  = szDataItem;
            psuCurr->szChannelDataType = szDataItem;
            return 0;
            }
        }

    DECODE_R_DS(CDT,  szChannelDataType)
    DECODE_R_DS(TK1,  szTrackNumber)            // TK1-n - Track number / Channel number
    DECODE_R_DS(TK2,  szRecordingTechnique)
    DECODE_R_DS(IDDR, szDerandomization)
    DECODE_R_DS(TK3,  szDataDirection)
    DECODE_R_DS(TK4,  szPhysicalChannelNumber)
    DECODE_R_DS(CHE,  szEnabled)                // CHE-n - Channel Enabled
    DECODE_R_DS(BDLN, szBusDataLinkName)        // BDLN-n - Data Link Name (-04, -05)
    DECODE_R_DS(PDLN, szPcmDataLinkName)        // PDLN-n - PCM Data Link Name (-04, -05)
    DECODE_R_DS(CDLN, szChanDataLinkName)       // CDLN-n - Channel Data Link Name (=> -07)
    DECODE_R_DS(SHTF, szSecondaryHeaderTimeFormat)

    // PCM attributes
    DECODE_R_DS(PDTF, suPCM.szDataTypeFormat)   // PDTF-n - PCM Data Type Format
    DECODE_R_DS(PDP,  suPCM.szDataPacking)      // PDP-n - PCM Data Packing Option
    DECODE_R_DS(RPS,  suPCM.szRecorderPolarity) // RPS-n - PCM Recorder Polarity Setting
    DECODE_R_DS(ICE,  suPCM.szInputClockEdge)   // ICE-n - PCM Input Clock Edge
    DECODE_R_DS(IST,  suPCM.szInputSignalType)  // IST-n - PCM Input Signal Type
    DECODE_R_DS(ITH,  suPCM.szInputThreshold)   // ITH-n - PCM Input Threshold
    DECODE_R_DS(ITM,  suPCM.szInputTermination) // ITM-n - PCM Input Termination
    DECODE_R_DS(PTF,  suPCM.szVideoTypeFormat)  // PTF-n - PCM Video Type Format

    DECODE_R_DS(MFF\\E,   suPCM.szMinorFrameFilterEnabled)
    DECODE_R_DS(POF\\E,   suPCM.szPPOverwriteFilterEnabled)
    DECODE_R_DS(POF\\T,   suPCM.szPPOverwriteFilterType)
    DECODE_R_DS(MFF\\FDT, suPCM.szMinorFrameFilterDefType)

    DECODE_R_DS(MFF\\N, suPCM.szNumOfMinorFrameFilterDefs)
    DECODE_2(MFF\\MFN, szFilteredMinorFrameNum, psuRRec->psuFirstDataSource, SuRDataSource, suPCM.psuFirstPcmMinorFrameFilter, SuRPcmMinorFrameFilter)

    DECODE_R_DS(SMF\\N, suPCM.szNumOfMeasurementOverwriteDefs)
    DECODE_2(SMF\\SMN,  szSelectedMeasurementName, psuRRec->psuFirstDataSource, SuRDataSource, suPCM.psuFirstPcmMeasurementOverwrite, SuRPcmMeasurementOverwrite)
    DECODE_2(SMF\\MFOT, szOverwriteTag,           psuRRec->psuFirstDataSource, SuRDataSource, suPCM.psuFirstPcmMeasurementOverwrite, SuRPcmMeasurementOverwrite)

    // 1553 attributes
    DECODE_R_DS(BTF,      su1553.szDataTypeFormat)
    DECODE_R_DS(MRF\\E,   su1553.szFilteringEnabled)
    DECODE_R_DS(MOF\\T,   su1553.szPostProcessOverFiltEnabled)
    DECODE_R_DS(MFD\\FDT, su1553.szMsgFilterDefinitionType)
    DECODE_R_DS(MFD\\N,   su1553.szNumberOfMessageFilterDefs)
    DECODE_R_1553_FILTER(MFD\\MID,  szMsgNumber)
    DECODE_R_1553_FILTER(MFD\\MT,   szMsgType)
    DECODE_R_1553_FILTER(CWE,       szCommandWordEntry)
    DECODE_R_1553_FILTER(CMD,       szCommandWord)
    DECODE_R_1553_FILTER(MFD\\TRA,  szRTAddress)
    DECODE_R_1553_FILTER(MFD\\TRM,  szTR)
    DECODE_R_1553_FILTER(MFD\\STA,  szSubaddress)
    DECODE_R_1553_FILTER(MFD\\DWC,  szWordCountModeCode)
    DECODE_R_1553_FILTER(RCWE,      szRcvCommandWordEntry)
    DECODE_R_1553_FILTER(RCMD,      szRcvCommandWord)
    DECODE_R_1553_FILTER(MFD\\RTRA, szRT2RTAddress)
    DECODE_R_1553_FILTER(MFD\\RSTA, szRT2RTSubaddress)
    DECODE_R_1553_FILTER(MFD\\RDWC, szRT2RTWordCount)

    DECODE_R_DS(BME\\N, su1553.szNumOfMeasurementOverwriteDefs)
    DECODE_R_1553_OVERWRITE(BME\\SMN , szSelectedMeasurementName)
    DECODE_R_1553_OVERWRITE(BME\\MFOT ,szOverwriteTag)

    // Analog Attributes
    DECODE_R_DS(ATF,    suAnalog.szDataTypeFormat)
    DECODE_R_DS(ACH\\N, suAnalog.szChansPerPkt)             // ACH\N-n - Analog channels per packet
    DECODE_R_DS(ASR,    suAnalog.szSampleRate)              // ASR-n - Analog sample rate
    DECODE_R_DS(ADP,    suAnalog.szDataPacking)             // ADP-n - Analog data packing
    DECODE_R_ANALOG_SUBCHAN(AMCE, szSubchannelEnabled)
    DECODE_R_ANALOG_SUBCHAN(AMCN, szSubchannelNumber)
    DECODE_R_ANALOG_SUBCHAN(AMN,  szMeasurementName)
    DECODE_R_ANALOG_SUBCHAN(ADL,  szDataLength)
    DECODE_R_ANALOG_SUBCHAN(AMSK, szBitMask)
    DECODE_R_ANALOG_SUBCHAN(AMTO, szMeasurementTransferOrder)
    DECODE_R_ANALOG_SUBCHAN(ASF,  szSampleFactor)
    DECODE_R_ANALOG_SUBCHAN(ASBW, szSampleFilter3dbBW)
    DECODE_R_ANALOG_SUBCHAN(ACP,  szACDCCoupling)
    DECODE_R_ANALOG_SUBCHAN(AII,  szInputImpedance)
    DECODE_R_ANALOG_SUBCHAN(AGI,  szChannelGain)
    DECODE_R_ANALOG_SUBCHAN(AFSI, szFullScaleRange)
    DECODE_R_ANALOG_SUBCHAN(AOVI, szOffsetVoltage)
    DECODE_R_ANALOG_SUBCHAN(AF,   szRecordedAnalogFormat)
    DECODE_R_ANALOG_SUBCHAN(AIT,  szInputType)
    DECODE_R_ANALOG_SUBCHAN(AV,   szAudio)
    DECODE_R_ANALOG_SUBCHAN(AVF,  szAudioFormat)

    // Discrete attributes
    DECODE_R_DS(DTF,    suDiscrete.szDataTypeFormat)
    DECODE_R_DS(DMOD,   suDiscrete.szMode)
    DECODE_R_DS(DSR,    suDiscrete.szSampleRate)
    DECODE_R_DS(NDM\\N, suDiscrete.szNumberOfMeasurements)
    DECODE_R_DISC_MESS(DMN,  szMeasurementName)
    DECODE_R_DISC_MESS(DMSK, szBitMask)
    DECODE_R_DISC_MESS(DMTO, szMeasurementTransferOrder)

    // ARINC 429 attributes
    DECODE_R_DS(ABTF, suArinc429.szDataTypeFormat)
    DECODE_R_DS(NAS\\N, suArinc429.szNumberOfSubchannels)
    DECODE_R_429_SUB(ASN, szNumber)
    DECODE_R_429_SUB(ANM, szName)

    // Video attributes
    DECODE_R_DS(VTF, suVideo.szDataTypeFormat)  // VTF-n - Video Data Type Format
    DECODE_R_DS(VXF, suVideo.szEncodeType)      // VXF-n - Video Encoder Type
    DECODE_R_DS(VST, suVideo.szSignalType)      // VST-n - Video Signal Type
    DECODE_R_DS(VSF, suVideo.szSignalFormat)    // VSF-n - Video Signal Format
    DECODE_R_DS(CBR, suVideo.szConstBitRate)    // CBR-n - Video Constant Bit Rate
    DECODE_R_DS(VBR, suVideo.szVarPeakBitRate)  // VBR-n - Video Variable Peak Bit Rate
    DECODE_R_DS(VED, suVideo.szEncodingDelay)   // VED-n - Video Encoding Delay
    DECODE_R_DS(VCO\\OE, suVideo.szOverlayEnabled)      // R-x\VCO\OE-n
    DECODE_R_DS(VCO\\X,  suVideo.szOverlayPosX)         // R-x\VCO\X-n
    DECODE_R_DS(VCO\\Y,  suVideo.szOverlayPosY)         // R-x\VCO\Y-n
    DECODE_R_DS(VCO\\OET, suVideo.szOverlayEventEnabled)// R-x\VCO\OET-n
    DECODE_R_DS(VCO\\OET, suVideo.szOverlayFormat)      // R-x\VCO\OLF-n
    DECODE_R_DS(VCO\\OBG, suVideo.szOverlayBackground)  // R-x\VCO\OBG-n
    DECODE_R_DS(ASI\\ASL, suVideo.szAudioInputLeft)     // R-x\ASI\ASL-n
    DECODE_R_DS(ASI\\ASR, suVideo.szAudioInputRight)    // R-x\ASI\ASR-n
    DECODE_R_DS(VDA, suVideo.szDataAlignment)   // R-x\VDA-n

    // Time attributes
    DECODE_R_DS(TTF, suTime.szDataTypeFormat)
    DECODE_R_DS(TFMT, suTime.szTimeFormat)
    DECODE_R_DS(TSRC, suTime.szTimeSource)

    // Image attributes
    DECODE_R_DS(ITF,  suImage.szDataTypeFormat)
    DECODE_R_DS(ITF,  suImage.szDataTypeFormat)
    DECODE_R_DS(SIT,  suImage.szStillImageType)
    DECODE_R_DS(DIF,  suImage.szDynamicImageFormat)
    DECODE_R_DS(ITSM, suImage.szTimeStampMode)
    DECODE_R_DS(DIAM, suImage.szDynamicImageAcqMode)
    DECODE_R_DS(IFR,  suImage.szFrameRate)
    DECODE_R_DS(PTG,  suImage.szPreTriggerFrames)
    DECODE_R_DS(TOTF, suImage.szTotalFrames)
    DECODE_R_DS(EXP,  suImage.szExposureTime)
    DECODE_R_DS(ROT,  suImage.szSensorRotation)
    DECODE_R_DS(SGV,  suImage.szSensorGainValue)
    DECODE_R_DS(SAG,  suImage.szSensorAutoGain)
    DECODE_R_DS(ISW,  suImage.szSensorWidth)
    DECODE_R_DS(ISH,  suImage.szSensorHeight)
    DECODE_R_DS(MIW,  suImage.szMaxImageWidth)
    DECODE_R_DS(MIH,  suImage.szMaxImageHeight)
    DECODE_R_DS(IW,   suImage.szImageWidth)
    DECODE_R_DS(IH,   suImage.szImageHeight)
    DECODE_R_DS(IOX,  suImage.szImageOffsetX)
    DECODE_R_DS(IOY,  suImage.szImageOffsetY)
    DECODE_R_DS(ILP,  suImage.szLinePitch)
    DECODE_R_DS(IBH,  suImage.szBinningHorizontal)
    DECODE_R_DS(IBV,  suImage.szBinningVertical)
    DECODE_R_DS(IDH,  suImage.szDecimationHorizontal)
    DECODE_R_DS(IDV,  suImage.szDecimationVertical)
    DECODE_R_DS(IRX,  suImage.szReverseX)
    DECODE_R_DS(IRY,  suImage.szReverseY)
    DECODE_R_DS(IPMN, suImage.szPixelDynRangeMin)
    DECODE_R_DS(IPMX, suImage.szPixelDynRangeMax)
    DECODE_R_DS(TIT,  suImage.szTestImageType)

    // UART attributes
    DECODE_R_DS(UTF,   suUART.szDataTypeFormat)
    DECODE_R_DS(NUS\\N, suUART.szNumberOfSubchannels)
    DECODE_R_UART_SUBCHAN(USCN, szNumber)
    DECODE_R_UART_SUBCHAN(UCNM, szName)
    DECODE_R_UART_SUBCHAN(UCR,  szBaudRate)
    DECODE_R_UART_SUBCHAN(UCB,  szBitsPerWord)
    DECODE_R_UART_SUBCHAN(UCP,  szParity)
    DECODE_R_UART_SUBCHAN(UCS,  szStopBits)
    DECODE_R_UART_SUBCHAN(UCIN, szInterface)
    DECODE_R_UART_SUBCHAN(UCBS, szBlockSize)
    DECODE_R_UART_SUBCHAN(UCSL, szSyncWordLength)
    DECODE_R_UART_SUBCHAN(UCSV, szBlockSyncValue)
    DECODE_R_UART_SUBCHAN(UCBR, szBlockRate)

    // Message Data attributes
    DECODE_R_DS(MTF, suMessage.szDataTypeFormat)
    DECODE_R_DS(NMS\\N, suMessage.szNumberofSubchannels)
    DECODE_R_MSG_SUBCHAN(MSCN, szNumber)
    DECODE_R_MSG_SUBCHAN(MCNM, szName)

    // 1394 attributes
    DECODE_R_DS(IETF, su1394.szDataTypeFormat)

    // Parallel attributes
    DECODE_R_DS(PLTF, suParallel.szDataTypeFormat)

    // Ethernet attributes
    DECODE_R_DS(ENTF, suEthernet.szDataTypeFormat)
    DECODE_R_DS(NNET\\N, suEthernet.szNumberOfNetworks)
    DECODE_R_ETH_SUBCHAN(ENBR, szNumber)
    DECODE_R_ETH_SUBCHAN(ENAM, szName)

    // TSPI attributes
    DECODE_R_DS(TDTF, suTSPI.szDataTypeFormat)

    // CAN Bus attributes
    DECODE_R_DS(CBTF, suCAN.szDataTypeFormat)
    DECODE_R_DS(NCB\\N, suCAN.szNumberofSubchannels)
    DECODE_R_CAN_SUBCHAN(CBN, szNumber)
    DECODE_R_CAN_SUBCHAN(CBM, szName)
    DECODE_R_CAN_SUBCHAN(CBBS, szBitRate)

    // Fibre Channel attributes
    DECODE_R_DS(FCTF, suFibre.szDataTypeFormat)
    DECODE_R_DS(FCSP, suFibre.szChannelSpeed)

    // TM Output attributes
    DECODE_R_DS(OSNM,  suTelemetryOut.szStreamName)
    DECODE_R_DS(SID,   suTelemetryOut.szStreamId)
    DECODE_R_DS(HRATE, suTelemetryOut.szConfigHashRate)
    DECODE_R_DS(CRATE, suTelemetryOut.szConfigPacketRate)

//    DECODE_COMMENT(szDataItem, psuRRec->psuFirstDataSource, SuRDataSource)

    // Reference Tracks
    DECODE_R(RT\\N, szNumOfReferenceTracks)
    DECODE_1(RT1, szTrackNumber,        psuRRec->psuFirstRefTrack, SuRRefTracks)
    DECODE_1(RT2, szReferenceFrequency, psuRRec->psuFirstRefTrack, SuRRefTracks)

    return -1;
    } // end bDecodeRLine



#ifdef __cplusplus
} // end namespace i106
#endif
