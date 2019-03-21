/****************************************************************************

 i106_data_stream.c - 

 Copyright (c) 2011 Irig106.org

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>

#if defined(__GNUC__)
#define SOCKET            int
#define INVALID_SOCKET    -1
#define SOCKET_ERROR      -1
#define SOCKADDR          struct sockaddr
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <assert.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#endif

#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_data_stream.h"

#ifdef __cplusplus
namespace Irig106 {
#endif

/*
 * Macros and definitions
 * ----------------------
 */

// Make a min() function
#if defined(_WIN32)
#define MIN(X, Y)   min(X, Y)
#else
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif

#define RCV_BUFFER_START_SIZE   32768
#define MAX_UDP_WRITE_SIZE      32726   // From Chapter 10.3.9.1.3
//#define MAX_UDP_WRITE_SIZE      104   // From Chapter 10.3.9.1.3


/*
 * Data structures
 * ---------------
 */

/// Data structure for IRIG 106 network handle
typedef struct
    {
    EnI106Ch10Mode      enNetMode;
    SOCKET              suIrigSocket;
    unsigned int        uUdpSeqNum;
    // Receive buffer stuff
    char              * pchRcvBuffer;
    unsigned long       ulRcvBufferLen;
    unsigned long       ulRcvBufferDataLen;
    int                 bBufferReady;
    unsigned long       ulBufferPosIdx;
    int                 bGotFirstSegment;
    // Transmit buffer stuff
    struct sockaddr_in  suSendIpAddress;
    uint16_t            uSendPort;
    unsigned int        uMaxUdpSize;    // Max size of Ch 10 message(s) not including transfer header
    } SuI106Ch10NetHandle;

/*
 * Module data
 * -----------
 */

static int                  m_bHandlesInited = bFALSE;
static SuI106Ch10NetHandle  m_suNetHandle[MAX_HANDLES];


/*
 * Function Declaration
 * --------------------
 */


/* ----------------------------------------------------------------------- */


// Open / Close

/// Open an IRIG 106 Live Data Streaming receive socket
EnI106Status I106_CALL_DECL
    enI106_OpenNetStreamRead(int iHandle, uint16_t uPort)
    {
    int                     iIdx;
    int                     iResult;
    struct sockaddr_in      ServerAddr;
#if defined(_MSC_VER) 
    WORD                    wVersionRequested;
    WSADATA                 wsaData;
#endif

    // Initialize handle data if necessary
    if (m_bHandlesInited == bFALSE)
        {
        for (iIdx=0; iIdx<MAX_HANDLES; iIdx++)
            {
            m_suNetHandle[iIdx].enNetMode  = I106_CLOSED;
            }
        m_bHandlesInited = bTRUE;
        } // end if file handles not inited yet


#ifdef MULTICAST
    int                     iInterfaceIdx;
    int                     iNumInterfaces;

    struct in_addr          NetInterfaces[10];
    struct in_addr          LocalInterfaceAddr;
    struct in_addr          LocalInterfaceMask;
    struct in_addr          IrigMulticastGroup;
#endif

#if defined(_MSC_VER) 
    // Initialize WinSock, request version 2.2
    wVersionRequested = MAKEWORD(2, 2);
    iResult = WSAStartup(wVersionRequested, &wsaData);

    if (iResult != 0)
        {
//      printf("Unable to initialize Winsock 2.2\n");
        return I106_OPEN_ERROR;
        }
#endif

    // Create a socket for listening to UDP
    m_suNetHandle[iHandle].suIrigSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_suNetHandle[iHandle].suIrigSocket == INVALID_SOCKET) 
        {
//        printf("socket() failed with error: %ld\n", WSAGetLastError());
#if defined(_MSC_VER) 
        WSACleanup();
#endif
        return I106_OPEN_ERROR;
        }

    // Bind to any local address
    ServerAddr.sin_family      = AF_INET;
    ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ServerAddr.sin_port        = htons(uPort);

    iResult = bind(m_suNetHandle[iHandle].suIrigSocket, (SOCKADDR*) &ServerAddr, sizeof(ServerAddr));
    if (iResult == SOCKET_ERROR) 
        {
//        printf("bind() failed with error: %ld\n", WSAGetLastError());
#if defined(_MSC_VER) 
        closesocket(m_suNetHandle[iHandle].suIrigSocket);
        WSACleanup();
#else
        close(m_suNetHandle[iHandle].suIrigSocket);
#endif
        return I106_OPEN_ERROR;
        }

