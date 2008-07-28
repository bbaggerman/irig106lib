/****************************************************************************

 i106_decode_tmats.h - 

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

 ****************************************************************************/

#ifndef _I106_DECODE_PCM_H
#define _I106_DECODE_PCM_H

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

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Channel specific data word
typedef struct PcmF1_S
    {
    uint32_t    uSyncOffset     : 18;      // Sync offset
    uint32_t    bUnpackedMode   :  1;      // Packed mode flag
    uint32_t    bPackedMode     :  1;      // Unpacked mode flag
    uint32_t    bThruMode       :  1;      // Throughput mode flag
    uint32_t    bAlignment      :  1;      // 16/32 bit alignment flag
    uint32_t    Reserved1       :  2;      // 
    uint32_t    uMajorFrStatus  :  2;      // Major frame lock status
    uint32_t    uMinorFrStatus  :  2;      // Minor frame lock status
    uint32_t    bMinorFrInd     :  1;      // Minor frame indicator
    uint32_t    bMajorFrInd     :  1;      // Major frame indicator
    uint32_t    bIPH            :  1;      // Intra-packet header flag
    uint32_t    Reserved2       :  1;      // 
#if !defined(__GNUC__)
    } SuPcmF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuPcmF1_ChanSpec;
#endif

// Intra-message header
typedef struct PcmF1_Header
    {
    uint64_t    suIntraPckTime;            // Reference time
    uint32_t    Reserved1       : 12;      // 
    uint32_t    uMajorFrStatus  :  2;      // Major frame lock status
    uint32_t    uMinorFrStatus  :  2;      // Minor frame lock status
    uint32_t    Reserved2       : 16;      // 
#if !defined(__GNUC__)
    } SuPcmF1_Header;
#else
    } __attribute__ ((packed)) SuPcmF1_Header;
#endif


#if defined(_MSC_VER)
#pragma pack(pop)
#endif

/*
 * Function Declaration
 * --------------------
 */



#ifdef __cplusplus
}
}
#endif

#endif
