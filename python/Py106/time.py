
import sys
import ctypes
import traceback

import datetime
import calendar

#from datetime import date
#from time     import time

import Py106.packet as packet
import Py106.status as status
import Py106.MsgDecodeTime as MsgDecodeTime


# ---------------------------------------------------------------------------
# IRIG time constants
# ---------------------------------------------------------------------------

class DateFmt(object):
    """ Packet Message Types """
    DAY      = 0x00     # Day of Year format
    DMY      = 0x01     # Day, Month, Year format


# ---------------------------------------------------------------------------
# IRIG time structures
# ---------------------------------------------------------------------------

class _ctIrig106Time(ctypes.Structure):
    """ Ctypes data structure for IRIG library time representation """
    _pack_   = 1
    _fields_ = [("Secs",            ctypes.c_uint64),   # This is a time_t
                ("Fracs",           ctypes.c_uint32),   # LSB = 100ns
                ("DateFormat",      ctypes.c_uint32)]   # Day or DMY format

     
    def set_from_irig_time(self, irig_time_in):
        self.Secs       = calendar.timegm(irig_time_in.time.timetuple())
        self.Fracs      = irig_time_in.time.microsecond * 10
        self.DateFormat = irig_time_in.dt_format

        
# ---------------------------------------------------------------------------
# Direct calls into the IRIG 106 dll
# ---------------------------------------------------------------------------

def I106_SetRelTime(handle, irig_time, rel_time):
    # handle - IRIG file handle
    # irig_time - Py106 time object holding absolute time
    # rel_time - 6 byte ctypes array with relative time count (usually from the packet header)
    ctype_irig_time = _ctIrig106Time()
    ctype_irig_time.set_from_IrigTime(irig_time)
    ret_status = packet.irig_data_dll.enI106_SetRelTime(handle, ctypes.byref(ctype_irig_time), rel_time)
    return ret_status


def I106_SyncTime(handle, require_sync, time_limit):
    """ Search for time and set relative to absolute time reference"""
    # handle - IRIG file handle
    # require_sync - Bool indicating whether time source must be externally sync'ed to be valid
    # time_limit = Maximum amount of time in seconds to scan forward in the file 
    return packet.irig_data_dll.enI106_SyncTime(handle, require_sync, time_limit)


def I106_Rel2IrigTime(handle, rel_time):
    """ Convert a 6 byte relative time to absolute time """
    # handle - IRIG file handle
    # rel_time - 6 byte ctypes array with relative time count (usually from the packet header)
    # irig_time - Py106 time object holding absolute time
    ctype_irig_time = _ctIrig106Time()
    ret_status = packet.irig_data_dll.enI106_Rel2IrigTime(handle, rel_time, ctypes.byref(ctype_irig_time))
    irig_time = IrigTime()
    irig_time.set_from_ctIrig106Time(ctype_irig_time)
    return ret_status, irig_time


def I106_RelInt2IrigTime(handle, rel_time):
    """ Convert a 64 bit integer relative time to absolute time """
    # handle - IRIG file handle
    # rel_time - 64 integer with relative time count (usually from a message header)
    # irig_time - Py106 time object holding absolute time
    ctype_rel_time  = ctypes.c_uint64(rel_time)
    ctype_irig_time = _ctIrig106Time()
    ret_status = packet.irig_data_dll.enI106_RelInt2IrigTime(handle, ctype_rel_time, ctypes.byref(ctype_irig_time))
    irig_time = IrigTime()
    irig_time.set_from_ctIrig106Time(ctype_irig_time)
    return ret_status, irig_time


def I106_IrigTime2String(irig_time_in):
    """ Convert a Py106 IRIG time to a string """
    # irig_time_in - Py106 time object holding absolute time
    # Returns a string representation of time in either Day or DMY format
    try:
        assert type(irig_time_in) is IrigTime
        iTime            = _ctIrig106Time()
        iTime.Secs       = calendar.timegm(irig_time_in.time.timetuple())
        iTime.Fracs      = irig_time_in.time.microsecond * 10
        iTime.DateFormat = irig_time_in.dt_format
        time_string = packet.irig_data_dll.IrigTime2String(ctypes.byref(iTime))
        return time_string.decode('ascii')
    except AssertionError:
        print ("Not class IrigTime - %s" % (type(irig_time_in)))
        return ""
    except:
        return ""
                        

# ---------------------------------------------------------------------------
# IRIG time classes
# ---------------------------------------------------------------------------

# IRIG time value in Python format

