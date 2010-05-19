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
#include "i106_decode_uart.h"
#include "i106_decode_discrete.h"
#include "i106_decode_index.h"


// Drag this stuff in if compiled in .NET environment
#if defined(_M_CEE)
using namespace System;
using namespace System::Text;
#endif

namespace Irig106
    {

     class Irig106Lib
        {

        public:
        // Constructor / destructor
        Irig106Lib(void);

//        protected:
        ~Irig106Lib(void);

        public:
            int                         iHandle;
            SuI106Ch10Header        *   pHeader;
            void                    *   pDataBuff;
            unsigned long               ulBuffSize;
            int                         bManageDataBuffMalloc;
            SuTmatsInfo                 suTmatsInfo;        // Decoded TMATS info tree
            Su1553F1_CurrMsg        *   psu1553CurrMsg;     // Current 1553 message
            SuTimeRef               *   psuTimeRef;         // Time
            SuDiscreteF1_CurrMsg    *   psuDiscreteCurrMsg; // Current discrete message
            SuUartF0_CurrMsg        *   psuUartCurrMsg;     // Current UART message


        // irig106ch10
        // -----------

        // Open / close
        EnI106Status Open(char * szFilename, EnI106Ch10Mode enMode=I106_READ);
#if defined(_M_CEE)
        EnI106Status Open(String ^ sFilename, EnI106Ch10Mode enMode=I106_READ);
#endif
        EnI106Status Close(void);

        // Read / Write
        EnI106Status ReadNextHeader();
        EnI106Status ReadPrevHeader();
        EnI106Status ReadData();

        EnI106Status WriteMsg()
            { return enI106Ch10WriteMsg(this->iHandle, this->pHeader, this->pDataBuff); }

        EnI106Status WriteMsg(SuI106Ch10Header * pHeader,
                              void             * pDataBuff)
            { return enI106Ch10WriteMsg(this->iHandle, pHeader, pDataBuff); }

        // Move file pointer
        EnI106Status FirstMsg(void)
            { return enI106Ch10FirstMsg(this->iHandle); }

        EnI106Status LastMsg(void)
            { return enI106Ch10LastMsg(this->iHandle); }

        EnI106Status SetPos(int64_t llOffset)
            { return enI106Ch10SetPos(this->iHandle, llOffset); }

        EnI106Status GetPos(int64_t * pllOffset)
            { return enI106Ch10GetPos(this->iHandle, pllOffset); }

        int HeaderInit(unsigned int       uChanID,
                       unsigned int       uDataType,
                       unsigned int       uFlags,
                       unsigned int       uSeqNum)
            { return iHeaderInit(this->pHeader, uChanID, uDataType, uFlags, uSeqNum); }


#if defined(_M_CEE)
        EnI106Status GetPos(int64_t % mpllOffset);
#endif

//       Utilities
        //EnI106Status iHeaderInit(SuI106Ch10Header * psuHeader,
        //        unsigned int       uChanID,
        //        unsigned int       uDataType,
        //        unsigned int       uFlags,
        //        unsigned int       uSeqNum);

        //int      iGetHeaderLen(SuI106Ch10Header * psuHeader);

        //int      iGetDataLen(SuI106Ch10Header * psuHeader);

        uint16_t CalcHeaderChecksum()
            { return uCalcHeaderChecksum(this->pHeader); }

        void SetHeaderChecksum()
            { 
            this->pHeader->uChecksum = this->CalcHeaderChecksum();
            return;
            }

        //uint16_t uCalcSecHeaderChecksum(SuI106Ch10Header * psuHeader);

        uint32_t CalcDataBuffReqSize(uint32_t uDataLen, int iChecksumType)
            { return uCalcDataBuffReqSize(uDataLen, iChecksumType); }

        uint32_t CalcDataBuffReqSize(uint32_t uDataLen)
            { return uCalcDataBuffReqSize(uDataLen, this->pHeader->ubyPacketFlags & I106CH10_PFLAGS_CHKSUM_MASK); }

        uint32_t CalcDataBuffReqSize()
            { return uCalcDataBuffReqSize(this->pHeader->ulDataLen, 
                this->pHeader->ubyPacketFlags & I106CH10_PFLAGS_CHKSUM_MASK); }

        EnI106Status AddDataFillerChecksum(SuI106Ch10Header * psuI106Hdr, unsigned char achData[])
            { return uAddDataFillerChecksum(psuI106Hdr, achData); }

        EnI106Status AddDataFillerChecksum()
            { return uAddDataFillerChecksum(this->pHeader, (unsigned char *)(this->pDataBuff)); }

//        EnI106Status AddDataFillerChecksum(SuI106Ch10Header * psuI106Hdr, unsigned char achData[])
//            { return uAddDataFillerChecksum(); }

//      i106_time
//      ---------
        EnI106Status Rel2IrigTime(SuIrig106Time  * psuTime)
            { return enI106_Rel2IrigTime(this->iHandle, this->pHeader->aubyRefTime, psuTime); }

        EnI106Status Irig2RelTime(SuIrig106Time  * psuTime,
                                  uint8_t          abyRelTime[])
            { return enI106_Irig2RelTime(this->iHandle, psuTime, abyRelTime); }

#if defined(_M_CEE)
        System::Void LLInt2TimeArray(int64_t * pllRelTime, uint8_t   abyRelTime[])
            { vLLInt2TimeArray(pllRelTime, abyRelTime); return; }

        System::Void LLInt2TimeArray(int64_t * pllRelTime)
            { vLLInt2TimeArray(pllRelTime, this->pHeader->aubyRefTime); return; }

        System::Void TimeArray2LLInt(uint8_t   abyRelTime[], int64_t * pllRelTime)
            { vTimeArray2LLInt(abyRelTime, pllRelTime); return; }

        System::Void TimeArray2LLInt(int64_t * pllRelTime)
            { vTimeArray2LLInt(this->pHeader->aubyRefTime, pllRelTime); return; }

        System::Void TimeArray2LLInt(int64_t % mpllRelTime);
#endif

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

        EnI106Status Decode_Tmats();

//      i106_decode_1553f1
//      ------------------

        EnI106Status Decode_First1553F1()
            { return enI106_Decode_First1553F1(this->pHeader, this->pDataBuff, this->psu1553CurrMsg); }

        EnI106Status Decode_Next1553F1()
            { return enI106_Decode_Next1553F1(this->psu1553CurrMsg); }

//      i106_decode_discrete
//      --------------------
        EnI106Status Decode_FirstDiscreteF1()
        { return enI106_Decode_FirstDiscreteF1(this->pHeader, this->pDataBuff, this->psuDiscreteCurrMsg, this->psuTimeRef); }

        EnI106Status Decode_NextDiscreteF1()
        { return enI106_Decode_NextDiscreteF1(this->pHeader, psuDiscreteCurrMsg, psuTimeRef); }


//      i106_decode_uart
//      ----------------
        EnI106Status Decode_FirstUartF0()
        { return enI106_Decode_FirstUartF0(this->pHeader,this->pDataBuff,this->psuUartCurrMsg); }

        EnI106Status Decode_NextUartF0()
        { return enI106_Decode_NextUartF0(this->psuUartCurrMsg) ;}

        }; // end ClIrig106 class

    } // end Irig106 namespace