#ifdef MULTICAST
    // Put the appropriate interface into multicast receive mode
    iNumInterfaces = GetInterfaces(NetInterfaces, 10);
    LocalInterfaceAddr.s_addr = inet_addr("192.0.0.0");
    LocalInterfaceMask.s_addr = inet_addr("255.0.0.0");
    IrigMulticastGroup.s_addr = inet_addr("239.0.1.1");
    for (iInterfaceIdx = 0; iInterfaceIdx < iNumInterfaces; iInterfaceIdx++)
        {
        if ((NetInterfaces[iInterfaceIdx].s_addr & LocalInterfaceMask.s_addr) == LocalInterfaceAddr.s_addr)
            {
            join_source_group(m_suNetHandle[iHandle].suIrigSocket, IrigMulticastGroup, NetInterfaces[iInterfaceIdx]);
            break;
            }
        }
#endif

    // Make sure the receive buffer is big enough for at least one UDP packet
    m_suNetHandle[iHandle].ulRcvBufferLen     = RCV_BUFFER_START_SIZE;
    m_suNetHandle[iHandle].pchRcvBuffer       = (char *)malloc(RCV_BUFFER_START_SIZE);

    m_suNetHandle[iHandle].ulRcvBufferDataLen = 0L;
    m_suNetHandle[iHandle].bBufferReady       = bFALSE;
    m_suNetHandle[iHandle].ulBufferPosIdx     = 0L;
    m_suNetHandle[iHandle].bGotFirstSegment   = bFALSE;

    m_suNetHandle[iHandle].enNetMode          = I106_READ_NET_STREAM;

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

/// Open an IRIG 106 Live Data Streaming send socket
EnI106Status I106_CALL_DECL
    enI106_OpenNetStreamWrite(int iHandle, uint32_t uIpAddress, uint16_t uUdpPort)
    {
    int                     iIdx;
    int                     iResult;
#ifdef SO_MAX_MSG_SIZE
    int                     iMaxMsgSizeLen;
#endif
#if defined(_MSC_VER) 
    WORD                    wVersionRequested;
    WSADATA                 wsaData;
    DWORD                   iMaxMsgSize;
#else
    socklen_t               iMaxMsgSize;
#endif


    // Initialize handle data if necessary
    if (m_bHandlesInited == bFALSE)
        {
        for (iIdx=0; iIdx<MAX_HANDLES; iIdx++)
            {
            m_suNetHandle[iIdx].enNetMode  = I106_CLOSED;
            m_suNetHandle[iIdx].uUdpSeqNum = 0;
            }
        m_bHandlesInited = bTRUE;
        } // end if file handles not inited yet


#if defined(_MSC_VER) 
    // Initialize WinSock, request version 2.2
    wVersionRequested = MAKEWORD(2, 2);
    iResult = WSAStartup(wVersionRequested, &wsaData);

    if (iResult != 0)
        {
//      printf("Unable to initialize Winsock 2.2\n");
        return I106_OPEN_ERROR;
        }
#endif

    // Create a socket for writing to UDP
    m_suNetHandle[iHandle].suIrigSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_suNetHandle[iHandle].suIrigSocket == INVALID_SOCKET) 
        {
//        printf("socket() failed with error: %ld\n", WSAGetLastError());
#if defined(_MSC_VER) 
        WSACleanup();
#endif
        return I106_OPEN_ERROR;
        }

    // Fill in the remote host information
    m_suNetHandle[iHandle].suSendIpAddress.sin_family      = AF_INET;
    m_suNetHandle[iHandle].suSendIpAddress.sin_port        = htons(uUdpPort);
    m_suNetHandle[iHandle].suSendIpAddress.sin_addr.s_addr = htonl(uIpAddress);

#ifdef SO_MAX_MSG_SIZE
    // getsockopt to retrieve the value of option SO_MAX_MSG_SIZE after a socket has been created.
    iMaxMsgSizeLen = sizeof(iMaxMsgSize);
    iResult = getsockopt(m_suNetHandle[iHandle].suIrigSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&iMaxMsgSize, &iMaxMsgSizeLen);
#else
    iResult = 1;
#endif

    if (iResult == 0)
        {
        // Use smaller, taking into account Ch 10 UDP transfer header
        m_suNetHandle[iHandle].uMaxUdpSize = MIN(iMaxMsgSize-6,MAX_UDP_WRITE_SIZE);
        }
    else
        m_suNetHandle[iHandle].uMaxUdpSize = MAX_UDP_WRITE_SIZE;

    m_suNetHandle[iHandle].enNetMode = I106_WRITE_NET_STREAM;

    return I106_OK;
    }


