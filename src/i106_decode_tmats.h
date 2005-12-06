/****************************************************************************

 i106_decode_tmats.h - 

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

   * Neither the name of the Georgia Tech Applied Research Corporation 
     nor the names of its contributors may be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.

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

 $RCSfile: i106_decode_tmats.h,v $
 $Date: 2005-12-06 16:31:23 $
 $Revision: 1.2 $

 ****************************************************************************/

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