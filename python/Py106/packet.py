"""
IRIG 106 Data DLL - This module provides an interface to the IRIG 106 data DLL.

The IRIG 106 DLL irig106.dll must be present somewhere in the system where
Windows can find it.

Message data structures are based on the ctypes module.  The main implication of
this is that to use data first the data buffer needs to be cast to the
appropriate data structure.  Then the fields are accessed using the '.contents'
attribute.

"""

import os.path
import sys
import platform
import ctypes
#import datetime
import Py106.status as status


# ---------------------------------------------------------------------------
# IRIG 106 data structures
# ---------------------------------------------------------------------------

class Header(ctypes.Structure):
    """ Data structure for IRIG 106 packet primary and secondary header """
    _pack_   = 1
    _fields_ = [("sync",            ctypes.c_uint16),
                ("ch_id",           ctypes.c_uint16),
                ("packet_len",      ctypes.c_uint32),
                ("data_len",        ctypes.c_uint32),
                ("hdr_ver",         ctypes.c_uint8),
                ("seq_num",         ctypes.c_uint8),
                ("packet_flags",    ctypes.c_uint8),
                ("data_type",       ctypes.c_uint8),
                ("ref_time",        ctypes.c_uint8 * 6),
                ("checksum",        ctypes.c_uint16),
                ("time",            ctypes.c_uint32 * 2),
                ("reserved",        ctypes.c_uint16),
                ("sec_checksum",    ctypes.c_uint16)]

# ---------------------------------------------------------------------------
# IRIG 106 constants
# ---------------------------------------------------------------------------


class FileMode():
    """ Data file open mode """
    CLOSED              = 0
    READ                = 1    # Open an existing file for reading
    OVERWRITE           = 2    # Create a new file or overwrite an exising file
    APPEND              = 3    # Append data to the end of an existing file
    READ_IN_ORDER       = 4    # Open an existing file for reading in time order
    READ_NET_STREAM     = 5    # Open network data stream


class DataType(object):
    """ Packet Message Types """
    COMPUTER_0          = 0x00
    USER_DEFINED        = 0x00
    COMPUTER_1          = 0x01
    TMATS               = 0x01
    COMPUTER_2          = 0x02
    RECORDING_EVENT     = 0x02
    COMPUTER_3          = 0x03
    RECORDING_INDEX     = 0x03
    COMPUTER_4          = 0x04
    COMPUTER_5          = 0x05
    COMPUTER_6          = 0x06
    COMPUTER_7          = 0x07
    PCM_FMT_0           = 0x08
    PCM_FMT_1           = 0x09
    PCM_FMT_2           = 0x0A
    IRIG_TIME           = 0x11
    IRIG_NETWORK_TIME   = 0x12
    MIL1553_FMT_1       = 0x19
    MIL1553_16PP194     = 0x1A      # 16PP194 Bus
    ANALOG              = 0x21
    DISCRETE            = 0x29
    MESSAGE             = 0x30
    ARINC_429_FMT_0     = 0x38
    VIDEO_FMT_0         = 0x40
    VIDEO_FMT_1         = 0x41
    VIDEO_FMT_2         = 0x42
    VIDEO_FMT_3         = 0x43
    VIDEO_FMT_4         = 0x44
    IMAGE_FMT_0         = 0x48
    IMAGE_FMT_1         = 0x49
    IMAGE_FMT_2         = 0x4A
    UART_FMT_0          = 0x50
    IEEE1394_FMT_0      = 0x58
    IEEE1394_FMT_1      = 0x59
    PARALLEL_FMT_0      = 0x60
    ETHERNET_FMT_0      = 0x68
    ETHERNET_FMT_1      = 0x69
    TSPI_FMT_0          = 0x70
    TSPI_FMT_1          = 0x71
    TSPI_FMT_2          = 0x72
    CAN_BUS             = 0x78
    FIBRE_CHAN_FMT_0    = 0x79
    FIBRE_CHAN_FMT_1    = 0x7A

    @staticmethod
    def type_name(type_num):
        name = {DataType.USER_DEFINED       : "User Defined",
                DataType.TMATS              : "TMATS",
                DataType.RECORDING_EVENT    : "Event",
                DataType.RECORDING_INDEX    : "Index",
                DataType.COMPUTER_4         : "Computer Generated 4",
                DataType.COMPUTER_5         : "Computer Generated 5",
                DataType.COMPUTER_6         : "Computer Generated 6",
                DataType.COMPUTER_7         : "Computer Generated 7",
                DataType.PCM_FMT_0          : "PCM Format 0",
                DataType.PCM_FMT_1          : "PCM Format 1",
                DataType.IRIG_TIME          : "Time",
                DataType.MIL1553_FMT_1      : "1553",
                DataType.MIL1553_16PP194    : "16PP194",
                DataType.ANALOG             : "Analog",
                DataType.DISCRETE           : "Discrete",
                DataType.MESSAGE            : "Message",
                DataType.ARINC_429_FMT_0    : "ARINC 429",
                DataType.VIDEO_FMT_0        : "Video Format 0",
                DataType.VIDEO_FMT_1        : "Video Format 1",
                DataType.VIDEO_FMT_2        : "Video Format 2",
                DataType.IMAGE_FMT_0        : "Image Format 0",
                DataType.IMAGE_FMT_1        : "Image Format 1",
                DataType.UART_FMT_0         : "UART",
                DataType.IEEE1394_FMT_0     : "IEEE 1394 Format 0",
                DataType.IEEE1394_FMT_1     : "IEEE 1394 Format 1",
                DataType.PARALLEL_FMT_0     : "Parallel",
                DataType.ETHERNET_FMT_0     : "Ethernet",
                DataType.CAN_BUS            : "CAN Bus",
                DataType.FIBRE_CHAN_FMT_0   : "Fibre Channel Format 0",
                DataType.FIBRE_CHAN_FMT_1   : "Fibre Channel Format 1"}
        return name.get(type_num, "Undefined")