/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL
    enI106_CloseNetStream(int iHandle)
    {

#ifdef MULTICAST
    // Restore the appropriate interface out of multicast receive mode
    iNumInterfaces = GetInterfaces(NetInterfaces, 10);
    LocalInterfaceAddr.s_addr = inet_addr("192.0.0.0");
    LocalInterfaceMask.s_addr = inet_addr("255.0.0.0");
    IrigMulticastGroup.s_addr = inet_addr("224.0.0.1");
    for (iInterfaceIdx = 0; iInterfaceIdx < iNumInterfaces; iInterfaceIdx++)
        {
        if ((NetInterfaces[iInterfaceIdx].s_addr & LocalInterfaceMask.s_addr) == LocalInterfaceAddr.s_addr)
            {
            leave_source_group(IrigSocket, IrigMulticastGroup, NetInterfaces[iInterfaceIdx]);
            break;
            }
        }
#endif

    switch (m_suNetHandle[iHandle].enNetMode)
        {
        case I106_READ_NET_STREAM :
            // Close the receive socket
#if defined(_MSC_VER) 
            closesocket(m_suNetHandle[iHandle].suIrigSocket);
            WSACleanup();
#else
            close(m_suNetHandle[iHandle].suIrigSocket);
#endif
            // Free up allocated memory
            free(m_suNetHandle[iHandle].pchRcvBuffer);
            m_suNetHandle[iHandle].pchRcvBuffer       = NULL;
            m_suNetHandle[iHandle].ulRcvBufferLen     = 0L;
            break;

        case I106_WRITE_NET_STREAM :
            // Close the transmit socket
#if defined(_MSC_VER) 
            closesocket(m_suNetHandle[iHandle].suIrigSocket);
            WSACleanup();
#else
            close(m_suNetHandle[iHandle].suIrigSocket);
#endif
            break;

        default :
            break;
        } // end switch on enNetMode

    // Mark this case closed
    m_suNetHandle[iHandle].enNetMode = I106_CLOSED;

    return I106_OK;
    }



// ----------------------------------------------------------------------------
// UDP Read routines
// ----------------------------------------------------------------------------

//! @brief Utility method for cross-platform recvmsg
//! @details Used by enI106_ReadNetStream to split a read across the
//!          header buffer and a body buffer.
//! @param suSocket The SOCKET handle to recv from
//! @param[out] pvBuffer1 Pointer to the first buffer to fill
//! @param[in]  ulBufLen1 Length of the first buffer, in bytes 
//! @param[out] pvBuffer2 Pointer to the second buffer to fill
//! @param[in]  ulBufLen2 Length of the second buffer, in bytes
//! @param[out] ulBytesRcvdOut Returns the number of bytes received and copied into the buffers
//! @return I106_OK On success;
//!         ulBytesRcvdOut contains the number of bytes read.
//! @return I106_MORE_DATA On a successful, but truncated, read;
//!         ulBytesRcvdOut contains the actual number of bytes read. Some data was lost.
//! @return I106_READ_ERROR On error;
//!         ulBytesRcvdOut is undefined
static EnI106Status
    RecvMsgSplit(SOCKET          suSocket,
                 void * const    pvBuffer1,
                 unsigned long   ulBufLen1,
                 void * const    pvBuffer2,
                 unsigned long   ulBufLen2,
                 unsigned long * pulBytesRcvdOut)
#if defined(_MSC_VER)
    {
    WSABUF         asuUdpRcvBuffs[2];
    DWORD          UdpRcvFlags = 0;
    DWORD          dwBytesRcvd = 0;
    int            iResult = 0;

    // Setup the message buffer structure
    asuUdpRcvBuffs[0].len = ulBufLen1;
    asuUdpRcvBuffs[0].buf = (char *)pvBuffer1;
    asuUdpRcvBuffs[1].len = ulBufLen2;
    asuUdpRcvBuffs[1].buf = (char *)pvBuffer2;

    iResult = WSARecv(suSocket, asuUdpRcvBuffs, 2, &dwBytesRcvd, &UdpRcvFlags, NULL, NULL);
    if( pulBytesRcvdOut )
        *pulBytesRcvdOut = (unsigned long)dwBytesRcvd;

    if( 0 == iResult )
        return I106_OK;
    else
        {
        int const err = WSAGetLastError();
        if( WSAEMSGSIZE == err )
            return I106_MORE_DATA;
        else
            return I106_READ_ERROR;
        }
    }
#else
    {
    struct msghdr  suMsgHdr = { 0 };
    struct iovec   asuUdpRcvBuffs[2];
    const int      UdpRcvFlags = 0;
    ssize_t        iResult = 0;

    // Setup the message buffer structure
    suMsgHdr.msg_iov    = asuUdpRcvBuffs;
    suMsgHdr.msg_iovlen = 2;

    asuUdpRcvBuffs[0].iov_len  = ulBufLen1;
    asuUdpRcvBuffs[0].iov_base = (char *)pvBuffer1;
    asuUdpRcvBuffs[1].iov_len  = ulBufLen2;
    asuUdpRcvBuffs[1].iov_base = (char *)pvBuffer2;

    iResult = recvmsg(suSocket, &suMsgHdr, UdpRcvFlags);
    if( pulBytesRcvdOut )
        *pulBytesRcvdOut = (unsigned long)iResult;

    if (iResult < 0)
        return I106_READ_ERROR;
    else if( MSG_TRUNC == suMsgHdr.msg_flags )
        return I106_MORE_DATA;
    else
        return I106_OK;
    }
