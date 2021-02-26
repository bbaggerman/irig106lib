import sys
import ctypes
import Py106.status as status
import Py106.packet as packet
from Py106.packet import irig_data_dll as IrigLib


class CsdwVideoF0(ctypes.Structure):
    """Channel specific data word for Video Format 0 packets"""
    _pack_ = 1
    _fields_ = [('Reserved', ctypes.c_uint32, 23),
                ('ByteAlign', ctypes.c_uint32, 1),
                ('PayloadType', ctypes.c_uint32, 4),
                ('KLV', ctypes.c_uint32, 1),
                ('SRS', ctypes.c_uint32, 1),
                ('IPHeader', ctypes.c_uint32, 1),
                ('EmbedTime', ctypes.c_uint32, 1)]


class IphVideoF0(ctypes.Structure):
    """Video Format 0 intra-packet header"""
    _pack_ = 1
    _fields_ = [('IntPktTime', ctypes.c_uint8 * 8)]


class MsgVideoF0(ctypes.Structure):
    """Video Format 0 message"""
    _pack_ = 1
    _fields_ = [('_CSDW', ctypes.POINTER(CsdwVideoF0)),
                ('_IPHeader', ctypes.POINTER(IphVideoF0)),
                ('_TSData', ctypes.POINTER(ctypes.c_uint8 * 188))]

    @property
    def CSDW(self):
        """Channel specific data word object"""
        return self._CSDW.contents

    @property
    def IPHeader(self):
        """Intra-packet header object"""
        return self._IPHeader.contents

    @property
    def byteorder(self):
        """Byte order of video transport stream data words"""
        if self._CSDW.contents.ByteAlign:
            return 'big'
        else:
            return 'little'

    def TSData(self, as_bytes=False):
        """Video transport data byte stream

        Return stream data as a bytes object when ``as_bytes=True``, otherwise
        as a list.
        """
        if as_bytes:
            stream = bytearray(self._TSData.contents)
            if self.byteorder == 'little':
                # Swap word bytes from little-endian to big-endian...
                stream[0::2], stream[1::2] = stream[1::2], stream[0::2]
            return bytes(stream)
        else:
            return self._TSData.contents


class DecodeVideoF0:
    """Decode Video Format 0 packets"""

    def __init__(self, pcktio):
        self._pio = pcktio
        self._currmsg = MsgVideoF0()

    def decode_first(self):
        status = IrigLib.enI106_Decode_FirstVideoF0(
            ctypes.byref(self._pio.Header), ctypes.byref(self._pio.Buffer),
            ctypes.byref(self._currmsg))
        if status != status.OK:
            raise RuntimeError(status.Message(status))
        return self._currmsg

    def decode_next(self):
        status = IrigLib.enI106_Decode_NextVideoF0(
            ctypes.byref(self._pio.Header), ctypes.byref(self._currmsg))
        if status == status.OK:
            return self._currmsg
        elif status == status.NO_MORE_DATA:
            return None
        raise RuntimeError(status.Message(status))

    def msgs(self):
        """Iterator over messages in one Video Format 0 packet"""
        yield self.decode_first()
        while self.decode_next():
            yield self._currmsg


def main():
    """Print out Video Format 0 data stream from a Ch10 file"""
    try:
        fname = sys.argv[1]
    except IndexError:
        print('Usage: {} <ch10 filename>'.format(sys.argv[0]))
        sys.exit(1)

    pkt = packet.IO()
    print('Input Ch10 file: {}'.format(fname))
    status = pkt.open(fname, packet.FileMode.READ)
    if status != status.OK:
        raise OSError('Error opening {}'.format(fname))

    vid = DecodeVideoF0(pkt)
    packet_count = 0
    tot_msgs = 0
    for p in pkt.packet_headers():
        packet_count += 1
        if p.DataType == packet.DataType.VIDEO_FMT_0:
            pkt.read_data()
            msg_count = 0
            for msg in vid.msgs():
                msg_count += 1
                print('Ch={:d}:packet=#{:d}:Msg=#{:d}: '
                      .format(pkt.Header.ch_id, packet_count, msg_count), end='')
                print('{}'.format(
                    ' '.join('0x{:x}'.format(b)
                             for b in msg.TSData(as_bytes=True))))
            tot_msgs += msg_count
    pkt.close()
    print('Total of {:d} messages in {:d} Video Format 0 packets'
          .format(tot_msgs, packet_count))


if __name__ == '__main__':
    main()
