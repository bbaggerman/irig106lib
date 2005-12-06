/****************************************************************************

 i106_decode_tmats.c - 

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

 Created by Bob Baggerman

 $RCSfile: i106_decode_tmats.c,v $
 $Date: 2005-12-06 16:36:00 $
 $Revision: 1.4 $

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdint.h"

#include "irig106ch10.h"
#include "i106_decode_tmats.h"




/*
 * Macros and definitions
 * ----------------------
 */

#define CR      (13)
#define LF      (10)

/*
 * Data structures
 * ---------------
 */

// 1553 bus attributes
// -------------------

// Individual bus attributes
typedef struct SuBusAttr
    {
    char              * szDLN;
    int                 iNumBuses;
    struct SuBusAttr  * psuNext;
    } SuBusAttr;


/*
 * Module data
 * -----------
 */

SuBusAttr             * m_psuFirstBus = NULL;


/*
 * Function Declaration
 * --------------------
 */



/* ======================================================================= */

/* The idea behind this routine is to read the TMATS record, parse it, and 
 * put the various data fields into a tree structure that can be used later
 * to find various settings.
 */

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106_Decode_Tmats(SuI106Ch10Header * psuHeader,
                        void             * pvBuff,
                        unsigned long      iBuffSize,
                        SuTmatsInfo      * psuInfo)
    {
    char   *pchInBuff;
    char    szLine[2000];
    int     iLineIdx;
    int     bReadingLine;
    char   *szCodeName;
    char   *szDataItem;

    // Buffer starts past Channel Specific Data
    pchInBuff    = (char *)pvBuff + 4;
    bReadingLine = bTRUE;

    // Loop until we get to the end of the buffer
    while (bTRUE)
        {

        // Fill a local buffer with one line
        // ---------------------------------

        // Initialize input line buffer
        szLine[0] = '\0';
        iLineIdx  = 0;

        // Read from buffer until complete line
        while (bReadingLine == bTRUE)
            {
            if ((*pchInBuff != CR) && (*pchInBuff != LF))
                {
                szLine[iLineIdx] = *pchInBuff;
                if (iLineIdx < 2000)
                  iLineIdx++;
                szLine[iLineIdx] = '\0';;
                } // end if not line terminator

            else
                {
                if (strlen(szLine) != 0)
                    bReadingLine = bFALSE;
                } // end if line terminator

            // Next character from buffer
            pchInBuff++;

            } // end while filling complete line

        // Decode the TMATS line
        // ---------------------

        // Go ahead and split the line into left hand and right hand sides
        szCodeName = strtok(szLine, ":");
        szDataItem = strtok(NULL, "");

        // Determine and decode different TMATS types
        switch (szCodeName[0])
        {
            case 'G' : // General Information
                break;

            case 'B' : // Bus Data Attributes

//m_psuFirstBus
                break;

            case 'R' : // Tape/Storage Source Attributes
                break;

            case 'T' : // Transmission Attributes
                break;

            case 'M' : // Multiplexing/Modulation Attributes
                break;

            case 'P' : // PCM Format Attributes
                break;

            case 'D' : // PCM Measurement Description
                break;

            case 'S' : // Packet Format Attributes
                break;

            case 'A' : // PAM Attributes
                break;

            case 'C' : // Data Conversion Attributes
                break;

            case 'H' : // Airborne Hardware Attributes
                break;

            case 'V' : // Vendor Specific Attributes
                break;

            default :
                break;

            } // end decoding switch

        // Done decoding, get the next input line
        bReadingLine = bTRUE;

        } // end looping forever on reading TMATS buffer

    // 


    return I106_OK;
    }



/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */


/* ------------------------------------------------------------------------ */


