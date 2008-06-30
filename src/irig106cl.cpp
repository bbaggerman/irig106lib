/****************************************************************************

  irig106cl.cpp - A class that implements the IRIG 106 library

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

#include <stdlib.h>   // For _MAX_PATH definition
#include <stdio.h>
#include <malloc.h>
#include <time.h>

#include "stdint.h"

#include "irig106cl.h"
//#pragma make_public(SuI106Ch10Header)

//using namespace Irig106;

// Drag this stuff in if compiled in .NET environment
#if defined(_M_CEE)
//using namespace System;
//	using namespace System::ComponentModel;
//	using namespace System::Collections;
//	using namespace System::IO;
using namespace System::Text;
using namespace System::Runtime::InteropServices;
#endif

namespace Irig106
{

//=========================================================================

// Constructor / destructor
Irig106Lib::Irig106Lib(void)
    {
    this->pHeader     = (SuI106Ch10Header *)malloc(sizeof(SuI106Ch10Header));
    this->pDataBuff   = NULL;
    this->ulBuffSize  = 0;
    }

Irig106Lib::~Irig106Lib(void) 
    {
//    Close();
    free(this->pHeader);
    free(this->pDataBuff);
    this->ulBuffSize = 0;
    }


//=========================================================================
// irig106ch10

// Open() with C string name
EnI106Status Irig106Lib::Open(char * szFilename)
    {
    EnI106Status    enStatus;

    // Open the data file
    enStatus = enI106Ch10Open(&iHandle, (char *)szFilename, I106_READ);

    return enStatus;
    }



//  Open() with .NET string name
#if defined(_M_CEE)
EnI106Status Irig106Lib::Open(String ^ sFilename)
    {
    EnI106Status    enStatus;
    const char    * szFilename;

    // Convert the filename into a good ol' C string, getting rid of that Unicode junk
    szFilename = (const char *)
        (Marshal::StringToHGlobalAnsi(sFilename)).ToPointer();

    // Make a pointer to the handle    
    pin_ptr<int>piHandle = &iHandle;
    
    // Open the data file
    enStatus = enI106Ch10Open(piHandle, (char *)szFilename, I106_READ);

    // Free up the filename storage space
    Marshal::FreeHGlobal(IntPtr((void*)szFilename));

    return enStatus;
    }
#endif

//-------------------------------------------------------------------------

//  Close
EnI106Status Irig106Lib::Close(void)
    {
    EnI106Status    enStatus;
    enStatus = enI106Ch10Close(iHandle);
    return enStatus;
    }

//-------------------------------------------------------------------------

// Read / Write
EnI106Status Irig106Lib::ReadNextHeader()
    {
    EnI106Status    enStatus;
    enStatus = enI106Ch10ReadNextHeader(iHandle, this->pHeader);
    return enStatus;
    }


//-------------------------------------------------------------------------

EnI106Status Irig106Lib::ReadPrevHeader()
    {
    EnI106Status    enStatus;
    enStatus = enI106Ch10ReadPrevHeader(iHandle, this->pHeader);
    return enStatus;
    }


//-------------------------------------------------------------------------

EnI106Status Irig106Lib::ReadData()
    {
    EnI106Status    enStatus;

    // Make sure the buffer is big enough
    if (this->ulBuffSize < this->pHeader->ulPacketLen)
        {
        this->pDataBuff = realloc(this->pDataBuff, this->pHeader->ulPacketLen);
        this->ulBuffSize = this->pHeader->ulPacketLen;
        }
    enStatus = enI106Ch10ReadData(iHandle, this->ulBuffSize, this->pDataBuff);

    return enStatus;
    }



//-------------------------------------------------------------------------

/*
    //// Utilities
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
*/




//=========================================================================
}
