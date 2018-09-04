import sys
import ctypes

import Py106.Packet as Packet
import Py106.Status as Status


# ---------------------------------------------------------------------------
# TMATS packet data structures
# ---------------------------------------------------------------------------

# TMATS packet channel specific data word

class TMATS_ChanSpec(ctypes.Structure):
    ''' TMATS Channel Specific Data Word '''
    _pack_   = 1
    _fields_ = [("Ch10Version",     ctypes.c_uint32,  8),
                ("ConfigChange",    ctypes.c_uint32,  1),
                ("Format",          ctypes.c_uint32,  1),
                ("Reserved",        ctypes.c_uint32, 22)]


# -----------------------------------------------------------------------------
# G Record structures
# -----------------------------------------------------------------------------

class TMATS_GDataSource(ctypes.Structure):
    ''' TMATS G Data Source structure '''
    _pack_   = 1

TMATS_GDataSource._fields_ = \
               [("Index",               ctypes.c_int),      # n
                ("DataSourceID",        ctypes.c_char_p),   # G\DSI-n
                ("DataSourceType",      ctypes.c_char_p),   # G\DST-n
                ("RRecord",             ctypes.c_void_p),
#               ("TRecord",             ctypes.c_void_p),
                ("Next",                ctypes.POINTER(TMATS_GDataSource))]

class TMATS_GRecord(ctypes.Structure):
    ''' TMATS G Record structure '''
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
# TMATS info structures
# -----------------------------------------------------------------------------

class TMATS_Info(ctypes.Structure):
    ''' TMATS information structure '''
    _pack_   = 1
    _fields_ = [("TmatsLines",          ctypes.c_void_p),
                ("NumberOfTmatsLines",  ctypes.c_ulong),
                ("AvailableTmatsLines", ctypes.c_ulong),
                ("Ch10Version",         ctypes.c_int),
                ("ConfigChange",        ctypes.c_int),
                ("FirstComment",        ctypes.c_void_p),
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
    ret_status = Packet.IrigDataDll.enI106_Decode_Tmats(ctypes.byref(header), ctypes.byref(msg_buffer), ctypes.byref(tmats_info))
    return ret_status

def I106_Decode_TMATS_Buff(tmats_buffer, tmats_info):
    ret_status = Packet.IrigDataDll.enI106_Decode_Tmats_Text(ctypes.byref(tmats_buffer), tmats_buffer._length_, ctypes.byref(tmats_info))
    return ret_status

def I106_Tmats_Find(tmats_info, tmats_code):
    tmats_code_ascii = tmats_code.encode('ascii')
    Packet.IrigDataDll.enI106_Tmats_Find.restype = ctypes.c_char_p
    tmats_value = Packet.IrigDataDll.enI106_Tmats_Find(ctypes.byref(tmats_info), ctypes.c_char_p(tmats_code_ascii))
    if tmats_value is None:
        return ""
    else:
        return tmats_value.decode()

def I106_Free_TmatsInfo(tmats_info):
    Packet.IrigDataDll.enI106_Free_TmatsInfo(ctypes.byref(tmats_info))
    return

# ---------------------------------------------------------------------------
# Static methods
# ---------------------------------------------------------------------------



# ---------------------------------------------------------------------------
# Decode 1553 class
# ---------------------------------------------------------------------------

class DecodeTMATS(object):
    ''' Decode TMATS packets '''

    def __init__(self, PacketIO):
        ''' Constructor '''
        self.PacketIO  = PacketIO
        self.TmatsInfo = TMATS_Info()
        
    def decode_tmats(self):
        ret_status= I106_Decode_TMATS(self.PacketIO.Header, self.PacketIO.Buffer, self.TmatsInfo)
        return ret_status

    def decode_tmats_buff(self, msg_buffer):
        I106_Decode_TMATS_Buff(msg_buffer, self.TmatsInfo)
        return

    def find(self, tmats_code):
        tmats_value = I106_Tmats_Find(self.TmatsInfo, tmats_code)
#        if (tmats_value == b""):
        return tmats_value

    def free_tmatsinfo(self):
        I106_Free_TmatsInfo(self.TmatsInfo)
        return
        

# ---------------------------------------------------------------------------
# Module initialization
# ---------------------------------------------------------------------------


# This test code just opens an IRIG file and prints some time 
    
if __name__=='__main__':
    
    print ("IRIG 106 Decode TMATS")
    
#    import Time
    
    # Make IRIG 106 library classes
    PktIO       = Packet.IO()
    TmatsDecode = DecodeTMATS(PktIO)
    DataType    = Packet.DataType()
    
    if len(sys.argv) > 1 :
        RetStatus = PktIO.open(sys.argv[1], Packet.FileMode.READ)
        if RetStatus != Status.OK :
            print ("Error opening data file %s" % (sys.argv[1]))
            sys.exit(1)
    else :
        print ("Usage : MsgDecodeTMATS.py <filename>")
        sys.exit(1)

    for PktHdr in PktIO.packet_headers():
        if PktHdr.DataType == Packet.DataType.TMATS:
            PktIO.read_data()
            status = TmatsDecode.decode_tmats()
#            print (TmatsDecode.irig_time)
            break
            
    PktIO.close()
    
#    ProgramName = TmatsDecode.TmatsInfo.FirstGRecord.contents.ProgramName    
#    print("Program Name : {0}".format(TmatsDecode.TmatsInfo.FirstGRecord.contents.ProgramName))
    ProgramName  = TmatsDecode.find("G\\PN")
    IrigVersion  = TmatsDecode.TmatsInfo.Ch10Version
    TmatsVersion = TmatsDecode.find("G\\106")
    
    print("File Name     : {0}".format(sys.argv[1]))
    print("Program Name  : {0}".format(ProgramName.decode()))
    print("IRIG Version  : {0}".format(IrigVersion))
    print("TMATS Version : {0}".format(TmatsVersion.decode()))
    
    TmatsDecode.free_tmatsinfo()
    