import sys
import ctypes

import Py106.packet as packet
import Py106.status as status
import Py106.time as time


# ---------------------------------------------------------------------------
# IRIG Time packet data structures
# ---------------------------------------------------------------------------

# Time packet channel specific data word

class ChanSpec_Time(ctypes.Structure):
    """ IRIG Time Channel Specific Data Word """
    _pack_   = 1
    _fields_ = [("Source",          ctypes.c_uint32,  4),
                ("TimeFormat",      ctypes.c_uint32,  4),
                ("LeapYear",        ctypes.c_uint32,  1),
                ("DateFormat",      ctypes.c_uint32,  1),
                ("Reserved",        ctypes.c_uint32, 22)]
    
# 1553 message intrapacket header fields
    
class Time_Day(ctypes.Structure):
    """ IRIG Time in DAY format """
    _pack_   = 1
    _fields_ = [("uTmn",            ctypes.c_uint16, 4),    # Tens of milliseconds
                ("uHmn",            ctypes.c_uint16, 4),    # Hundreds of milliseconds
                ("uSn",             ctypes.c_uint16, 4),    # Units of seconds
                ("uTSn",            ctypes.c_uint16, 3),    # Tens of seconds
                ("Reserved1",       ctypes.c_uint16, 1),
                
                ("uMn",             ctypes.c_uint16, 4),    # Units of minutes
                ("uTMn",            ctypes.c_uint16, 3),    # Tens of minutes
                ("Reserved2",       ctypes.c_uint16, 1),
                ("uHn",             ctypes.c_uint16, 4),    # Units of hours
                ("uTHn",            ctypes.c_uint16, 2),    # Tens of Hours
                ("Reserved3",       ctypes.c_uint16, 2),
                
                ("uDn",             ctypes.c_uint16, 4),    # Units of day number
                ("uTDn",            ctypes.c_uint16, 4),    # Tens of day number
                ("uHDn",            ctypes.c_uint16, 2),    # Hundreds of day number
                ("Reserved4",       ctypes.c_uint16, 6)]


class Time_DMY(ctypes.Structure):
    """ IRIG Time in DMY format """
    _pack_   = 1
    _fields_ = [("uTmn",            ctypes.c_uint16, 4),    # Tens of milliseconds
                ("uHmn",            ctypes.c_uint16, 4),    # Hundreds of milliseconds
                ("uSn",             ctypes.c_uint16, 4),    # Units of seconds
                ("uTSn",            ctypes.c_uint16, 3),    # Tens of seconds
                ("Reserved1",       ctypes.c_uint16, 1),

                ("uMn",             ctypes.c_uint16, 4),    # Units of minutes
                ("uTMn",            ctypes.c_uint16, 3),    # Tens of minutes
                ("Reserved2",       ctypes.c_uint16, 1),
                ("uHn",             ctypes.c_uint16, 4),    # Units of hours
                ("uTHn",            ctypes.c_uint16, 2),    # Tens of Hours
                ("Reserved3",       ctypes.c_uint16, 2),

                ("uDn",             ctypes.c_uint16, 4),    # Units of day number
                ("uTDn",            ctypes.c_uint16, 4),    # Tens of day number
                ("uOn",             ctypes.c_uint16, 4),    # Units of month number
                ("uTOn",            ctypes.c_uint16, 1),    # Tens of month number
                ("Reserved4",       ctypes.c_uint16, 3),

                ("uYn",             ctypes.c_uint16, 4),    # Units of year number
                ("uTYn",            ctypes.c_uint16, 4),    # Tens of year number
                ("uHYn",            ctypes.c_uint16, 4),    # Hundreds of year number
                ("uOYn",            ctypes.c_uint16, 2),    # Thousands of year number
                ("Reserved5",       ctypes.c_uint16, 2)]

    
class Time_Packet(ctypes.Union):
    """ 1553 intra-packet header """
    def __init(self, Value=0):
        self._fields_.Value = Value

    _pack_   = 1
    _fields_ = [("Day",           Time_Day),
                ("DMY",           Time_DMY)]


# ---------------------------------------------------------------------------
# Direct calls into the IRIG 106 dll
# ---------------------------------------------------------------------------

def I106_Decode_TimeF1(header, msg_buffer):
    native_irig_time = time._ctIrig106Time()
    ret_status = packet.irig_data_dll.enI106_Decode_TimeF1(ctypes.byref(header), ctypes.byref(msg_buffer), ctypes.byref(native_irig_time))
    irig_time = time.IrigTime()
    irig_time.set_from_ctIrig106Time(native_irig_time)
    return ret_status, irig_time


#def I106_Decode_TimeF1_Buff(date_fmt, leap_year, msg_buffer, irig_time):
#    packet.irig_data_dll.enI106_Decode_TimeF1(date_fmt, leap_year, ctypes.byref(msg_buffer), ctypes.byref(irig_time))
#    return


#def I106_Encode_TimeF1(header, time_src, time_fmt, date_fmt, msg_buffer, native_irig_time):
#    ret_status = packet.irig_data_dll.enI106_Encode_TimeF1(ctypes.byref(header), time_src, time_fmt, date_fmt, ctypes.byref(irig_time), ctypes.byref(msg_buffer))
#    return ret_status


# ---------------------------------------------------------------------------
# Static methods
# ---------------------------------------------------------------------------



# ---------------------------------------------------------------------------
# Decode 1553 class
# ---------------------------------------------------------------------------

class DecodeTimeF1(object):
    """ Decode Time Format 1 packets """

    def __init__(self, packet_io):
        """ Constructor """
        self.packet_io  = packet_io
        self.irig_time = time.IrigTime()
     
    def decode_time_f1(self):
        ret_status, self.irig_time = I106_Decode_TimeF1(self.packet_io.header, self.packet_io.buffer)
        return ret_status

    def decode_time_f1_buff(self, date_fmt, leap_year, msg_buffer):
        I106_Decode_TimeF1(date_fmt, leap_year, msg_buffer, self.irig_time)
        return

        

# ---------------------------------------------------------------------------
# Module initialization
# ---------------------------------------------------------------------------


# This test code just opens an IRIG file and prints some time 
    
if __name__=='__main__':
    print ("IRIG 106 Decode Time")
    
#    import Time
    
    # Make IRIG 106 library classes
    pkt_io      = packet.IO()
    time_utils  = time.Time(pkt_io)
    time_decode = DecodeTimeF1(pkt_io)
    
    packet_count = 0
    
    if len(sys.argv) > 1 :
        open_status = pkt_io.open(sys.argv[1], packet.FileMode.READ)
        if open_status != status.OK :
            print ("Error opening data file %s" % (sys.argv[1]))
            sys.exit(1)
    else :
        print ("Usage : MsgDecodeTime.py <filename>")
        sys.exit(1)

    sync_status = time_utils.sync_time(False, 0)
    if sync_status != status.OK:
        print ("Sync Status = %s" % status.Message(sync_status))
        sys.exit(1)
    
    for pkt_hdr in pkt_io.packet_headers():
        if pkt_hdr.data_type == packet.DataType.IRIG_TIME:
            packet_count += 1
            pkt_io.read_data()
            status = time_decode.decode_time_f1()
            print (time_decode.irig_time)
            
    pkt_io.close()
    
    print ("Time Packets = %d" % packet_count)
    
