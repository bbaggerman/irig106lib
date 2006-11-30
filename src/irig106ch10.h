/****************************************************************************

  irig106ch10.h - 

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

 $RCSfile: irig106ch10.h,v $
 $Date: 2006-11-30 02:38:44 $
 $Revision: 1.11 $

 ****************************************************************************/

#ifndef _irig106ch10_h_
#define _irig106ch10_h_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */

#define I106_DLL_DECLSPEC
#define I106_CALL_DECL

#if !defined(bTRUE)
#define bTRUE       (1==1)
#define bFALSE      (1==0)
#endif

#define MAX_HANDLES         4

#define IRIG106_SYNC        0xEB25

// Define the longest file path string size
#undef  MAX_PATH
#define MAX_PATH                       260

// Header and secondary header sizes
#define HEADER_SIZE         24
#define SEC_HEADER_SIZE     12

// Header packet flags
#define I106CH10_PFLAGS_CHKSUM_NONE    (uint8_t)0x00
#define I106CH10_PFLAGS_CHKSUM_8       (uint8_t)0x01
#define I106CH10_PFLAGS_CHKSUM_16      (uint8_t)0x02
#define I106CH10_PFLAGS_CHKSUM_32      (uint8_t)0x03
#define I106CH10_PFLAGS_OVERFLOW       (uint8_t)0x10
#define I106CH10_PFLAGS_TIMESYNCERR    (uint8_t)0x20
#define I106CH10_PFLAGS_SEC_HEADER     (uint8_t)0x80

// Header data types
#define I106CH10_DTYPE_COMPUTER_0      (uint8_t)0x00
#define I106CH10_DTYPE_USER_DEFINED    (uint8_t)0x00
#define I106CH10_DTYPE_COMPUTER_1      (uint8_t)0x01
#define I106CH10_DTYPE_TMATS           (uint8_t)0x01
#define I106CH10_DTYPE_COMPUTER_2      (uint8_t)0x02
#define I106CH10_DTYPE_RECORDING_EVENT (uint8_t)0x02
#define I106CH10_DTYPE_COMPUTER_3      (uint8_t)0x03
#define I106CH10_DTYPE_RECORDING_INDEX (uint8_t)0x03
#define I106CH10_DTYPE_COMPUTER_4      (uint8_t)0x04
#define I106CH10_DTYPE_COMPUTER_5      (uint8_t)0x05
#define I106CH10_DTYPE_COMPUTER_6      (uint8_t)0x06
#define I106CH10_DTYPE_COMPUTER_7      (uint8_t)0x07
#define I106CH10_DTYPE_PCM             (uint8_t)0x09
#define I106CH10_DTYPE_IRIG_TIME       (uint8_t)0x11
#define I106CH10_DTYPE_1553_FMT_1      (uint8_t)0x19
#define I106CH10_DTYPE_ANALOG          (uint8_t)0x21
#define I106CH10_DTYPE_DISCRETE        (uint8_t)0x29
#define I106CH10_DTYPE_MESSAGE         (uint8_t)0x31
#define I106CH10_DTYPE_ARINC_429       (uint8_t)0x39
#define I106CH10_DTYPE_MPEG2           (uint8_t)0x40
#define I106CH10_DTYPE_IMAGE           (uint8_t)0x41
#define I106CH10_DTYPE_UART            (uint8_t)0x50

// Error return codes
typedef enum
    {
    I106_OK                 =  0,   // Everything okey dokey
    I106_OPEN_ERROR         =  1,   // Fatal problem opening for read or write
    I106_OPEN_WARNING       =  2,   // Non-fatal problem opening for read or write
    I106_EOF                =  3,   // End of file encountered
    I106_BOF                =  4,   // 
    I106_READ_ERROR         =  5,   // Error reading data from file
    I106_WRITE_ERROR        =  6,   // Error writing data to file
    I106_MORE_DATA          =  7,   // 
    I106_SEEK_ERROR         =  8,
    I106_WRONG_FILE_MODE    =  9,
    I106_NOT_OPEN           = 10,
    I106_ALREADY_OPEN       = 11,
    I106_BUFFER_TOO_SMALL   = 12,
    I106_NO_MORE_DATA       = 13,
    I106_NO_FREE_HANDLES    = 14,
    I106_INVALID_HANDLE     = 15,
    I106_TIME_NOT_FOUND     = 16,
    } EnI106Status;

// Data file open mode
typedef enum
    {
    I106_READ               = 1,    // Open an existing file for reading
    I106_OVERWRITE          = 2,    // Create a new file or overwrite an exising file
    I106_APPEND             = 3,    // Append data to the end of an existing file
    } EnI106Ch10Mode;


/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// IRIG 106 header and optional secondary header data structure
typedef struct
    {
    uint16_t      uSync;                // Packet Sync Pattern
    uint16_t      uChID;                // Channel ID
    uint32_t      ulPacketLen;          // Data length
    uint32_t      ulDataLen;            // Data length
    uint8_t       ubyHdrVer;            // Header Version
    uint8_t       ubySeqNum;            // Sequence Number
    uint8_t       ubyPacketFlags;       // PacketFlags
    uint8_t       ubyDataType;          // Data type
    uint8_t       aubyRefTime[6];       // Reference time
    uint16_t      uChecksum;            // Header Checksum
    uint32_t      aulTime[2];           // Time (start secondary header)
    uint16_t      uReserved;            //
    uint16_t      uSecChecksum;         // Secondary Header Checksum
#if !defined(__GNUC__)
    } SuI106Ch10Header;
#else
    } __attribute__ ((packed)) SuI106Ch10Header;
#endif

// Read state is used to keep track of the next expected data file structure
typedef enum
    {
    enClosed        = 0,
    enWrite         = 1,
    enReadUnsynced  = 2,
    enReadHeader    = 3,
    enReadData      = 4,
    } EnFileState;

// Data structure for IRIG 106 read/write handle
typedef struct
    {
    int             bInUse;
    int             iFile;
    char            szFileName[MAX_PATH];
    EnFileState     enFileState;
    unsigned long   ulCurrPacketLen;
    unsigned long   ulCurrHeaderBuffLen;
    unsigned long   ulCurrDataBuffLen;
    unsigned long   ulCurrDataBuffReadPos;
    unsigned long   ulTotalBytesWritten;
    char            achReserve[128];
    } SuI106Ch10Handle;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Global data
 * -----------
 */

extern SuI106Ch10Handle  g_suI106Handle[4];


/*
 * Function Declaration
 * --------------------
 */

// Open / Close

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10Open          (int               * piI106Ch10Handle,
                             const char          szOpenFileName[],
                             EnI106Ch10Mode      enMode);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10Close         (int                 iI106Handle);


// Read / Write
// ------------

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10ReadNextHeader(int                 iI106Ch10Handle,
                             SuI106Ch10Header  * psuI106Hdr);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10ReadPrevHeader(int                 iI106Ch10Handle,
                             SuI106Ch10Header  * psuI106Hdr);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10ReadData(int                 iI106Ch10Handle,
                       unsigned long       ulBuffSize,
                       void              * pvBuff);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10WriteMsg(int                   iI106Ch10Handle,
                       SuI106Ch10Header    * psuI106Hdr,
                       void                * pvBuff);


// Move file pointer
// -----------------

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10FirstMsg(int iI106Ch10Handle);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10LastMsg(int iI106Ch10Handle);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10SetPos(int iI106Ch10Handle, int64_t llOffset);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    enI106Ch10GetPos(int iI106Ch10Handle, int64_t * pllOffset);


// Utilities
// ---------

I106_DLL_DECLSPEC int I106_CALL_DECL 
    iHeaderInit(SuI106Ch10Header * psuHeader,
                unsigned int       uChanID,
                unsigned int       uDataType,
                unsigned int       uFlags,
                unsigned int       uSeqNum);

I106_DLL_DECLSPEC int I106_CALL_DECL 
    iGetHeaderLen(SuI106Ch10Header * psuHeader);

I106_DLL_DECLSPEC int I106_CALL_DECL 
    iGetDataLen(SuI106Ch10Header * psuHeader);

I106_DLL_DECLSPEC uint16_t I106_CALL_DECL 
    uCalcHeaderChecksum(SuI106Ch10Header * psuHeader);

I106_DLL_DECLSPEC uint16_t I106_CALL_DECL 
    uCalcSecHeaderChecksum(SuI106Ch10Header * psuHeader);

/*
I106_DLL_DECLSPEC int I106_CALL_DECL 
    bCalcDataChecksum(void * pvBuff);
*/

I106_DLL_DECLSPEC uint32_t I106_CALL_DECL 
    uCalcDataBuffReqSize(uint32_t uDataLen, int iChecksumType);

I106_DLL_DECLSPEC EnI106Status I106_CALL_DECL 
    uAddDataFillerChecksum(SuI106Ch10Header * psuI106Hdr, unsigned char achData[]);

 
#ifdef __cplusplus
}
#endif

#endif
