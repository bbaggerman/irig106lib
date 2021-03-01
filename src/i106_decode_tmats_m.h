/****************************************************************************

 i106_decode_tmats_m.h - Decode TMATS M fields

 Copyright (c) 2020 Irig106.org

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

#ifndef _I106_DECODE_TMATS_M_H
#define _I106_DECODE_TMATS_M_H

#include "i106_decode_tmats_common.h"

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


// M Records
// =========

// M record
// --------

typedef PUBLIC struct SuMRecord_S
    {
    int                         iIndex;                 // M-x
    char                      * szDataSourceID;         // M-x\ID
    char                      * szSignalStuctType;      // M-x\BB1
    char                      * szModulationSense;      // M-x\BB2
    char                      * szCompLPFBandwidth;     // M-x\BB3
    char                      * szBasebandSignalType;   // M-x\BSG1
    char                      * szBasebandLPFBandwidth; // M-x\BSF1
    char                      * szBasebandLPFType;      // M-x\BSF2
    char                      * szBBDataLinkName;       // M-x\BB\DLN
    struct SuPRecord_S        * psuPRecord;             // Corresponding P record
    char                      * szMeasurementName;      // M-x\BB\MN
    struct SuBRecord_S        * psuBRecord;             // Corresponding B record
// TODO - Add subcarriers section
    struct SuMRecord_S        * psuNext;                // Next record in linked list
    } SuMRecord;



/*
 * Function Declaration
 * --------------------
 */

int bDecodeMLine(char * szCodeName, char * szDataItem, SuMRecord ** ppsuFirstMRec);

#ifdef __cplusplus
}
}
#endif


#endif
