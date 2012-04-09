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
//#include <iostream>
//#include <fstream>
//#include <string>
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


#define RCV_BUFFER_START_SIZE   32768

/*
 * Data structures
 * ---------------
 */

/// Data structure for IRIG 106 network handle
typedef struct
    {
    // Network socket and receive buffer stuff
    SOCKET              suIrigSocket;
    unsigned int        uUdpSeqNum;
    char              * pchRcvBuffer;
    unsigned long       ulRcvBufferLen;
    unsigned long       ulRcvBufferDataLen;
    int                 bBufferReady;
    unsigned long       ulBufferPosIdx;
    int                 bGotFirstSegment;
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
    enI106_OpenNetStream (int iHandle, uint16_t uPort)
    {
//    int                     iIdx;
    int                     iResult;

#if defined(_MSC_VER) 
    WORD                    wVersionRequested;
    WSADATA                 wsaData;
#endif

#ifdef MULTICAST
    int                     iInterfaceIdx;
    int                     iNumInterfaces;

    struct in_addr          NetInterfaces[10];
    struct in_addr          LocalInterfaceAddr;
    struct in_addr          LocalInterfaceMask;
    struct in_addr          IrigMulticastGroup;
#endif

    struct sockaddr_in      ServerAddr;

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
        WSACleanup();
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
        closesocket(m_suNetHandle[iHandle].suIrigSocket);
#if defined(_MSC_VER) 
        WSACleanup();
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

    closesocket(m_suNetHandle[iHandle].suIrigSocket);
#if defined(_MSC_VER) 
    WSACleanup();
#endif

    // Free up allocated memory
    free(m_suNetHandle[iHandle].pchRcvBuffer);
    m_suNetHandle[iHandle].pchRcvBuffer       = NULL;
    m_suNetHandle[iHandle].ulRcvBufferLen     = 0L;

    return I106_OK;
    }



// ------------------------------------------------------------------------

// Get the next header.

int I106_CALL_DECL
    enI106_ReadNetStream(int            iHandle,
                         void         * pvBuffer,
                         unsigned int   iBuffSize)
    {
    int                             iResult;
    SuUDP_Transfer_Header_Seg       suUdpSeg;  // Same prefix as the header of an unsegmented msg

#if defined(_MSC_VER) 
    WSABUF                          asuUdpRcvBuffs[2];
    DWORD                           UdpRcvFlags;
#else
    struct msghdr                   suMsgHdr;
    struct iovec                    asuUdpRcvBuffs[2];
    int                             UdpRcvFlags;
#endif
    unsigned long                   ulBytesRcvd;
    int                             iCopySize;

    SuI106Ch10Header              * psuHeader;

    // Setup the message buffer structure
#if !defined(_MSC_VER) 
    suMsgHdr.msg_iov    = asuUdpRcvBuffs;
    suMsgHdr.msg_iovlen = 2;
#endif

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

#if defined(_MSC_VER)
            // Make the WinSock return code more like POSIX to simplify the logic
            // WinSock returns -1 when the message is larger than the buffer
            // Thus, (iResult==-1) && WSAEMSGSIZE is expected, as we're only reading the header
            if( (iResult == -1)  )
            {
                int const err = WSAGetLastError(); // called out for debugging
                if( err == WSAEMSGSIZE)
                    iResult = sizeof(suUdpSeg); // The buffer was filled
            }
#endif

            // If I don't get a full buffer then bail
            if( iResult != sizeof(suUdpSeg) )
                {
                // Because we're peeking, we have to make sure to drop the bad packet.
                // On error, there is nothing to drop. Only drop undersized packets.
                if( iResult != -1 )
                    {
                    // Toss this packet so the MSG_PEEK doesn't loop on it endlessly
                    // We don't care about the return value, we're failing anyways.
                    (void)recvfrom(m_suNetHandle[iHandle].suIrigSocket, (char *)&suUdpSeg, sizeof(suUdpSeg), 0, 0, 0);
                    }

                m_suNetHandle[iHandle].bBufferReady     = bFALSE;
                m_suNetHandle[iHandle].bGotFirstSegment = bFALSE;
                m_suNetHandle[iHandle].ulBufferPosIdx   = 0L;
                return -1;
                }

            //! @todo Check the version field for a known version

            // Check and handle UDP sequence number
            if (suUdpSeg.uSeqNum != m_suNetHandle[iHandle].uUdpSeqNum+1)
                printf("UDP Sequence Gap - %u  %u\n", m_suNetHandle[iHandle].uUdpSeqNum, suUdpSeg.uSeqNum);
            m_suNetHandle[iHandle].uUdpSeqNum = suUdpSeg.uSeqNum;

            // Handle full and segmented packet types
            switch (suUdpSeg.uMsgType)
                {
                case 0 : // Full packet(s)
//printf("Full - ");

                    UdpRcvFlags = 0;
#if defined(_MSC_VER) 
                    asuUdpRcvBuffs[0].len = 4;
                    asuUdpRcvBuffs[0].buf = (char *)&suUdpSeg;
                    asuUdpRcvBuffs[1].len = m_suNetHandle[iHandle].ulRcvBufferLen;
                    asuUdpRcvBuffs[1].buf = m_suNetHandle[iHandle].pchRcvBuffer;
                    iResult = WSARecv(m_suNetHandle[iHandle].suIrigSocket, asuUdpRcvBuffs, 2, &ulBytesRcvd, &UdpRcvFlags, NULL, NULL);
#else
                    asuUdpRcvBuffs[0].iov_len  = 4;
                    asuUdpRcvBuffs[0].iov_base = (char *)&suUdpSeg;
                    asuUdpRcvBuffs[1].iov_len  = m_suNetHandle[iHandle].ulRcvBufferLen;
                    asuUdpRcvBuffs[1].iov_base = m_suNetHandle[iHandle].pchRcvBuffer;
                    iResult = recvmsg(m_suNetHandle[iHandle].suIrigSocket, &suMsgHdr, UdpRcvFlags);
#endif
                    if (iResult != 0)
                        {
                        m_suNetHandle[iHandle].bBufferReady     = bFALSE;
                        m_suNetHandle[iHandle].bGotFirstSegment = bFALSE;
                        m_suNetHandle[iHandle].ulBufferPosIdx   = 0L;
                        return -1;
                        }

//printf("Size = %lu\n", ulBytesRcvd - 4);

                    m_suNetHandle[iHandle].ulRcvBufferDataLen = ulBytesRcvd - 4;
                    m_suNetHandle[iHandle].bBufferReady       = bTRUE;
                    m_suNetHandle[iHandle].ulBufferPosIdx     = 0L;
                    break;

                case 1 : // Segmented packet
//printf("Segmented - ");
                    // Get buffers setup
#if defined(_MSC_VER) 
                    asuUdpRcvBuffs[0].len = 12;
                    asuUdpRcvBuffs[0].buf = (char *)&suUdpSeg;
#else
                    asuUdpRcvBuffs[0].iov_len  = 12;
                    asuUdpRcvBuffs[0].iov_base = (char *)&suUdpSeg;
#endif

                    // Always write to the beginning of the buffer while waiting for the first segment
                    // The first UDP packet is guaranteed to fit our default starting size
                    if (m_suNetHandle[iHandle].bGotFirstSegment == bFALSE)
                        {
#if defined(_MSC_VER) 
                        asuUdpRcvBuffs[1].len = m_suNetHandle[iHandle].ulRcvBufferLen;
                        asuUdpRcvBuffs[1].buf = m_suNetHandle[iHandle].pchRcvBuffer;
#else
                        asuUdpRcvBuffs[1].iov_len  = m_suNetHandle[iHandle].ulRcvBufferLen;
                        asuUdpRcvBuffs[1].iov_base = m_suNetHandle[iHandle].pchRcvBuffer;
#endif
                        }
                    else
                        {
#if defined(_MSC_VER) 
                        asuUdpRcvBuffs[1].len = m_suNetHandle[iHandle].ulRcvBufferLen - suUdpSeg.uSegmentOffset;
                        asuUdpRcvBuffs[1].buf = &(m_suNetHandle[iHandle].pchRcvBuffer[suUdpSeg.uSegmentOffset]);
#else
                        asuUdpRcvBuffs[1].iov_len  = m_suNetHandle[iHandle].ulRcvBufferLen - suUdpSeg.uSegmentOffset;
                        asuUdpRcvBuffs[1].iov_base = &(m_suNetHandle[iHandle].pchRcvBuffer[suUdpSeg.uSegmentOffset]);
#endif
                        }

                    UdpRcvFlags = 0;
                    iResult = WSARecv(m_suNetHandle[iHandle].suIrigSocket, asuUdpRcvBuffs, 2, &ulBytesRcvd, &UdpRcvFlags, NULL, NULL);
                    if (iResult != 0)
                        {
                        m_suNetHandle[iHandle].bBufferReady     = bFALSE;
                        m_suNetHandle[iHandle].bGotFirstSegment = bFALSE;
                        m_suNetHandle[iHandle].ulBufferPosIdx   = 0L;
                        return -1;
                        }

//printf("Offset = %u\n", suUdpSeg.uSegmentOffset);

                    // Make sure we can access Ch 10 header info
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
                        ((suUdpSeg.uSegmentOffset + ulBytesRcvd - 12) >= psuHeader->ulPacketLen)) // Last UDP buffer
                        {
    //if ((suUdpSeg.uSegmentOffset + ulBytesRcvd - 12) > psuHeader->ulPacketLen)
    //    printf("Last packet too long");
                        m_suNetHandle[iHandle].bBufferReady     = bTRUE;
                        m_suNetHandle[iHandle].bGotFirstSegment = bFALSE;
                        m_suNetHandle[iHandle].ulBufferPosIdx     = 0L;
                        } // end if got first and last packet

                    break;

                default :
                    // RETURN ERRROR
                    break;
                } // end switch on UDP packet type
            } // end while reading for a complete buffer
        } // end if called and buffer not ready

    // Copy data to the user buffer
    iCopySize = min(m_suNetHandle[iHandle].ulRcvBufferDataLen - m_suNetHandle[iHandle].ulBufferPosIdx, iBuffSize);
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


#ifdef __cplusplus
} // end namespace
#endif
