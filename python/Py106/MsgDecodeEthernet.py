'''
Created on Jan 4, 2012

@author: rb45
'''

import sys
import ctypes
#import socket

import Py106.Packet as Packet
import Py106.Status as Status

# ---------------------------------------------------------------------------
# Ethernet packet data structures
# ---------------------------------------------------------------------------

# Ethernet F0 packet channel specific data word

class ChanSpec_EthF0(ctypes.Structure):
    ''' Ethernet Format 0 Channel Specific Data Word '''
    _pack_   = 1
    _fields_ = [("MsgCnt",          ctypes.c_uint32, 16),
                ("Reserved",        ctypes.c_uint32,  9),
                ("TTB",             ctypes.c_uint32,  3),
                ("Format",          ctypes.c_uint32,  4)]
    
# Ethernet Format 0 message intrapacket header fields
    
class HdrEthF0_Fields(ctypes.Structure):
    ''' Ethernet F0 intra-packet header '''
    _pack_   = 1
    _fields_ = [("PktTime",         ctypes.c_uint64   ),
                ("MsgDataLen",      ctypes.c_uint32, 14),
                ("LengthError",     ctypes.c_uint32,  1),
                ("DataCrcError",    ctypes.c_uint32,  1),
                ("NetID",           ctypes.c_uint32,  8),
                ("Speed",           ctypes.c_uint32,  4),
                ("Content",         ctypes.c_uint32,  2),
                ("FrameError",      ctypes.c_uint32,  1),
                ("FrameCrcError",   ctypes.c_uint32,  1)]

class HdrEthF0(ctypes.Union):
    ''' Ethernet F0 intra-packet header '''
    def __init(self, Value=0):
        self._fields_.Value = Value

    _pack_   = 1
    _fields_ = [("Value",           ctypes.c_uint16 * 8),
                ("Field",           HdrEthF0_Fields)]

# Ethernet frame structure
    
class EthernetF0_Physical_FullMAC_Fields(ctypes.Structure):
    ''' Ethernet physical frame '''
    _pack_   = 1
    _fields_ = [("DstAddr",         ctypes.c_uint8 * 6),
                ("SrcAddr",         ctypes.c_uint8 * 6),
                ("TypeLen",         ctypes.c_uint16),    # Byte swapped!
                ("Data",            ctypes.c_uint8 * 1500)]

class EthernetF0_Physical_FullMAC(ctypes.Union):
    ''' Ethernet physical frame '''
    _pack_   = 1
    _fields_ = [("Value",           ctypes.c_uint8 * 1522),
                ("Field",           EthernetF0_Physical_FullMAC_Fields)]

# IPv4 Header
    
class IPv4_Header(ctypes.Structure):
    ''' IPv4 packet header '''
    _pack_   = 1
    _fields_ = [("TotalLen",        ctypes.c_uint32, 16),
                ("ECN",             ctypes.c_uint32,  2),
                ("DSCP",            ctypes.c_uint32,  6),
                ("IHL",             ctypes.c_uint32,  4),
                ("Version",         ctypes.c_uint32,  4),
                ("FragOffset",      ctypes.c_uint32, 13),
                ("Flags",           ctypes.c_uint32,  3),
                ("Ident",           ctypes.c_uint32, 16),
                ("HeaderChksum",    ctypes.c_uint32, 16),
                ("Protocol",        ctypes.c_uint32,  8),
                ("TTL",             ctypes.c_uint32,  8),
                ("SrcIPAddr",       ctypes.c_uint32),
                ("DstIPAddr",       ctypes.c_uint32)]
    
# First / Next state

class CurrMsg_EthF0(ctypes.Structure):
    ''' Data structure for the current Ethernet F0 message info structure '''
    _pack_   = 1
    _fields_ = [("FrameNumber",     ctypes.c_uint32),
                ("PktDataLength",   ctypes.c_uint32),
                ("pChanSpec",       ctypes.POINTER(ChanSpec_EthF0)),
                ("pEthernetHdr",    ctypes.POINTER(HdrEthF0)),
                ("pData",           ctypes.POINTER(EthernetF0_Physical_FullMAC))]


# ---------------------------------------------------------------------------
# Direct calls into the IRIG 106 dll
# ---------------------------------------------------------------------------

def I106_Decode_FirstEthernetF0(header, msg_buffer, curr_msg):
    ret_status = Packet.IrigDataDll.enI106_Decode_FirstEthernetF0(ctypes.byref(header), ctypes.byref(msg_buffer),  ctypes.byref(curr_msg))
    return ret_status

def I106_Decode_NextEthernetF0(curr_msg):
    ret_status = Packet.IrigDataDll.enI106_Decode_NextEthernetF0(ctypes.byref(curr_msg))
    return ret_status

# ---------------------------------------------------------------------------
# Static methods
# ---------------------------------------------------------------------------

def swap32(x):
    return (((x << 24) & 0xFF000000) |
            ((x <<  8) & 0x00FF0000) |
            ((x >>  8) & 0x0000FF00) |
            ((x >> 24) & 0x000000FF))
    
def swap16(x):
    return (((x <<  8) & 0xFF00) |
            ((x >>  8) & 0x00FF))
    
def EthAddr2Str(EthAddr):
    return "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x" % (EthAddr[0],EthAddr[1],EthAddr[2],EthAddr[3],EthAddr[4],EthAddr[5])
    
def EthAddr2Long(EthAddr):
    EthLong = \
        (EthAddr[0] << 40) | (EthAddr[1] << 32) | (EthAddr[2] << 24) | \
        (EthAddr[3] << 16) | (EthAddr[4] <<  8) | (EthAddr[5]      )
    return EthLong

def Long2EthAddr(LongEthAddr):
    EthAddr = [0, 0, 0, 0, 0, 0]
    EthAddr[0] = (LongEthAddr >> 40) & 0xff
    EthAddr[1] = (LongEthAddr >> 32) & 0xff
    EthAddr[2] = (LongEthAddr >> 24) & 0xff
    EthAddr[3] = (LongEthAddr >> 16) & 0xff
    EthAddr[4] = (LongEthAddr >>  8) & 0xff
    EthAddr[5] = (LongEthAddr      ) & 0xff
    return EthAddr

# ---------------------------------------------------------------------------
# Decode Ethernet class
# ---------------------------------------------------------------------------

class DecodeEthernetF0(object):
    '''
    Decode Ethernet Format 0 packets
    '''

    def __init__(self, PacketIO):
        '''
        Constructor
        '''
        self.PacketIO = PacketIO
        self.CurrMsg  = CurrMsg_EthF0()
     
    def decode_first_ethernetf0(self):
        RetStatus = I106_Decode_FirstEthernetF0(self.PacketIO.Header, self.PacketIO.Buffer,  self.CurrMsg)
        return RetStatus
        
    def decode_next_ethernetf0(self):
        RetStatus = I106_Decode_NextEthernetF0(self.CurrMsg)
        return RetStatus
 
    def msgs(self):
        ''' Iterator of individual Ethernet messages '''
        RetStatus = self.decode_first_ethernetf0()
        while RetStatus == Packet.Status.OK: 
            yield self.CurrMsg
            RetStatus = self.decode_next_ethernetf0()
        

# ---------------------------------------------------------------------------
# Module initialization
# ---------------------------------------------------------------------------

if __name__=='__main__':
    print ("IRIG 106 Decode Ethernet")
    
    import Py106.Time
    
    # Make IRIG 106 library classes
    PktIO       = Packet.IO()
    TimeUtils   = Py106.Time.Time(PktIO)
    DecodeEthF0 = DecodeEthernetF0(PktIO)
    
    # Initialize counts variables
    PacketCount = 0
    MsgCount = 0
    DataType = Packet.DataType()
    
    if len(sys.argv) > 1 :
        RetStatus = PktIO.open(sys.argv[1], Packet.FileMode.READ)
        if RetStatus != Status.OK :
            print ("Error opening data file %s" % (sys.argv[1]))
            sys.exit(1)
    else :
        print ("Usage : MsgDecodeEthernet.py <filename>")
        sys.exit(1)

    RetStatus = TimeUtils.SyncTime(False, 0)
    if RetStatus != Status.OK:
        print ("Sync Status = %s" % Status.Message(RetStatus))
        sys.exit(1)
    
    TalkerListener = {}
    for PktHdr in PktIO.packet_headers():
        if PktHdr.DataType == Packet.DataType.ETHERNET_FMT_0:
            PacketCount += 1
            PktIO.read_data()
            MsgCnt = 0
            for Msg in DecodeEthF0.msgs():
                Msg.pData.contents.TypeLen = swap16(Msg.pData.contents.Field.TypeLen)
                msg_time = TimeUtils.RelInt2IrigTime(Msg.pEthernetHdr.contents.Field.PktTime)
                
                # Print out the packet data
                sys.stdout.write ("%s Ch %3i   %s %s %4x" % (  \
                    msg_time,                           \
                    PktIO.Header.ChID,                 \
                    EthAddr2Str(Msg.pData.contents.Field.DstAddr), \
                    EthAddr2Str(Msg.pData.contents.Field.SrcAddr), \
                    Msg.pData.contents.Field.TypeLen))
                TalkerListenerIdx = (EthAddr2Long(Msg.pData.contents.Field.SrcAddr), EthAddr2Long(Msg.pData.contents.Field.DstAddr))
                if TalkerListenerIdx in TalkerListener:
                    TalkerListener[TalkerListenerIdx] += 1
                else:
                    TalkerListener[TalkerListenerIdx] = 1
                print ("")
                
                MsgCnt += 1
#            print ("MsgCnt = %d" % (MsgCnt))
                
    PktIO.close()

    # Print out talkers and listeners
    #       00:17:f2:d4:09:32 --> 01:00:5e:60:e0:0a - 6540 packets
    print ("")
    print ("  Source Addr         Destination Addr    Packets")
    print ("-----------------     -----------------   -------")
    for (SrcAddr, DstAddr) in TalkerListener.keys():
        print ("%s --> %s - %d" % \
               (EthAddr2Str(Long2EthAddr(SrcAddr)), \
                EthAddr2Str(Long2EthAddr(DstAddr)), \
                TalkerListener[(SrcAddr, DstAddr)]))
        
    print ("Ch 10 Ethernet Packets = %d" % PacketCount)
    