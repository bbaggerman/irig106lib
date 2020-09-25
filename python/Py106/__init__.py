
"""
Py106 - This python package provides an interface to the IRIG 106 data DLL.

Message data structures are based on the ctypes module.  The main implication of
this is that to use message data, first the data buffer needs to be cast to the appropriate
data structure. 

Example 1

    # Import the irig106 package
    import Py106
    
    # Create IRIG IO object
    IrigIO  = Py106.packet.IO()
    
    # Open data file for reading
    IrigIO.open("data.ch10", IrigIO.FileMode.READ)
    
    # Read IRIG headers
    for pkt_hdr in IrigIO.packet_headers():
        print "Ch ID %3i  %s" % (IrigIO.Header.ch_id, Py106.packet.DataType.TypeName(pkt_hdr.DataType))
                
    IrigIO.close()


Basic packet I/O is handled in the "Packet" module. Return status values are
defined in the "status" module. Both of these modules are loaded automatically
when the Py106 package is loaded.  Other modules must be loaded by user code if
they are to be used.  For example, 1553 decoding is supported by including the line

    import Py106.MsgDecode1553

Py106.Packet - Basic packet reading and writing

Py106.status - Return status values for all modules

Py106.MsgDecode1553 - Extract 1553 messages from a 1553 packet




"""


try:
    import Py106.packet as packet
    import Py106.status as status
#    import Py106.Time
    
except ImportError:
    print ("Py106 init error - Error importing modules")

