"""
Created on Jan 4, 2012

@author: rb45
"""

import sys
import ctypes

import Py106.packet as packet
import Py106.status as status

# ---------------------------------------------------------------------------
# 1553 packet data structures
# ---------------------------------------------------------------------------

# 1553 packet channel specific data word

class ChanSpec_1553F1(ctypes.Structure):
    """ 1553 Channel Specific Data Word """
    _pack_   = 1
    _fields_ = [("MsgCnt",          ctypes.c_uint32, 24),
                ("Reserved",        ctypes.c_uint32,  6),
                ("TTB",             ctypes.c_uint32,  2)]

# 1553 message intrapacket header fields
# Testing these with bit-wise operations can be quite a bit
# faster. These flags are in p1553Hdr.contents.Value[4]. Bit
# masks for each flag are shown below.

class Hdr1553_BlockStatus(ctypes.Structure):
    """ 1553 intra-packet header flags"""
    _pack_   = 1
    _fields_ = [("Reserved1",       ctypes.c_uint16, 3),
                ("WordError",       ctypes.c_uint16, 1),    # 0x0008
                ("SyncError",       ctypes.c_uint16, 1),    # 0x0010
                ("WordCntError",    ctypes.c_uint16, 1),    # 0x0020
                ("Reserved2",       ctypes.c_uint16, 3),
                ("RespTimeout",     ctypes.c_uint16, 1),    # 0x0200
                ("FormatError",     ctypes.c_uint16, 1),    # 0x0400
                ("RT2RT",           ctypes.c_uint16, 1),    # 0x0800
                ("MsgError",        ctypes.c_uint16, 1),    # 0x1000
                ("BusID",           ctypes.c_uint16, 1),    # 0x2000
                ("Reserved3",       ctypes.c_uint16, 2)]

class Hdr1553_Fields(ctypes.Structure):
    """ 1553 intra-packet header """
    _pack_   = 1
    _fields_ = [("PktTime",         ctypes.c_uint64),
                ("BlockStatus",     Hdr1553_BlockStatus),
                ("GapTime1",        ctypes.c_uint8),
                ("GapTime2",        ctypes.c_uint8),
                ("MsgLen",          ctypes.c_uint16)]

class Hdr1553(ctypes.Union):
    """ 1553 intra-packet header """
    def __init(self, value=0):
        self._fields_.Value = value

    _pack_   = 1
    _fields_ = [("Value",           ctypes.c_uint16 * 7),
                ("Field",           Hdr1553_Fields)]


# 1553 command word

class CmdWord_Fields(ctypes.Structure):
    """ 1553 Command Word broken down by field"""
    _pack_   = 1
    _fields_ = [("WordCnt",         ctypes.c_uint16, 5),
                ("SubAddr",         ctypes.c_uint16, 5),
                ("TR",              ctypes.c_uint16, 1),
                ("RTAddr" ,         ctypes.c_uint16, 5)]

class CmdWord(ctypes.Union):
    """ 1553 Command Word """
    def __init(self, value=0):
        self._fields_.Value = value

    def __repr__(self):
        TR = ("R", "T")
        return "{0:2d}-{1}-{2:02d}-{3:02d} ({4:04x})".format(
                self.Field.RTAddr,  \
                TR[self.Field.TR],  \
                self.Field.SubAddr, \
                self.Field.WordCnt, \
                self.Value)

    _pack_   = 1
    _fields_ = [("Value",           ctypes.c_uint16),
                ("Field",           CmdWord_Fields)]

# 1553 status word

class StatWord_Fields(ctypes.Structure):
    """ 1553 Command Word broken down by field"""
    _pack_   = 1
    _fields_ = [("TerminalFlag",     ctypes.c_uint16, 1),
                ("DynamicBusAccept", ctypes.c_uint16, 1),
                ("SubsystemFlag",    ctypes.c_uint16, 1),
                ("Busy",             ctypes.c_uint16, 1),
                ("BCastRcvd",        ctypes.c_uint16, 1),
                ("Reserved",         ctypes.c_uint16, 3),
                ("ServiceRequest",   ctypes.c_uint16, 1),
                ("Instrumentation",  ctypes.c_uint16, 1),
                ("MsgError",         ctypes.c_uint16, 1),
                ("RTAddr" ,          ctypes.c_uint16, 5)]

class StatWord(ctypes.Union):
    """ 1553 Status Word """
    def __init(self, Value=0):
        self._fields_.Value = Value

    _pack_   = 1
    _fields_ = [("Value",           ctypes.c_uint16),
                ("Field",           StatWord_Fields)]

# First / Next state

class CurrMsg_1553F1(ctypes.Structure):
    """ Data structure for the current 1553 message info structure """
    _pack_   = 1
    _fields_ = [("MsgNum",          ctypes.c_uint32),
                ("CurrOffset",      ctypes.c_uint32),
                ("DataLen",         ctypes.c_uint32),
                ("pChanSpec",       ctypes.POINTER(ChanSpec_1553F1)),
                ("p1553Hdr",        ctypes.POINTER(Hdr1553)),
                ("pCmdWord1",       ctypes.POINTER(CmdWord)),
                ("pCmdWord2",       ctypes.POINTER(CmdWord)),
                ("pStatWord1",      ctypes.POINTER(StatWord)),
                ("pStatWord2",      ctypes.POINTER(StatWord)),
                ("WordCnt",         ctypes.c_uint16),
                ("pData",           ctypes.POINTER(ctypes.c_uint16 * 32))]


