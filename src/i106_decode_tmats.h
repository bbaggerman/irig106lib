

#ifndef _I106_DECODE_TMATS_H
#define _I106_DECODE_TMATS_H

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

// Current 1553 message
typedef struct
    {
    unsigned int            uMsgNum;
    uint16_t              * pauData;
    } SuTmatsInfo;

// G record
typedef struct
    {
    
    };


/*
// C record - Data Conversion Attribute
typedef struct SuDataConv
    {
    int                     iIndex;
    char                   *pszMeasName;
    struct SuDataConv      *psuNext;
    } SuDataConv_C
*/



// B record - Bus Attributes
typedef struct SuBusesAttr
    {
    int                     iIndex;
    char                   *pszDataLinkName;
    char                   *pszComment;
    struct SuBusesAttr     *psuNext;
    struct SuBusAttr       *psuFirstBusAttr;
    } SuBusAttr_B

typedef struct SuBusAttr
    {
    int                     iBusNum;
    char                   *pszBusName;
    char                   *pszBusType;
    int                     iIndex;
    struct SuBusAttr       *psuNext;
    } SuBusAttr_B

// P record - PCM Format Attributes
typedef struct SuPCMAttr
    {
    int                     iIndex;
    char                   *pszDataLinkName;
    char                   *pszComment;
    struct SuBusesAttr     *psuNext;
    struct SuBusAttr       *psuFirstBusAttr;
    } SuBusAttr_B

// M record - Multiplex/Modulation Attributes
typedef struct SuMuxModAttr
    {
    int                     iIndex;
    char                   *pszDataLinkName;
    char                   *pszBaseBandDLN;
    char                    szBaseBandSigType[4];
    char                   *pszComment;
    struct SuBusesAttr     *psuNext;
    struct SuBusAttr       *psuFirstBusAttr;
    } SuBusAttr_B



/*
 * Function Declaration
 * --------------------
 */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Decode_Tmats(SuI106Ch10Header * psuHeader,
                        void             * pvBuff,
                        unsigned long      iBuffSize,
                        SuTmatsInfo      * psuInfo);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif