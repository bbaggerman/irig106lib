/****************************************************************************

 i106_decode_uartf0.h - 

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

#ifndef _I106_DECODE_UARTF0_H
#define _I106_DECODE_UARTF0_H


#ifdef __cplusplus
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
#pragma pack(push)
#pragma pack(1)
#endif

/* UART Format 0 */

// Channel specific header
typedef struct 
    {
    uint32_t    Reserved     : 31;      
    uint32_t    bIPH         :  1;      // Intra-Packet Header enabled    
#if !defined(__GNUC__)
    } SuUartF0_ChanSpec;
#else
    } __attribute__ ((packed)) SuUartF0_ChanSpec;
#endif

// Intra-message header
typedef struct 
    {    
    uint16_t    uDataLength     : 16;    // Length of the UART data in bytes
    uint16_t    uSubchannel     : 14;    // Subchannel for the following data
    uint16_t    uReserved       : 1;
    uint16_t    bParityError    : 1;     //Parity Error    
#if !defined(__GNUC__)
    } SuUartF0_Header;
#else
    } __attribute__ ((packed)) SuUartF0_Header;
#endif

// Current UART message
typedef struct
    {
    unsigned int            uBytesRead;
    SuUartF0_ChanSpec     * psuChanSpec;
    SuIntraPacketTS       * psuIPTimeStamp;
    SuUartF0_IPHeader     * psuUartHdr;            
    uint16_t              * pauData;
    } SuUartF0_CurrMsg;


#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstUartF0(SuI106Ch10Header         * psuHeader,
                              void                     * pvBuff,
                              SuUartF0_CurrMsg         * psuCurrMsg,
                              SuTimeRef                * psuTimeRef);

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextUartF0(SuI106Ch10Header         * psuHeader,
                             SuUartF0_CurrMsg         * psuCurrMsg,
                             SuTimeRef                * psuTimeRef);

#ifdef __cplusplus
}
#endif

#endif