#endif

// ------------------------------------------------------------------------
//! @brief Utility method for dropping a peek'd bad packet
//! @details If enI106_ReadNetStream MSG_PEEK's a packet that is too small,
//!     then we have to remove it from the socket buffer before we can bail.
//!     Otherwise, the next MSG_PEEK will see the same bad packet.
static void
    DropBadPacket(int iHandle)
{
    char dummy;
    // We don't care about the return value, we're failing anyways.
    (void)recvfrom(m_suNetHandle[iHandle].suIrigSocket, &dummy, sizeof(dummy), 0, 0, 0);
}


// ------------------------------------------------------------------------
// Get the next header.

// =================================================================
// GIANT TODO --> HANDLE FORMAT 3 AS WELL AS FORMAT 1. SKIP FORMAT 2
// =================================================================

int I106_CALL_DECL
    enI106_ReadNetStream(int            iHandle,
                         void         * pvBuffer,
                         unsigned int   iBuffSize)
    {
    // The minimum packet size needed for a valid segmented message packet
    enum { MIN_SEG_LEN = sizeof(SuUDP_Transfer_Header_F1_Seg)-1 + sizeof(SuI106Ch10Header) };

    int                             iResult;
    SuUDP_Transfer_Header_F1_Seg    suUdpSeg;  // Same prefix as the header of an unsegmented msg

    unsigned long                   ulBytesRcvd;
    int                             iCopySize;

    SuI106Ch10Header              * psuHeader;

    // If we don't have a buffer ready to read from then read network packets
    if (m_suNetHandle[iHandle].bBufferReady == bFALSE)
        {
        // Get ready for a new buffer of data
        m_suNetHandle[iHandle].bBufferReady     = bFALSE;
        m_suNetHandle[iHandle].bGotFirstSegment = bFALSE;
        m_suNetHandle[iHandle].ulBufferPosIdx   = 0L;

        // Read until we've got a complete Ch 10 packet(s)
        while (m_suNetHandle[iHandle].bBufferReady == bFALSE)
            {
            // Peek at the message to determine the msg type (segmented or non-segmented)
            iResult = recvfrom(m_suNetHandle[iHandle].suIrigSocket, (char *)&suUdpSeg, sizeof(suUdpSeg), MSG_PEEK, NULL, NULL);
//printf("recvfrom = %d\n", iResult);
#if defined(_MSC_VER)
            // Make the WinSock return code more like POSIX to simplify the logic
            // WinSock returns -1 when the message is larger than the buffer
            // Thus, (iResult==-1) && WSAEMSGSIZE is expected, as we're only reading the header
            if( (iResult == -1)  )
                {
                int const err = WSAGetLastError(); // called out for debugging
                if (err == WSAEMSGSIZE)
                    iResult = sizeof(suUdpSeg); // The buffer was filled
                }
#endif

            if( iResult == -1 )
                {
                enI106_DumpNetStream(iHandle);
                return -1;
                }

            // If I don't have at least enough for a common header then drop and bail
            // We'll check length again later, which depends on the msg type
            if( iResult < UDP_Transfer_Header_F1_NonSeg_Len )
                {
//printf("msg bytes (%d) < transfer header length (%d)\n", iResult, UDP_Transfer_Header_NonSeg_Len);
                // Because we're peeking, we have to make sure to drop the bad packet.
                DropBadPacket(iHandle);
                enI106_DumpNetStream(iHandle);
                continue;
                }

            //! @todo Check the version field for a known version

            // Check and handle UDP sequence number
            if (suUdpSeg.uUdpSeqNum != m_suNetHandle[iHandle].uUdpSeqNum+1)
                {
                enI106_DumpNetStream(iHandle);
//printf("UDP Sequence Gap - %u  %u\n", m_suNetHandle[iHandle].uUdpSeqNum, suUdpSeg.uSeqNum);
                }
            m_suNetHandle[iHandle].uUdpSeqNum = suUdpSeg.uUdpSeqNum;

            // Handle full and segmented packet types
            switch (suUdpSeg.uMsgType)
                {
                case 0 : // Full packet(s)
//printf("Full - ");

                    iResult = RecvMsgSplit(m_suNetHandle[iHandle].suIrigSocket,
                                           &suUdpSeg,
                                           UDP_Transfer_Header_F1_NonSeg_Len,
                                           m_suNetHandle[iHandle].pchRcvBuffer,
                                           m_suNetHandle[iHandle].ulRcvBufferLen,
                                           &ulBytesRcvd);
                    if (I106_OK != iResult)
                        {
                        enI106_DumpNetStream(iHandle);
                        if( I106_READ_ERROR == iResult )
                            return -1;
                        else
                            continue;
                        }

//printf("Size = %lu\n", ulBytesRcvd - UDP_Transfer_Header_NonSeg_Len);

                    m_suNetHandle[iHandle].ulRcvBufferDataLen = ulBytesRcvd - UDP_Transfer_Header_F1_NonSeg_Len;
                    m_suNetHandle[iHandle].bBufferReady       = bTRUE;
                    m_suNetHandle[iHandle].ulBufferPosIdx     = 0L;
                    break;

                case 1 : // Segmented packet
//printf("Segmented - ");

                    // We need at least enough for a segmented header to do the next read.
                    if( iResult < UDP_Transfer_Header_Seg_Len )
                        {
                        DropBadPacket(iHandle);
                        enI106_DumpNetStream(iHandle);
                        continue;
                        }

                    // Always write to the beginning of the buffer while waiting for the first segment
                    // The first UDP packet is guaranteed to fit our default starting size
                    if (m_suNetHandle[iHandle].bGotFirstSegment == bFALSE)
                        {
                        iResult = RecvMsgSplit(m_suNetHandle[iHandle].suIrigSocket,
                                               &suUdpSeg,
                                               UDP_Transfer_Header_Seg_Len,
                                               m_suNetHandle[iHandle].pchRcvBuffer,
                                               m_suNetHandle[iHandle].ulRcvBufferLen,
                                               &ulBytesRcvd);
                        }
                    else
                        {
                        iResult = RecvMsgSplit(m_suNetHandle[iHandle].suIrigSocket,
                                               &suUdpSeg,
                                               UDP_Transfer_Header_Seg_Len,
                                               &(m_suNetHandle[iHandle].pchRcvBuffer[suUdpSeg.uSegmentOffset]),
                                               m_suNetHandle[iHandle].ulRcvBufferLen - suUdpSeg.uSegmentOffset,
                                               &ulBytesRcvd);
                        }

                    if (I106_OK != iResult)
                        {
                        enI106_DumpNetStream(iHandle);
                        if( I106_READ_ERROR == iResult )
                            return -1;
                        else
                            continue;
                        }

//printf("Offset = %u\n", suUdpSeg.uSegmentOffset);

                    // Make sure we can access Ch 10 header info
                    if( ulBytesRcvd < MIN_SEG_LEN )
                        {
                        DropBadPacket(iHandle);
                        enI106_DumpNetStream(iHandle);
                        continue;
                        }

                    psuHeader = (SuI106Ch10Header *)m_suNetHandle[iHandle].pchRcvBuffer;

                    // If it's the first packet then figure out if our buffer is large enough for the whole Ch10 packet
                    if (suUdpSeg.uSegmentOffset == 0)
                        {
                        if (psuHeader->ulPacketLen > m_suNetHandle[iHandle].ulRcvBufferLen)
                            {
                            m_suNetHandle[iHandle].ulRcvBufferLen = psuHeader->ulPacketLen + 0x4000;
                            m_suNetHandle[iHandle].pchRcvBuffer   = (char *)realloc(m_suNetHandle[iHandle].pchRcvBuffer,m_suNetHandle[iHandle].ulRcvBufferLen);
                            psuHeader = (SuI106Ch10Header *)m_suNetHandle[iHandle].pchRcvBuffer;
                            } // end if buffer too small for whole Ch 10 packet
                        m_suNetHandle[iHandle].bGotFirstSegment   = bTRUE;
                        m_suNetHandle[iHandle].ulRcvBufferDataLen = psuHeader->ulPacketLen;
                        } // end if first packet

                    // If we've gotten the first and last packets then mark the buffer as full and ready
                    if ((m_suNetHandle[iHandle].bGotFirstSegment == bTRUE) &&                     // First UDP buffer
                        ((suUdpSeg.uSegmentOffset + ulBytesRcvd - UDP_Transfer_Header_Seg_Len) >= psuHeader->ulPacketLen)) // Last UDP buffer
                        {
//if ((suUdpSeg.uSegmentOffset + ulBytesRcvd - UDP_Transfer_Header_Seg_Len) > psuHeader->ulPacketLen)
    //printf("Last packet too long");
                        m_suNetHandle[iHandle].bBufferReady     = bTRUE;
                        m_suNetHandle[iHandle].bGotFirstSegment = bFALSE;
                        m_suNetHandle[iHandle].ulBufferPosIdx     = 0L;
                        } // end if got first and last packet

                    break;

                default :
                    // The peek'd packet specifies some unknown/junk message type
                    // Toss this packet so the MSG_PEEK doesn't loop on it endlessly
                    DropBadPacket(iHandle);
                    enI106_DumpNetStream(iHandle);
                    continue;
                } // end switch on UDP packet type
            } // end while reading for a complete buffer
        } // end if called and buffer not ready

    // Copy data to the user buffer
    iCopySize = MIN(m_suNetHandle[iHandle].ulRcvBufferDataLen - m_suNetHandle[iHandle].ulBufferPosIdx, iBuffSize);
    memcpy(pvBuffer, &m_suNetHandle[iHandle].pchRcvBuffer[m_suNetHandle[iHandle].ulBufferPosIdx], iCopySize);

    // Update buffer status
    m_suNetHandle[iHandle].ulBufferPosIdx += iCopySize;
    if (m_suNetHandle[iHandle].ulBufferPosIdx >= m_suNetHandle[iHandle].ulRcvBufferDataLen)
        {
        m_suNetHandle[iHandle].bBufferReady = bFALSE;
        }

    return iCopySize;
    }


