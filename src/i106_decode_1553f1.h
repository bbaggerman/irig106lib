

#ifndef _I106_DECODE_1553F1_H
#define _I106_DECODE_1553F1_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)


/*
 * Macros and definitions
 * ----------------------
 */



/*
 * Data structures
 * ---------------
 */

typedef struct 
    {
    uint16_t    uWC : 5;
    uint16_t    uSA : 5;
    uint16_t    uTR : 1;
    uint16_t    uRT : 5;
    } SuCmdWord;

/* 1553 Format 1 */

// Channel specific header
typedef struct 
    {
    uint32_t    uMsgCnt      : 24;      // Message count
    uint32_t    Reserved     :  6;
    uint32_t    uTTB         :  2;      // Time tag bits
    } Su1553F1_ChanSpec;

// Intra-message header
typedef struct 
    {
    uint8_t     aubyIntPktTime[8];      // Reference time
    uint16_t    Reserved1       : 3;    // Reserved
    uint16_t    bWordError      : 1;
    uint16_t    bSyncError      : 1;
    uint16_t    bWordCntError   : 1;
    uint16_t    Reserved2       : 3;
    uint16_t    bRespTimeout    : 1;
    uint16_t    bFormatError    : 1;
    uint16_t    bRT2RT          : 1;
    uint16_t    bMsgError       : 1;
    uint16_t    iBusID          : 1;
    uint16_t    Reserved3       : 2;
    uint8_t     uGapTime1;
    uint8_t     uGapTime2;
    uint16_t    uMsgLen;
    } Su1553F1_Header;

// Current 1553 message
typedef struct
    {
    unsigned int            uMsgNum;
    Su1553F1_ChanSpec     * psuChanSpec;
    Su1553F1_Header       * psu1553Hdr;
    uint16_t              * puCmdWord1;
    uint16_t              * puCmdWord2;
    uint16_t              * puStatWord1;
    uint16_t              * puStatWord2;
    uint16_t              * pauData;
    } Su1553F1_CurrMsg;


/*
 * Function Declaration
 * --------------------
 */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Decode_First1553F1(SuI106Ch10Header * psuHeader,
                              void             * pvBuff,
                              Su1553F1_CurrMsg * psuMsg);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Decode_Next1553F1(Su1553F1_CurrMsg * psuMsg);

char * szCmdWord(unsigned int iCmdWord);
int i1553WordCnt(const SuCmdWord * psuCmdWord);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif