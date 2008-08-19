/****************************************************************************

 i106_decode_discrete.h -

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

#ifndef _I106_DECODE_DISCRETE_H
#define _I106_DECODE_DISCRETE_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */
#define I106CH10_NUM_DISCRETE_INPUTS_PER_STATE    (uint16_t)32


/*
 * Data structures
 * ---------------
 */

/* Discrete Format 1 */

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

// Channel specific header
typedef struct
   {
    uint32_t    uReserved1  :  24;
    uint32_t    uLength     :  5;      // Number of bits in the event
    uint32_t    uReserved2  :  1;
    uint32_t    uAlignment  :  1;      // 0 = lsb, 1 = msb
    uint32_t    uRecState   :  1;      // 0 = date recorded on change, 1 = recorded at time interval
#if !defined(__GNUC__)
    } SuDiscreteF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuDiscreteF1_ChanSpec;
#endif

// Current discrete message
typedef struct
    {
    unsigned int            uBytesRead;
    SuDiscreteF1_ChanSpec * psuChanSpec;
    SuIntraPacketTS       * psuIPTimeStamp;
    uint32_t                uDiscreteData;
#if !defined(__GNUC__)
    } SuDiscreteF1_CurrMsg;
#else
    } __attribute__ ((packed)) SuDiscreteF1_CurrMsg;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL
    enI106_Decode_FirstDiscreteF1(SuI106Ch10Header     * psuHeader,
                                  void                 * pvBuff,
                                  SuDiscreteF1_CurrMsg * psuCurrMsg,
                                  SuTimeRef            * psuTimeRef);

EnI106Status I106_CALL_DECL
    enI106_Decode_NextDiscreteF1(SuI106Ch10Header     * psuHeader,
                                 SuDiscreteF1_CurrMsg * psuCurrMsg,
                                 SuTimeRef            * psuTimeRef);

#ifdef __cplusplus
} // end extern "C"
} // end namespace Irig106
#endif

#endif