// ------------------------------------------------------------------------

// Manipulate receive buffer
// -------------------------

// Invalidate the receive buffer so that the next enI106_ReadNetStream()
// causes a new complete buffer to be read from the network

EnI106Status I106_CALL_DECL
    enI106_DumpNetStream(int iHandle)
    {
    m_suNetHandle[iHandle].bBufferReady     = bFALSE;
    m_suNetHandle[iHandle].bGotFirstSegment = bFALSE;
    m_suNetHandle[iHandle].ulBufferPosIdx   = 0L;

    return I106_OK;
    }



// ------------------------------------------------------------------------

EnI106Status I106_CALL_DECL
    enI106_MoveReadPointer(int iHandle, long iRelOffset)
    {
    long    lNewPosition;

    lNewPosition = m_suNetHandle[iHandle].ulBufferPosIdx + iRelOffset;
    if (lNewPosition < 0)
        m_suNetHandle[iHandle].ulBufferPosIdx = 0L;

    else if ((unsigned long)lNewPosition >= m_suNetHandle[iHandle].ulRcvBufferDataLen)
        {
        m_suNetHandle[iHandle].ulBufferPosIdx = 0L;
        m_suNetHandle[iHandle].bBufferReady   = bFALSE;
        }

    else
        m_suNetHandle[iHandle].ulBufferPosIdx = (unsigned long)lNewPosition;

    return I106_OK;
    }