# ---------------------------------------------------------------------------
# Direct calls into the IRIG 106 dll
# ---------------------------------------------------------------------------

def I106_Decode_First1553F1(header, msg_buffer, curr_msg):
    ret_status = packet.irig_data_dll.enI106_Decode_First1553F1(ctypes.byref(header), ctypes.byref(msg_buffer),  ctypes.byref(curr_msg))
    return ret_status

def I106_Decode_Next1553F1(curr_msg):
    ret_status = packet.irig_data_dll.enI106_Decode_Next1553F1(ctypes.byref(curr_msg))
    return ret_status

def I106_WordCnt1553(cmd_word):
    return packet.irig_data_dll.i1553WordCnt(cmd_word)

#def CmdWordStr(cmd_word):
#    Looks like szCmdWord wasn't exported from the DLL
#    cmd_word_string = packet.irig_data_dll.szCmdWord(cmd_word)
#    return cmd_word_string


# ---------------------------------------------------------------------------
# Static methods
# ---------------------------------------------------------------------------

def word_cnt(CommandWord):
    if type(CommandWord) is int:
        return packet.irig_data_dll.i1553WordCnt(ctypes.byref(ctypes.c_uint16(CommandWord)))
    elif type(CommandWord) is CmdWord:
        return packet.irig_data_dll.i1553WordCnt(ctypes.byref(ctypes.c_uint16(CommandWord.Value)))
    else:
        return None

# ---------------------------------------------------------------------------
# Decode 1553 class
# ---------------------------------------------------------------------------

class Decode1553F1(object):
    """
    Decode 1553 Format 1 packets
    """

    def __init__(self, packet_io):
        """
        Constructor
        """
        self.packet_io = packet_io
        self.CurrMsg  = CurrMsg_1553F1()

    def decode_first1553f1(self):
        ret_status = I106_Decode_First1553F1(self.packet_io.header, self.packet_io.buffer,  self.CurrMsg)
        return ret_status

    def decode_next1553f1(self):
        ret_status = I106_Decode_Next1553F1(self.CurrMsg)
        return ret_status

    def word_cnt(self, CommandWord):
        if type(CommandWord) is int:
            return packet.irig_data_dll.i1553WordCnt(ctypes.byref(ctypes.c_uint16(CommandWord)))
        elif type(CommandWord) is CmdWord:
            return packet.irig_data_dll.i1553WordCnt(ctypes.byref(ctypes.c_uint16(CommandWord.Value)))
        else:
            return None

    def msgs(self):
        """ Iterator of individual 1553 messages """
        ret_status = self.decode_first1553f1()
        while ret_status == packet.status.OK:
            yield self.CurrMsg
            ret_status = self.decode_next1553f1()


# ---------------------------------------------------------------------------
# Module initialization
# ---------------------------------------------------------------------------

# packet.irig_data_dll.szCmdWord.restype = ctypes.c_char_p


# This test code just opens an IRIG file and does a histogram of the
# data types

if __name__=='__main__':
    print ("IRIG 106 Decode 1553")

    import Py106.time

    # Make IRIG 106 library classes
    pkt_io     = packet.IO()
    time_utils = Py106.time.Time(pkt_io)
    decode1553 = Decode1553F1(pkt_io)

    # Initialize counts variables
    TR = ("R", "T")
    packet_count = 0
    msg_count = 0

    if len(sys.argv) > 1 :
        open_status = pkt_io.open(sys.argv[1], packet.FileMode.READ)
        if open_status != status.OK :
            print ("Error opening data file %s" % (sys.argv[1]))
            sys.exit(1)
    else :
        print ("Usage : MsgDecodeTime.py <filename>")
        sys.exit(1)

    time_sync_status = time_utils.sync_time(False, 0)
    if time_sync_status != status.OK:
        print ("Sync Status = %s" % status.Message(time_sync_status))
        sys.exit(1)

    for pkt_hdr in pkt_io.packet_headers():
        if pkt_hdr.data_type == packet.DataType.MIL1553_FMT_1:
            packet_count += 1
            pkt_io.read_data()
            msg_cnt = 0
            for msg in decode1553.msgs():
#                time_utils.RelInt2IrigTime()
                WC = decode1553.word_cnt(msg.pCmdWord1.contents.Value)
                msg_time = time_utils.rel_int_to_irig_time(msg.p1553Hdr.contents.Field.PktTime)
                sys.stdout.write("%s Ch %3i   %2i-%s-%02i-%02i (%04x)  " % (  \
                    msg_time,                               \
                    pkt_io.header.ch_id,                    \
                    msg.pCmdWord1.contents.Field.RTAddr,    \
                    TR[msg.pCmdWord1.contents.Field.TR],    \
                    msg.pCmdWord1.contents.Field.SubAddr,   \
                    msg.pCmdWord1.contents.Field.WordCnt,   \
                    msg.pCmdWord1.contents.Value))
                if msg.p1553Hdr.contents.Field.BlockStatus.MsgError == 0:
                    for iDataIdx in range(WC):
                        sys.stdout.write("%04x " % msg.pData.contents[iDataIdx])
                else:
                    sys.stdout.write("Msg Error")
                print ("")

                msg_count += 1
#            print ("MsgCnt = %d" % (MsgCnt))

    pkt_io.close()

    print ("1553 Packets = %d" % packet_count)