class IrigTime(object):
    """ Py106 native IRIG time value """
    def __init___(self):
        self.time = datetime()
        self.dt_format = DateFmt.DAY
        
    def __str__(self):
        return I106_IrigTime2String(self)
    
    def set_from_ctIrig106Time(self, ctype_irig_time):
        self.time      = datetime.datetime.utcfromtimestamp(ctype_irig_time.Secs) 
        self.time      = self.time.replace(microsecond=ctype_irig_time.Fracs//10)
        self.dt_format = ctype_irig_time.DateFormat


# ---------------------------------------------------------------------------

# Time calculations for a packet in a PackedIO buffer

class Time(object):
    """ IRIG time handling for an open file """

    def __init__(self, packet_io):
        self.packet_io = packet_io
                
    def set_rel_time(self):
        """Set relative time to clock time"""
        # This routine assumes a time packet has been read and is sitting in 
        # the packet_io buffer

        # Bail out if the data isn't a time packet
        if self.packet_io.header.data_type != packet.DataType.IRIG_TIME:
            return

        # Bail out if there is not data in the packet data buffer (i.e. the packet
        # data wasn't read
        if self.packet_io.buffer._length_ == 0:
            return
                
        # Decode the time in the packet buffer
        ret_status, irig_time = MsgDecodeTime.I106_Decode_TimeF1(self.packet_io.header, self.packet_io.buffer)
        if ret_status != status.OK:
            return
        
        I106_SetRelTime(self.packet_io._handle, irig_time, self.packet_io.header.ref_time)
        return
    

    def sync_time(self, require_sync, time_limit):
        return I106_SyncTime(self.packet_io._handle, require_sync, time_limit)
    

    def rel_to_irig_time(self, rel_time):
        """ Calculate time from a Relative Time Counter value. 
            rel_time is 6 byte array typically from the packet header. 
            Returns a Py106 time value """
        try:
#            iTime = _ctIrig106Time()
            ret_status, irig_time = I106_Rel2IrigTime(self.packet_io._handle, rel_time)
            if ret_status != status.OK:
                return None
        except Exception:
#            traceback.print_exc()
            return None
        
        return irig_time

    def rel_int_to_irig_time(self, rel_time):
        """ Calculate time from a Relative Time Counter value. 
            rel_time is 64 bit int typically from a message header. 
            Returns a Py106 time value """
        try:
#            iTime = _ctIrig106Time()
            ret_status, irig_time = I106_RelInt2IrigTime(self.packet_io._handle, rel_time)
            if ret_status != status.OK:
                return None
        except Exception:
            traceback.print_exc()
            return None
    
        return irig_time

# ---------------------------------------------------------------------------
# Module initialization
# ---------------------------------------------------------------------------

packet.irig_data_dll.IrigTime2String.restype = ctypes.c_char_p

# This test code just opens an IRIG file and does a few time tests

if __name__=='__main__':
    print ("IRIG 106 Time")
    packet_io     = packet.IO()
    time_utils = Time(packet_io)
    
    if len(sys.argv) > 1 :
        open_status = packet_io.open(sys.argv[1], packet.FileMode.READ)
        if open_status != status.OK :
            print ("Error opening data file %s" % (sys.argv[1]))
            sys.exit(1)
    else :
        print ("Usage : time.py <filename>")
        sys.exit(1)
    
    sync_status = time_utils.sync_time(False, 0)
    if sync_status != status.OK:
        print ("Sync Status = %s" % status.Message(sync_status))
        sys.exit(1)
    
    # Read IRIG headers
    for pkt_hdr in packet_io.packet_headers():
        int_ref_time = pkt_hdr.ref_time[5] << 8*5 | \
                       pkt_hdr.ref_time[4] << 8*4 | \
                       pkt_hdr.ref_time[3] << 8*3 | \
                       pkt_hdr.ref_time[2] << 8*2 | \
                       pkt_hdr.ref_time[1] << 8   | \
                       pkt_hdr.ref_time[0]
#        IrigTime = iTime.Rel2IrigTime(IntRefTime)
        pkt_time = time_utils.rel_to_irig_time(pkt_hdr.ref_time)
        print ("'%s' %012X  ch_id %3d  Data Type %-16s" % ( \
            pkt_time, int_ref_time, pkt_hdr.ch_id, packet.DataType.type_name(pkt_hdr.data_type)))
#            IrigTime2String(pkt_time), IntRefTime, pkt_hdr.ch_id, packet.DataType.TypeName(pkt_hdr.DataType))
#            pkt_time.time.isoformat(' '), IntRefTime, pkt_hdr.ch_id, packet.DataType.TypeName(pkt_hdr.DataType))
                
    packet_io.close()

    
