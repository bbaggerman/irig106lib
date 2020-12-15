import sys
import ctypes

import Py106.packet as packet
import Py106.status as status


# ---------------------------------------------------------------------------
# TMATS packet data structures
# ---------------------------------------------------------------------------

# TMATS packet channel specific data word

class TMATS_ChanSpec(ctypes.Structure):
    """ TMATS Channel Specific Data Word """
    _pack_   = 1
    _fields_ = [("Ch10Version",     ctypes.c_uint32,  8),
                ("ConfigChange",    ctypes.c_uint32,  1),
                ("Format",          ctypes.c_uint32,  1),
                ("Reserved",        ctypes.c_uint32, 22)]


# -----------------------------------------------------------------------------
# G Record structures
# -----------------------------------------------------------------------------

class TMATS_GDataSource(ctypes.Structure):
    """ TMATS G Data Source structure """
    _pack_   = 1


TMATS_GDataSource._fields_ = \
               [("Index",               ctypes.c_int),      # n
                ("DataSourceID",        ctypes.c_char_p),   # G\DSI-n
                ("DataSourceType",      ctypes.c_char_p),   # G\DST-n
                ("RRecord",             ctypes.c_void_p),
#               ("TRecord",             ctypes.c_void_p),
                ("Next",                ctypes.POINTER(TMATS_GDataSource))]


class TMATS_GRecord(ctypes.Structure):
    """ TMATS G Record structure """
    _pack_   = 1
    _fields_ = [("ProgramName",         ctypes.c_char_p),   # G\PN
                ("TestItem",            ctypes.c_char_p),   # G\TA
                ("FileName",            ctypes.c_char_p),   # G\FN
                ("Irig106Rev",          ctypes.c_char_p),   # G\106
                ("OriginationDate",     ctypes.c_char_p),   # G\OD
                ("RevisionNumber",      ctypes.c_char_p),   # G\RN
                ("RevisionDate",        ctypes.c_char_p),   # G\RD
                ("UpdateNumber",        ctypes.c_char_p),   # G\UN
                ("UpdateDate",          ctypes.c_char_p),   # G\UD
                ("TestNumber",          ctypes.c_char_p),   # G\TN
                ("NumOfContacts",       ctypes.c_char_p),   # G\POC\N
                ("FirstContact",        ctypes.c_void_p),
                ("NumDataSources",      ctypes.c_char_p),   # G\DSI\N
                ("FirstGDataSource",    ctypes.POINTER(TMATS_GDataSource)),
                ("TestDuration",        ctypes.c_char_p),   # G\TI1
                ("PreTestRequirement",  ctypes.c_char_p),   # G\TI2
                ("PostTestRequirement", ctypes.c_char_p),   # G\TI3
                ("Classification",      ctypes.c_char_p),   # G\SC
                ("Checksum",            ctypes.c_char_p),   # G\SHA
                ("FirstComment",        ctypes.c_char_p)]   # G\COM

# -----------------------------------------------------------------------------
# TMATS Lines structures
# -----------------------------------------------------------------------------

class TMATS_Lines(ctypes.Structure):
    """ TMATS Lines structure """
    _pack_ = 1
    _fields_ = [("CodeName", ctypes.c_char_p),
                ("DataItem", ctypes.c_char_p)]

# -----------------------------------------------------------------------------
# TMATS Comment structures
# -----------------------------------------------------------------------------

class TMATS_Comment(ctypes.Structure):
    """ TMATS Comment structure """
    _pack_ = 1


TMATS_Comment._fields_ = [("Comment", ctypes.c_char_p),
                          ("Next", ctypes.POINTER(TMATS_Comment))]

# -----------------------------------------------------------------------------
# TMATS Point of contact structures
# -----------------------------------------------------------------------------

class TMATS_PointOfContact(ctypes.Structure):
    """ TMATS Point of Contact structure """
    _pack_ = 1


TMATS_PointOfContact._fields_ = [("Index", ctypes.c_int),  # X\POC-n
                                 ("Name", ctypes.c_char_p),  # X\POC1-n
                                 ("Agency", ctypes.c_char_p),  # X\POC2-n
                                 ("Address", ctypes.c_char_p),  # X\POC3-n
                                 ("Telephone", ctypes.c_char_p),  # X\POC4-n
                                 ("Next", ctypes.POINTER(TMATS_PointOfContact))]

# -----------------------------------------------------------------------------
# TMATS info structures
# -----------------------------------------------------------------------------