// ----------------------------------------------------------------------------
// UDP Write routines
// ----------------------------------------------------------------------------

EnI106Status I106_CALL_DECL
    enI106_WriteNetStream(int           iHandle,
                          void        * pvBuffer,
                          uint32_t      uBuffSize)
    {
    EnI106Status        enStatus;
    EnI106Status        enReturnStatus;
    void              * pvCurrSendBuffPos;
    uint32_t            uCurrSendBuffLen;
    SuI106Ch10Header  * psuCurrCh10Header;
    SuI106Ch10Header  * psuNextCh10Header;

    enReturnStatus = I106_OK;

    // Check for initialized
    if (m_suNetHandle[iHandle].enNetMode != I106_WRITE_NET_STREAM)
        return I106_NOT_OPEN;

    // THIS WOULD BE A GOOD PLACE TO CHECK DATA PACKET INTEGRITY SOMEDAY

    // Queue up the first IRIG packet
    psuCurrCh10Header = (SuI106Ch10Header *)pvBuffer;
    uCurrSendBuffLen  = psuCurrCh10Header->ulPacketLen;
    pvCurrSendBuffPos = pvBuffer;
//    psuNextCh10Header = (SuI106Ch10Header  *)((char *)pvBuffer + psuCurrCh10Header->ulPacketLen);

//    if ((char *)psuNextCh10Header >= ((char *)pvBuffer + uBuffSize))
//        psuNextCh10Header = NULL;

    // If psuNextCh10Header > pvBuffer + uBuffSize then this is a malformed buffer.
//    assert((char *)psuNextCh10Header <= ((char *)pvBuffer + uBuffSize));

    // Step through IRIG packets until the length would exceed max UDP packet size
    while (1==1)
        {
        // If current packet size > max then send segmented packet
        if (psuCurrCh10Header->ulPacketLen > m_suNetHandle[iHandle].uMaxUdpSize)
            {
            // This big packet had better be the first one in our current send buffer
//          assert(pvCurrSendBuffPos == psuCurrCh10Header);
            assert(psuCurrCh10Header->uSync == 0xEB25);

            // Send segmented packet
            enStatus = enI106_WriteNetSegmented(iHandle, pvCurrSendBuffPos, uCurrSendBuffLen);
            if (enStatus != I106_OK)
                enReturnStatus = enStatus;

            // Update pointer to the next IRIG packet, or exit if we are done
            pvCurrSendBuffPos = ((char *)pvCurrSendBuffPos + psuCurrCh10Header->ulPacketLen);
            psuCurrCh10Header = (SuI106Ch10Header *)pvCurrSendBuffPos;
            if ((char *)pvCurrSendBuffPos >= ((char *)pvBuffer + uBuffSize))
                {
                break;
                }
            else
                {
                uCurrSendBuffLen  = psuCurrCh10Header->ulPacketLen;
                continue;
                }
            } // end if big segmented packet

        // If no more Ch 10 packets then send what we have, done
        if (((char *)psuCurrCh10Header + psuCurrCh10Header->ulPacketLen) >= ((char *)pvBuffer + uBuffSize))
            {
            // If psuNextCh10Header > pvBuffer + uBuffSize then this is a malformed buffer.
//          assert((char *)psuNextCh10Header <= ((char *)pvBuffer + uBuffSize));

            assert(uCurrSendBuffLen <= m_suNetHandle[iHandle].uMaxUdpSize);
            // Send non-segmented packet
            enStatus = enI106_WriteNetNonSegmented(iHandle, pvCurrSendBuffPos, uCurrSendBuffLen);
            if (enStatus != I106_OK)
                enReturnStatus = enStatus;

            // Done sending packets
            break;
            } // end if last message(s) in buffer

        // There is another buffer so let's check its size. If next packet would put us over 
        // max size then send what we have
        psuNextCh10Header = (SuI106Ch10Header *)((char *)psuCurrCh10Header + psuCurrCh10Header->ulPacketLen);
        // Might want to validate sync word, packet header checksum, and packet checksum
        assert(psuNextCh10Header->uSync == 0xEB25);

        if ((uCurrSendBuffLen + psuNextCh10Header->ulPacketLen) > m_suNetHandle[iHandle].uMaxUdpSize)
            {
            assert(psuCurrCh10Header->uSync == 0xEB25);

            // Send non-segmented packet
            enStatus = enI106_WriteNetNonSegmented(iHandle, pvCurrSendBuffPos, uCurrSendBuffLen);
            if (enStatus != I106_OK)
                enReturnStatus = enStatus;

            // Update pointer to the next IRIG packet, or exit if we are done
            pvCurrSendBuffPos = psuNextCh10Header;
            psuCurrCh10Header = psuNextCh10Header;
            uCurrSendBuffLen  = psuCurrCh10Header->ulPacketLen;
            if ((char *)pvCurrSendBuffPos >= ((char *)pvBuffer + uBuffSize))
                {
                // We should never get here but just in case
                assert(1==0);
                break;
                }
            else
                {
                continue;
                }
            } // end if next packet puts us over the max size

        // Nothing to send so advance to the next Ch 10 packet
        psuCurrCh10Header += psuCurrCh10Header->ulPacketLen;
        // Might want to validate sync word, packet header checksum, and packet checksum
        assert(psuCurrCh10Header->uSync == 0xEB25);

        } // Done stepping through all IRIG packets

    return enReturnStatus;
    }


// ------------------------------------------------------------------------

// Send a non-segmented UDP packet

EnI106Status I106_CALL_DECL
    enI106_WriteNetNonSegmented(
            int           iHandle,
            void        * pvBuffer,
            uint32_t      uBuffSize)
    {
    EnI106Status        enReturnStatus;

#if defined(_MSC_VER)
//  SOCKET_ADDRESS      suMsSendIpAddress;
    WSAMSG              suMsMsgInfo;
    WSABUF              suMsBuffInfo[2];
    WSABUF              suMsControl;
    DWORD               lBytesSent;
#endif
    int                 iSendStatus;

    SuUDP_Transfer_Header_F1_NonSeg    suUdpHeaderNonF1Seg;

    enReturnStatus = I106_OK;

    // Setup the non-segemented transfer header
    suUdpHeaderNonF1Seg.uFormat  = 1;
    suUdpHeaderNonF1Seg.uMsgType = 0;
    suUdpHeaderNonF1Seg.uUdpSeqNum  = m_suNetHandle[iHandle].uUdpSeqNum;

    // Send the IRIG UDP packet
#if defined(_MSC_VER)
    // I don't really want or need control data. I hope this doesn't 
    // cause WSASendMsg() to fail.
    suMsControl.buf           = NULL;
    suMsControl.len           = 0;

    // Setup pointers to the data to be sent.
    suMsBuffInfo[0].buf       = (CHAR *)&suUdpHeaderNonF1Seg;
    suMsBuffInfo[0].len       = 4;
    suMsBuffInfo[1].buf       = (CHAR *)pvBuffer;
    suMsBuffInfo[1].len       = uBuffSize;

    // Setup the send info for WSASendMsg()
    suMsMsgInfo.name          = (SOCKADDR*)&(m_suNetHandle[iHandle].suSendIpAddress);  // THIS IS AMBIGUOUS IN MSDN
    suMsMsgInfo.namelen       = sizeof(m_suNetHandle[iHandle].suSendIpAddress);
    suMsMsgInfo.lpBuffers     = suMsBuffInfo;
    suMsMsgInfo.dwBufferCount = 2;
    suMsMsgInfo.Control       = suMsControl;
    suMsMsgInfo.dwFlags       = 0;

    // Send it. Done!
    iSendStatus = WSASendMsg(m_suNetHandle[iHandle].suIrigSocket, &suMsMsgInfo, 0, &lBytesSent, NULL, NULL);
    if (iSendStatus != 0)
        enReturnStatus = I106_WRITE_ERROR;
#else
// TODO - LINUX CODE
#endif

    // Increment the sequence number for next time
    m_suNetHandle[iHandle].uUdpSeqNum++;

    return enReturnStatus;
    }


// ------------------------------------------------------------------------

// Send a segmented UDP packet

EnI106Status I106_CALL_DECL
    enI106_WriteNetSegmented(
            int           iHandle,
            void        * pvBuffer,
            uint32_t      uBuffSize)
    {
    EnI106Status        enReturnStatus;
    uint32_t            uBuffIdx;
    char              * pchBuffer;
    uint32_t            uSendSize;
    int                 iSendStatus;
    SuI106Ch10Header  * psuHeader;

#if defined(_MSC_VER)
    WSAMSG              suMsMsgInfo;
    WSABUF              suMsBuffInfo[2];
    WSABUF              suMsControl;
    DWORD               lBytesSent;
#endif

    SuUDP_Transfer_Header_F1_Seg       suUdpHeaderF1Seg;

    enReturnStatus = I106_OK;

    // Setup the segemented transfer header
    psuHeader = (SuI106Ch10Header *)pvBuffer;

    memset(&suUdpHeaderF1Seg, 0, 12);
    suUdpHeaderF1Seg.uFormat      = 1;
    suUdpHeaderF1Seg.uMsgType     = 1;
    suUdpHeaderF1Seg.uChID        = psuHeader->uChID;
    suUdpHeaderF1Seg.uChanSeqNum  = psuHeader->ubySeqNum;

    // Send the IRIG UDP packets
    uBuffIdx = 0;
    while (uBuffIdx < uBuffSize)
        {
        suUdpHeaderF1Seg.uUdpSeqNum        = m_suNetHandle[iHandle].uUdpSeqNum;
        suUdpHeaderF1Seg.uSegmentOffset = uBuffIdx;

        pchBuffer  = (char *)pvBuffer + uBuffIdx;

        uSendSize = MIN(m_suNetHandle[iHandle].uMaxUdpSize, uBuffSize-uBuffIdx);
#if defined(_MSC_VER)
        // I don't really want or need control data. I hope this doesn't 
        // cause WSASendMsg() to fail.
        suMsControl.buf           = NULL;
        suMsControl.len           = 0;

        // Setup pointers to the data to be sent.
        suMsBuffInfo[0].buf       = (CHAR *)&suUdpHeaderF1Seg;
        suMsBuffInfo[0].len       = 12;
        suMsBuffInfo[1].buf       = pchBuffer;
        suMsBuffInfo[1].len       = uSendSize;

        // Setup the send info for WSASendMsg()
        suMsMsgInfo.name          = (SOCKADDR*)&(m_suNetHandle[iHandle].suSendIpAddress);  // THIS IS AMBIGUOUS IN MSDN
        suMsMsgInfo.namelen       = sizeof(m_suNetHandle[iHandle].suSendIpAddress);
        suMsMsgInfo.lpBuffers     = suMsBuffInfo;
        suMsMsgInfo.dwBufferCount = 2;
        suMsMsgInfo.Control       = suMsControl;
        suMsMsgInfo.dwFlags       = 0;

        // Send it. Done!
        iSendStatus = WSASendMsg(m_suNetHandle[iHandle].suIrigSocket, &suMsMsgInfo, 0, &lBytesSent, NULL, NULL);
        if (iSendStatus != 0)
            {
            enReturnStatus = I106_WRITE_ERROR;
            break;
            }

#else
// TODO - LINUX CODE
#endif

        // Update the buffer index
        uBuffIdx += uSendSize;

        // Increment the sequence number for next time
        m_suNetHandle[iHandle].uUdpSeqNum++;

        } // end while not at the end of the buffer

    return enReturnStatus;
    }
#ifdef __cplusplus
} // end namespace
#endif