# ---------------------------------------------------------------------------
# Direct calls into the IRIG 106 dll
# ---------------------------------------------------------------------------

def I106_Ch10Open(file_name, file_mode):
    """ Open IRIG 106 Ch 10 data file """
    # file_name - File name to open
    # file_mode - Py106 FileMode() class value
    # Returns handle - IRIG file handle
    handle = ctypes.c_int32(0)
    # ret_status = irig_data_dll.enI106Ch10Open(ctypes.byref(handle), file_name,
    #                                         file_mode)
    ret_status = irig_data_dll.enI106Ch10Open(
        ctypes.byref(handle), file_name.encode('ascii'), file_mode)
    return (ret_status, handle)


def I106_Ch10Close(handle):
    """ Close IRIG 106 Ch 10 data file """
    # handle - IRIG file handle
    ret_status = irig_data_dll.enI106Ch10Close(handle)
    return ret_status


def I106_Ch10ReadNextHeader(handle, pkt_header):
    """ Read next packet header """
    # handle - IRIG file handle
    # pkt_header - Py106 Header() class, mutable
    ret_status = irig_data_dll.enI106Ch10ReadNextHeader(handle,
                                                      ctypes.byref(pkt_header))
    return ret_status


def I106_Ch10ReadPrevHeader(handle, pkt_header):
    """ Read previous packet header """
    # handle - IRIG file handle
    # pkt_header - Py106 class Header(), mutable
    ret_status = irig_data_dll.enI106Ch10ReadPrevHeader(handle,
                                                      ctypes.byref(pkt_header))
    return ret_status


def I106_Ch10ReadData(handle, buff_size, data_buff):
    # handle - IRIG file handle
    # buff_size - Size of data_buff
    # data_buff - Ctypes string buffer, mutable
    ret_status = irig_data_dll.enI106Ch10ReadData(handle, buff_size,
                                                ctypes.byref(data_buff))
    return ret_status


def I106_Ch10FirstMsg(handle):
    # handle - IRIG file handle
    ret_status = irig_data_dll.enI106Ch10FirstMsg(handle)
    return ret_status


def I106_Ch10LastMsg(handle):
    # handle - IRIG file handle
    ret_status = irig_data_dll.enI106Ch10LastMsg(handle)
    return ret_status


def I106_Ch10SetPos(handle, offset):
    # handle - IRIG file handle
    # offset - file offset
    ret_status = irig_data_dll.enI106Ch10SetPos(handle, offset)
    return ret_status


def I106_Ch10GetPos(handle):
    # handle - IRIG file handle
    offset = ctypes.c_uint64(0)
    ret_status = irig_data_dll.enI106Ch10GetPos(handle, ctypes.byref(offset))
    return (ret_status, offset.value)


# ---------------------------------------------------------------------------
# IRIG IO class
# ---------------------------------------------------------------------------