class TMATS_Info(ctypes.Structure):
    """ TMATS information structure """
    _pack_   = 1
    _fields_ = [("TmatsLines",          ctypes.POINTER(TMATS_Lines)),
                ("NumberOfTmatsLines",  ctypes.c_ulong),
                ("AvailableTmatsLines", ctypes.c_ulong),
                ("Ch10Version",         ctypes.c_int),
                ("ConfigChange",        ctypes.c_int),
                ("FirstComment",        ctypes.POINTER(TMATS_Comment)),
                ("FirstGRecord",        ctypes.POINTER(TMATS_GRecord)),
                ("FirstRRecord",        ctypes.c_void_p),
                ("FirstMRecord",        ctypes.c_void_p),
                ("FirstBRecord",        ctypes.c_void_p),
                ("FirstPRecord",        ctypes.c_void_p),
                ("FirstTRecord",        ctypes.c_void_p),
                ("FirstDRecord",        ctypes.c_void_p),
                ("FirstSRecord",        ctypes.c_void_p),
                ("FirstARecord",        ctypes.c_void_p),
                ("FirstCRecord",        ctypes.c_void_p),
                ("FirstHRecord",        ctypes.c_void_p),
                ("FirstVRecord",        ctypes.c_void_p),
                ("FirstMemBlock",       ctypes.c_void_p)]

# ---------------------------------------------------------------------------
# Direct calls into the IRIG 106 dll
# ---------------------------------------------------------------------------

def I106_Decode_TMATS(header, msg_buffer, tmats_info):
    ret_status = packet.irig_data_dll.enI106_Decode_Tmats(ctypes.byref(header), ctypes.byref(msg_buffer), ctypes.byref(tmats_info))
    return ret_status

def I106_Decode_TMATS_Buff(tmats_buffer, tmats_info):
    ret_status = packet.irig_data_dll.enI106_Decode_Tmats_Text(ctypes.byref(tmats_buffer), tmats_buffer._length_, ctypes.byref(tmats_info))
    return ret_status

def I106_Tmats_Find(tmats_info, tmats_code):
    tmats_code_ascii = tmats_code.encode('ascii')
    packet.irig_data_dll.enI106_Tmats_Find.restype = ctypes.c_char_p
    tmats_value = packet.irig_data_dll.enI106_Tmats_Find(ctypes.byref(tmats_info), ctypes.c_char_p(tmats_code_ascii))
    if tmats_value is None:
        return ""
    else:
        return tmats_value.decode('ascii')

def I106_Free_TmatsInfo(tmats_info):
    packet.irig_data_dll.enI106_Free_TmatsInfo(ctypes.byref(tmats_info))
    return

# ---------------------------------------------------------------------------
# Static methods
# ---------------------------------------------------------------------------



# ---------------------------------------------------------------------------
# Decode 1553 class
# ---------------------------------------------------------------------------

class DecodeTMATS(object):
    """ Decode TMATS packets """

    def __init__(self, packet_io):
        """ Constructor """
        self.packet_io  = packet_io
        self.tmats_info = TMATS_Info()

    def decode_tmats(self):
        ret_status= I106_Decode_TMATS(self.packet_io.header, self.packet_io.buffer, self.tmats_info)
        return ret_status

    def decode_tmats_buff(self, msg_buffer):
        I106_Decode_TMATS_Buff(msg_buffer, self.tmats_info)
        return

    def find(self, tmats_code):
        tmats_value = I106_Tmats_Find(self.tmats_info, tmats_code)
#        if (tmats_value == b""):
        return tmats_value

    def free_tmatsinfo(self):
        I106_Free_TmatsInfo(self.tmats_info)
        return

    @property
    def ch10ver(self):
        """Ch10 (RCC) version label"""
        rccver = {0:  "106-05 or earlier",
                  7:  "106-07",
                  8:  "106-09",
                  9:  "106-11",
                  10: "106-13",
                  11: "106-15",
                  12: "106-17",
                  13: "106-19"}
        return rccver[self.tmats_info.Ch10Version]


# ---------------------------------------------------------------------------
# Module initialization
# ---------------------------------------------------------------------------


# This test code just opens an IRIG file and prints some time

if __name__=='__main__':

    print ("IRIG 106 Decode TMATS")

#    import Time

    # Make IRIG 106 library classes
    packet_io    = packet.IO()
    tmats_decode = DecodeTMATS(packet_io)

    if len(sys.argv) > 1 :
        open_status = packet_io.open(sys.argv[1], packet.FileMode.READ)
        if open_status != status.OK :
            print ("Error opening data file %s" % (sys.argv[1]))
            sys.exit(1)
    else :
        print ("Usage : MsgDecodeTMATS.py <filename>")
        sys.exit(1)

    for pkt_hdr in packet_io.packet_headers():
        if pkt_hdr.data_type == packet.DataType.TMATS:
            packet_io.read_data()
            status = tmats_decode.decode_tmats()
#            print (TmatsDecode.irig_time)
            break

    packet_io.close()

#    ProgramName = TmatsDecode.tmats_info.FirstGRecord.contents.ProgramName
#    print("Program Name : {0}".format(TmatsDecode.tmats_info.FirstGRecord.contents.ProgramName))
    program_name  = tmats_decode.find("G\\PN")
    irig_version  = tmats_decode.tmats_info.Ch10Version
    tmats_version = tmats_decode.find("G\\106")

    print("File Name     : {0}".format(sys.argv[1]))
    print("Program Name  : {0}".format(program_name))
    print("IRIG Version  : {0}".format(irig_version))
    print("TMATS Version : {0}".format(tmats_version))

    tmats_decode.free_tmatsinfo()
