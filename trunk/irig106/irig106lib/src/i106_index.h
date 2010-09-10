/****************************************************************************

 i106_index.h - 

 Copyright (c) 2006 Irig106.org

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

#ifndef _I106_INDEX_H
#define _I106_INDEX_H

#include <string.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "stdint.h"
#include "Irig106Ch10.h"
#include "i106_time.h"
#include "i106_decode_time.h"

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

typedef struct
    {
    uint32_t    uIdxEntCount    : 16;   // Total number of indexes
    uint32_t    uReserved       : 13;
    uint32_t    bIntraPckHdr    :  1;   // Intra-packet header present
    uint32_t    bFileSize       :  1;   // File size present
    uint32_t    uIndexType      :  1;   // Index type
#if !defined(__GNUC__)
    } SuIndex_ChanSpec;
#else
    } __attribute__((packed)) SuIndex_ChanSpec;
#endif

typedef struct
    {
    uint64_t        ui64RelTime;
    SuIrig106Time   suIrigTime;
    uint64_t        ui64FileOffset;
    } SuIndexTableNode;


typedef struct
    {
    int iCurrent;
    } SuRootIndexPacket;

// This struct holds the data of the first 4 bytes in the node index entry layout
typedef struct 
    {
    uint32_t    uChannelID      : 16;
    uint32_t    uDataType       :  8;
    uint32_t    uReserved       :  8;
    //uint32_t u32Offset_LSLW;
    //uint32_t u32Offset_MSLW;
    uint64_t    uOffset;
#if !defined(__GNUC__)
    } SuIndex_Data;
#else
    } __attribute__((packed)) SuIndex_Data;
#endif



/*
 * Global data
 * -----------
 */


/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL enIndexPresent(int iHandle, int * bFoundIndex);

EnI106Status I106_CALL_DECL enReadIndexes(int iHandle);

#if 0

EnI106Status I106_CALL_DECL SaveIndexTable(char* strFileName);

EnI106Status I106_CALL_DECL ProcessRootIndexPackBody( int iHandle, SuI106Ch10Header suHeader );

EnI106Status I106_CALL_DECL ProcessNodeIndexPack(int iHandle, uint64_t u64NodeIndexOffset);

EnI106Status I106_CALL_DECL ProcessNodeIndexPackBody(int iHandle, SuI106Ch10Header suHeader);

EnI106Status ReallocIndexTable();

void DeleteIndexTable();

EnI106Status ReadTimeDataPacketBody(SuI106Ch10Header suTimeDataHeader);
#endif

#ifdef __cplusplus
}
}
#endif

#endif