class IO(object):
    """
    IRIG 106 packet data input / output
    """

    # Constructor
    # -----------

    def __init__(self):
        self._handle = ctypes.c_uint32(-1)
        self.header  = Header()
        self.buffer  = ctypes.create_string_buffer(0)

    # Open and close
    # --------------
    def open(self, filename, file_mode):
        """Open an IRIG file for reading or writing"""
        ret_status, self._handle = I106_Ch10Open(filename, file_mode)
        return ret_status

    def close(self):
        """Close an open IRIG file"""
        ret_status = I106_Ch10Close(self._handle)
        return ret_status

    # Read / Write
    # ------------
    def read_next_header(self):
        """Move to and read the next header"""
        ret_status = I106_Ch10ReadNextHeader(self._handle, self.header)
        return ret_status

    def read_prev_header(self):
        """Move to and read the previous header"""
        ret_status = I106_Ch10ReadPrevHeader(self._handle, self.header)
        return ret_status

    def read_data(self):
        """Read data portion of packet"""
        if self.header.packet_len > self.buffer._length_:
            self.buffer = ctypes.create_string_buffer(self.header.packet_len)
        ret_status = I106_Ch10ReadData(self._handle, self.buffer._length_,
                                      self.buffer)
        return ret_status

    def packet_headers(self, ch_ids=()):
        """Iterator of individual packet headers"""
        ret_status = self.read_next_header()
        while ret_status == status.OK:
            if (len(ch_ids) == 0) or (self.header.ch_id in ch_ids):
                yield self.header
            ret_status = self.read_next_header()

    # Other utility functions
    # -----------------------
    def first(self):
        """Set the position to the first packet in the file"""
        ret_status = I106_Ch10FirstMsg(self._handle)
        return ret_status

    def last(self):
        """Set position to the last packet in the file"""
        ret_status = I106_Ch10LastMsg(self._handle)
        return ret_status

    def set_pos(self, offset):
        """Set the current file offset in bytes"""
        ret_status = I106_Ch10SetPos(self._handle, offset)
        return ret_status

    def get_pos(self):
        """Get the current file offset in bytes"""
        (ret_status, offset) = I106_Ch10GetPos(self._handle)
        return (ret_status, offset)


# ---------------------------------------------------------------------------
# Module initialization
# ---------------------------------------------------------------------------

# Load the correct dynamic library based on the platform
if platform.system() == "Windows":
    # 32 bit
    if sys.maxsize < 2**32:
        DLL_FILENAME = "irig106.dll"
    # 64 bit
    else:
        DLL_FILENAME = "irig106-x64.dll"
else:
    DLL_FILENAME = "libirig106.so"

script_file_path, script_filename = os.path.split(__file__)
full_dll_filename = os.path.join(script_file_path, DLL_FILENAME)
#print ("File               %s\n" % (__file__))
#print ("File Path          %s\n" % (FilePath))
#print ("Full DLL File Name %s\n" % (FullDllFileName))
#irig_data_dll = ctypes.cdll.LoadLibrary(DllFileName)
irig_data_dll = ctypes.cdll.LoadLibrary(full_dll_filename)

# This test code just opens an IRIG file and does a histogram of the
# data types

if __name__ == '__main__':

    print("IRIG 106 packet_io")
    pkt_io = IO()

    # Initialize counts variables
    counts = {}

    if len(sys.argv) > 1:
        open_status = pkt_io.open(sys.argv[1], FileMode.READ)
        if open_status != status.OK:
            print("Error opening data file '%s'" % (sys.argv[1]))
            sys.exit(1)
    else:
        print("Usage : packet.py <filename>")
        sys.exit(1)

#    The old traditional (aka FORTRAN) way of doing it
#    while True:
#        ret_status = packet_io.read_next_header()
#        if ret_status != status.OK:
#            break
#        if Counts.has_key(packet_io.header.DataType):
#           Counts[packet_io.header.DataType] += 1
#        else:
#            Counts[packet_io.header.DataType]  = 1

    # Using Python iteration
    for pkt_hdr in pkt_io.packet_headers():
#        if Counts.has_key(pkt_hdr.DataType):
        if pkt_hdr.data_type in counts:
            counts[pkt_hdr.data_type] += 1
        else:
            counts[pkt_hdr.data_type] = 1

    pkt_io.close()

    for data_type_num in counts:
        print("Data Type %-24s Counts = %d" % (DataType.type_name(data_type_num),
                                               counts[data_type_num]))
