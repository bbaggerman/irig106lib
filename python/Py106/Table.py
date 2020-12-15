# -*- coding: utf-8 -*-

import calendar
#import collections
import ctypes
import datetime
import os
import sys
#import time

#import Py106
import Py106.packet
import Py106.status
import Py106.time
import Py106.MsgDecode1553

import tables
import numpy


# ---------------------------------------------------------------------------
# Class to represent 1553 message index data type
# ---------------------------------------------------------------------------

class IndexMsg1553(tables.IsDescription):
    """ Class to hold index information 
        msg_index - Array index of message
        msg_time  - UTC time of indexed message
    """
    offset     = tables.UInt64Col()
    time       = tables.Time64Col()
    channel_id = tables.UInt16Col()
    rt         = tables.UInt8Col()
    tr         = tables.UInt8Col()
    subaddr    = tables.UInt8Col()
    
        

# ---------------------------------------------------------------------------
# Class to handle 1553 data types
# ---------------------------------------------------------------------------

class Msg1553(object):
    def __init__(self, ch10_h5_file = None):
        
        self.ch10_h5_file = ch10_h5_file
        if ch10_h5_file != None:
            self.layout_version = ch10_h5_file.root.Bus_Data.attrs.layout_version
        else:
            self.layout_version = 2
        
        # 1553 message data
        self.msg_time       = Py106.time.IrigTime()
        self.chan_id        = numpy.uint16(0)
        self.header_flags   = numpy.uint16(0)
        self.cmd_word_1     = numpy.uint16(0)
        self.stat_word_1    = numpy.uint16(0)
        self.cmd_word_2     = numpy.uint16(0)
        self.stat_word_2    = numpy.uint16(0)
        self.data           = []
    
# ---------------------------------------------------------------------------

    def encode_tuple(self):
        """ Take Msg1553 data and encode it into a tuple for storage in an H5 file """

        # Version 1 - Data is stored in H5 table as a pickled Python tuple
        if self.layout_version == 1:
            time_secs  = calendar.timegm(self.msg_time.time.timetuple())
            time_fracs = self.msg_time.time.microsecond
            time_fmt   = self.msg_time.dt_format
            flags_p    = ctypes.pointer(self.header_flags)
            flags_ip   = ctypes.cast(flags_p, ctypes.POINTER(ctypes.c_uint16))
            return                                \
                (time_secs,                       \
                 time_fracs,                      \
                 numpy.uint8(time_fmt),           \
                 numpy.uint16(self.chan_id),      \
                 numpy.uint16(flags_ip[0]),       \
                 numpy.uint16(self.cmd_word_1),   \
                 numpy.uint16(self.stat_word_1),  \
                 numpy.uint16(self.cmd_word_2),   \
                 numpy.uint16(self.stat_word_2),  \
                 self.data)

        # Version 2 - Data is stored as a variable length array of 16 bit integers
        elif self.layout_version == 2:
            time_secs  = calendar.timegm(self.msg_time.time.timetuple())
            time_fracs = self.msg_time.time.microsecond
            time_fmt   = self.msg_time.dt_format
            flags_p    = ctypes.pointer(self.header_flags)
            flags_ip   = ctypes.cast(flags_p, ctypes.POINTER(ctypes.c_uint16))

            # Make the list of 16 bit integers for data storage
            return_value = []
            return_value.append(numpy.uint16((time_secs>> 16) & 0x0000ffff))
            return_value.append(numpy.uint16((time_secs     ) & 0x0000ffff))
            return_value.append(numpy.uint16((time_fracs>>16) & 0x0000ffff))
            return_value.append(numpy.uint16((time_fracs    ) & 0x0000ffff))
            return_value.append(numpy.uint16(time_fmt))
            return_value.append(numpy.uint16(self.chan_id))
            return_value.append(numpy.uint16(flags_ip[0]))
            return_value.append(numpy.uint16(self.cmd_word_1))
            return_value.append(numpy.uint16(self.stat_word_1))
            return_value.append(numpy.uint16(self.cmd_word_2))
            return_value.append(numpy.uint16(self.stat_word_2))
            for data_word in self.data:
                return_value.append(numpy.uint16(data_word))
            
            return return_value
        
        else:
            return


# ---------------------------------------------------------------------------

    def decode_tuple(self, msg_tuple):
        """ Take a tuple from an H5 file and decode it into Msg1553 data """

        # Version 1 - Data is stored in H5 table as a pickled Python tuple
        if self.layout_version == 1:
            self.msg_time = Py106.time.IrigTime()
            self.msg_time.time = datetime.datetime.utcfromtimestamp(msg_tuple[0]) 
            self.msg_time.time = self.msg_time.time.replace(microsecond=msg_tuple[1])
            self.msg_time.dt_format = msg_tuple[2]

            self.chan_id     = msg_tuple[3]
            
            self.header_flags = Py106.MsgDecode1553.Hdr1553_Flags()
            flags_p          = ctypes.pointer(self.header_flags)
            flags_ip         = ctypes.cast(flags_p, ctypes.POINTER(ctypes.c_uint16))
            flags_ip[0]      = msg_tuple[4]

            self.cmd_word_1  = Py106.MsgDecode1553.CmdWord(msg_tuple[5])
            self.stat_word_1 = Py106.MsgDecode1553.StatWord(msg_tuple[6])
            self.cmd_word_2  = Py106.MsgDecode1553.CmdWord(msg_tuple[7])
            self.stat_word_2 = Py106.MsgDecode1553.StatWord(msg_tuple[8])
            self.data        = msg_tuple[9]

        elif self.layout_version == 2:
            self.msg_time = Py106.time.IrigTime()
            self.msg_time.time = datetime.datetime.utcfromtimestamp((msg_tuple[0]<<16)|msg_tuple[1]) 
            self.msg_time.time = self.msg_time.time.replace(microsecond=(msg_tuple[2]<<16)|msg_tuple[3])
            self.msg_time.dt_format = msg_tuple[4]

            self.chan_id     = msg_tuple[5]
            
            self.header_flags = Py106.MsgDecode1553.Hdr1553_Flags()
            flags_p          = ctypes.pointer(self.header_flags)
            flags_ip         = ctypes.cast(flags_p, ctypes.POINTER(ctypes.c_uint16))
            flags_ip[0]      = msg_tuple[6]

            self.cmd_word_1  = Py106.MsgDecode1553.CmdWord(msg_tuple[7])
            self.stat_word_1 = Py106.MsgDecode1553.StatWord(msg_tuple[8])
            self.cmd_word_2  = Py106.MsgDecode1553.CmdWord(msg_tuple[9])
            self.stat_word_2 = Py106.MsgDecode1553.StatWord(msg_tuple[10])
            self.data        = []
            for data_index in range(11, len(msg_tuple)):
                self.data.append(msg_tuple[data_index])

        else:
            return


# ---------------------------------------------------------------------------

    def find_msgs(self, channel_id, rt=-1, tr=-1, sa=-1, add_more=False):

        
        # Make the query string
        query = str.format("(channel_id == {0})", channel_id)
        if rt != -1:
            query += str.format(" & (rt == {0})", rt)
        if tr != -1:
            query += str.format(" & (tr == {0})", tr)
        if sa != -1:
            query += str.format(" & (subaddr == {0})", sa)

        row_offsets  = self.ch10_h5_file.root.Bus_Data_Index.readWhere(query, field="offset")
        if add_more == False:            
            self.row_offsets = row_offsets
        else:
            self.row_offsets = numpy.concatenate((self.row_offsets, row_offsets))
            
        # THIS IS OK FOR NOW BUT NEED TO SORT BY TIME
        self.row_offsets.sort()
        
# ---------------------------------------------------------------------------

    def msgs(self, index_low=0, index_high=-1):
        """ A generator that returns the specified 1553 messages one message at a time """

        if index_high == -1:
            index_high = len(self.row_offsets)
            
        # Return one 1553 message at a time
        for offset in self.row_offsets[index_low:index_high]:
            self.decode_tuple(self.ch10_h5_file.root.Bus_Data[offset])
            yield self
            
# ---------------------------------------------------------------------------

    def num_msgs(self):
        """Return the number of 1553 messages in the list"""
        return len(self.row_offsets)


# ---------------------------------------------------------------------------
# Module functions
# ---------------------------------------------------------------------------

