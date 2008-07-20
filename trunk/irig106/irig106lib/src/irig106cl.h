/****************************************************************************

  irig106cl.h - A .NET 2005 class that implements the IRIG 106 library

 Copyright (c) 2007 Irig106.org

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

#pragma once

#include "config.h"
#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_decode_tmats.h"
#include "i106_decode_1553f1.h"


// Drag this stuff in if compiled in .NET environment
#if defined(_M_CEE)
using namespace System;
using namespace System::Text;
#endif

namespace Irig106
    {

    PUBLIC_CLASS class Irig106Lib
        {

        public:
        // Constructor / destructor
        Irig106Lib(void);

//        protected:
        ~Irig106Lib(void);

        public:
            int                 iHandle;
            SuI106Ch10Header  * pHeader;
            void              * pDataBuff;
            unsigned long       ulBuffSize;

        // irig106ch10
        // -----------

        // Open / close
        EnI106Status Open(char * szFilename);
#if defined(_M_CEE)
        EnI106Status Open(String ^ sFilename);
#endif
        EnI106Status Close(void);

        // Read / Write
        EnI106Status ReadNextHeader();
        EnI106Status ReadPrevHeader();
        EnI106Status ReadData();

        //EnI106Status WriteMsg(SuI106Ch10Header  * psuI106Hdr,
        //                      void              * pvBuff)
        //    { return enI106Ch10WriteMsg(this->iHandle, psuI106Hdr, pvBuff); }

        // Move file pointer
        EnI106Status FirstMsg(void)
            { return enI106Ch10FirstMsg(this->iHandle); }

        EnI106Status LastMsg(void)
            { return enI106Ch10LastMsg(this->iHandle); }

        EnI106Status SetPos(int64_t llOffset)
            { return enI106Ch10SetPos(this->iHandle, llOffset); }

        EnI106Status GetPos(int64_t * pllOffset)
            { return enI106Ch10GetPos(this->iHandle, pllOffset); }

//       Utilities 
        //EnI106Status iHeaderInit(SuI106Ch10Header * psuHeader,
        //        unsigned int       uChanID,
        //        unsigned int       uDataType,
        //        unsigned int       uFlags,
        //        unsigned int       uSeqNum);

        //int      iGetHeaderLen(SuI106Ch10Header * psuHeader);

        //int      iGetDataLen(SuI106Ch10Header * psuHeader);

        //uint16_t uCalcHeaderChecksum(SuI106Ch10Header * psuHeader);

        //uint16_t uCalcSecHeaderChecksum(SuI106Ch10Header * psuHeader);

        //uint32_t uCalcDataBuffReqSize(uint32_t uDataLen, int iChecksumType);

        //EnI106Status uAddDataFillerChecksum(SuI106Ch10Header * psuI106Hdr, unsigned char achData[]);

//      i106_time
//      ---------
        EnI106Status Rel2IrigTime(SuIrig106Time  * psuTime)
            { return enI106_Rel2IrigTime(this->iHandle, this->pHeader->aubyRefTime, psuTime); }

        EnI106Status Irig2RelTime(SuIrig106Time  * psuTime,
                                  uint8_t          abyRelTime[])
            { return enI106_Irig2RelTime(this->iHandle, psuTime, abyRelTime); }

        EnI106Status SyncTime(int  bRequireSync=bFALSE,  // Require external time sync
                              int  iTimeLimit=0)         // Max scan ahead time in seconds, 0 = no limit
            { return enI106_SyncTime(this->iHandle, bRequireSync, iTimeLimit); }

        char * szTime2String(SuIrig106Time * psuTime)
            { return IrigTime2String(psuTime); }

#if defined(_M_CEE)
        String ^ strTime2String(SuIrig106Time * psuTime);
#endif

//      i106_decode_time
//      ----------------


//      i106_decode_tmats
//      -----------------

        EnI106Status Decode_Tmats(SuTmatsInfo * psuTmatsInfo)
            { return enI106_Decode_Tmats(this->pHeader, this->pDataBuff, psuTmatsInfo); }

//      i106_decode_1553f1
//      ------------------

        EnI106Status Decode_First1553F1(Su1553F1_CurrMsg * psuMsg)
            { return enI106_Decode_First1553F1(this->pHeader, this->pDataBuff, psuMsg); }

        EnI106Status Decode_Next1553F1(Su1553F1_CurrMsg * psuMsg)
            { return enI106_Decode_Next1553F1(psuMsg); }

//        int i1553WordCnt(const SuCmdWordU * psuCmdWord);

        }; // end ClIrig106 class
    } // end Irig106 namespace

/*
ms-help://MS.VSCC.v80/MS.MSDN.v80/MS.WIN32COM.v10.en/dncomg/html/msdn_cpptocom.htm#dbcppdll

*/