def import_open(irig_filename, hdf5_filename="", force=False, status_callback=None):
    """ Open a Ch 10 file and read it into a PyTables table or open
        an existing (i.e. already converted) table.
        irig_filename   - Name of Ch 10 data file.
        hdf5_filename   - Name of resultant hdf5 file.  If blank then the hdf5 file name
            will be the same as the Ch 10 file but with a ".h5" extension.
        force           - If true force a rebuild of hdf5 file, even if it exists already.
        status_callback - User callback function that will be called periodically as conversion
            proceeds. Will be called with one parameter with a value from 0.0 to 1.0 indicating
            how far along the conversion has progressed into the Ch 10 data file.
        Return the PyTable table file object
    """

    # Make the HDF5 file name
    if hdf5_filename == "" :
        (hdf5_filename, ext) = os.path.splitext(irig_filename)
        hdf5_filename += ".h5"

    # Try opening an existing HDF5 file
    ch10_h5_file = None
    if force == False:
        try:
            ch10_h5_file = tables.open_file(hdf5_filename, mode = "r")
        except Exception as e:
            ch10_h5_file = None
        else:
            return ch10_h5_file

    # HDF5 file doesn't exists so make one
    if ch10_h5_file == None:
        ch10_h5_file = import_ch10(irig_filename, hdf5_filename, status_callback)
            
    return ch10_h5_file


# ---------------------------------------------------------------------------

def open_h5(hdf5_filename):
    """ Open an existing H5 file
        hdf5_filename   - Name of resultant hdf5 file.
        Return the PyTable table file object
    """

    # Try opening an existing HDF5 file
    try:
        ch10_h5_file = tables.open_file(hdf5_filename, mode = "r")
    except Exception as e:
        return None
    else:
        return ch10_h5_file


# ---------------------------------------------------------------------------

def import_ch10(irig_filename, hdf5_filename, status_callback=None):
    # Make IRIG 106 library classes
    pkt_io     = Py106.packet.IO()
    time_utils = Py106.time.Time(pkt_io)
    decode1553 = Py106.MsgDecode1553.Decode1553F1(pkt_io)

    # Initialize variables
    packet_count      = 0
    packet_count_1553 = 0
    msg_count_1553    = 0

    # Open the IRIG file
    ret_status = pkt_io.open(irig_filename, Py106.packet.FileMode.READ)
    if ret_status != Py106.status.OK :
        print("Error opening data file %s" % (irig_filename))
        sys.exit(1)

    # If using status callback then get file size
    if status_callback != None:
        file_size = os.stat(irig_filename).st_size
            
#        ret_status = time_utils.sync_time(False, 10)
#        if ret_status != Py106.status.OK:
#            print ("Sync Status = %s" % Py106.status.Message(ret_status))
#            sys.exit(1)

    # Set the default 1553 message table layout version
    layout_version = 2

    # Open the PyTable tables
    ch10_h5_file = tables.open_file(hdf5_filename, mode = "w", title = "Ch 10 Data File")

    # Create the 1553 message table
    if   layout_version == 1:
        ch10_bus_data = ch10_h5_file.create_vlarray("/", "Bus_Data", tables.ObjectAtom(), title="1553 Bus Data")
    elif layout_version == 2:
        ch10_bus_data = ch10_h5_file.create_vlarray("/", "Bus_Data", tables.UInt16Atom(), title="1553 Bus Data")
        
    ch10_bus_data.attrs.layout_version = layout_version

    # Create the 1553 message index table
    ch10_bus_data_index = ch10_h5_file.create_table("/", "Bus_Data_Index", IndexMsg1553, "1553 Bus Data Index")
    
    # Iterate over all the IRIG packets        
    for pkt_hdr in pkt_io.packet_headers():
        packet_count += 1
        
        # Update the callback function if it exists
        if status_callback != None:
            (status, offset) = pkt_io.get_pos()
            progress = float(offset) / float(file_size)
            status_callback(progress)

        if pkt_hdr.data_type == Py106.packet.DataType.IRIG_TIME:
            pkt_io.read_data()
            time_utils.set_rel_time()

        if pkt_hdr.data_type == Py106.packet.DataType.MIL1553_FMT_1:

            packet_count_1553 += 1
            pkt_io.read_data()
            for Msg in decode1553.msgs():
                msg_count_1553 += 1

                # Extract the import 1553 info
                WC = decode1553.word_cnt(Msg.pCmdWord1.contents.Value)

                # Put the 1553 message data into our storage class
                msg_1553              = Msg1553()
                msg_1553.msg_time     = time_utils.rel_int_to_irig_time(Msg.p1553Hdr.contents.Field.PktTime)
                msg_1553.chan_id      = numpy.uint16(pkt_io.header.ch_id)
                msg_1553.header_flags = Msg.p1553Hdr.contents.Field.Flags
                msg_1553.cmd_word_1   = numpy.uint16(Msg.pCmdWord1.contents.Value)
                msg_1553.stat_word_1  = numpy.uint16(Msg.pStatWord1.contents.Value)
                if (Msg.p1553Hdr.contents.Field.Flags.RT2RT == 0):
                    msg_1553.cmd_word_2  = numpy.uint16(0)
                    msg_1553.stat_word_2 = numpy.uint16(0)
                else:
                    msg_1553.cmd_word_2  = numpy.uint16(Msg.pCmdWord2.contents.Value)
                    msg_1553.stat_word_2 = numpy.uint16(Msg.pStatWord2.contents.Value)
                msg_1553.data = numpy.array(Msg.pData.contents[0:WC])
                msg_1553.layout_version = ch10_bus_data.attrs.layout_version
                DataMsg = msg_1553.encode_tuple()

                ch10_bus_data.append(DataMsg)

                # Store the 1553 command word index
                row_offset = ch10_bus_data.nrows - 1
                time_tuple_utc = msg_1553.msg_time.time.timetuple()
                timestamp_utc  = calendar.timegm(time_tuple_utc)
                timestamp_utc += msg_1553.msg_time.time.microsecond / 1000000.0
                
                new_row = ch10_bus_data_index.row
                new_row['offset']     = row_offset
                new_row['time']       = timestamp_utc
                new_row['channel_id'] = pkt_io.header.ch_id
                new_row['rt']         = Msg.pCmdWord1.contents.Field.RTAddr
                new_row['tr']         = Msg.pCmdWord1.contents.Field.TR
                new_row['subaddr']    = Msg.pCmdWord1.contents.Field.SubAddr
                new_row.append()
                
            # Done with 1553 messages in packet

    ch10_h5_file.flush()
    pkt_io.close()
    
    return ch10_h5_file
        
                
# ---------------------------------------------------------------------------

def close(ch10_h5_file):
    """ Close an open H5 file
        ch10_h5_file   - Open H5 file handle
    """

    if ch10_h5_file != None:
        ch10_h5_file = ch10_h5_file.close()


# ---------------------------------------------------------------------------
# Module initialization
# ---------------------------------------------------------------------------

#msg_tuple = collections.namedtuple('msg_tuple', "Time ChanID CmdWord1 StatWord1 CmdWord2 StatWord2 Data")

# This test code just opens an IRIG file and does a histogram of the 
# data types
    
if __name__=='__main__':
    print("IRIG 106 H5 Table")

    # Open an existing H5 file or convert an IRIG file    
    if len(sys.argv) > 1 :
        ch10_h5_file = import_open(sys.argv[1], force=False)
    else :
        print("Usage : Table.py <filename>")
        sys.exit(1)

    # Demonstrate using the opened table directly
    # -------------------------------------------
    # Use this programming technique when you want to random access to all 1553
    # messages in the table
    
    # Make an instance of the 1553 message class
    msg_read = Msg1553(ch10_h5_file)

    # The decoder must know the format version that the data was stored in
#    msg_read.layout_version = ch10_h5_file.root.Bus_Data.attrs.layout_version
#    print "1553 Table Message Version {0}".format(ch10_h5_file.root.Bus_Data.attrs.layout_version)
    
    # Get a slice of data out of the middle the h5 table
    start = ch10_h5_file.root.Bus_Data.nrows // 2
    stop  = start + 10
    msg_slice = ch10_h5_file.root.Bus_Data[start:stop]
    
    # Print out the slices of 1553 data
    for idx in range(10):
        msg_read.decode_tuple(msg_slice[idx])
        print("{0} Chan ID {1} {2}".format(msg_read.msg_time, msg_read.chan_id, msg_read.cmd_word_1),)
        for data_index in range(0, len(msg_read.data)):
            print("0x{0:04x}".format(msg_read.data[data_index]),)
        print("")


    # Demonstrate querying for a subset of messages and then interating through them
    # ------------------------------------------------------------------------------
    # Use this programming technique to get a subset of known messages and iterate
    # through them one at a time

    print("")
        
    # Make an instance of the 1553 message class
    msg_read = Msg1553(ch10_h5_file)

    # Query the open h5 file for specific messages
    msg_read.find_msgs(ch10_h5_file, 2112, 10, 1, 21)
    msg_read.find_msgs(ch10_h5_file, 2112, 31, 0, 5, add_more=True)

    for msg in msg_read.msgs(ch10_h5_file):
        print("{0} Chan ID {1} {2}".format(msg.msg_time, msg.chan_id, msg.cmd_word_1, ),)
        for data_index in range(0, len(msg.data)):
            print("{0:04x}".format(msg.data[data_index]),)
        print("")

    # Clean up
    msg_read = None
    