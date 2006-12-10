----------
IRIG106LIB
----------

Copyright (c) 2006 Irig106.org
Created by Bob Baggerman
bob.baggerman@gtri.gatech.edu

$RCSfile: readme.txt,v $
$Date: 2006-12-10 15:18:26 $
$Revision: 1.4 $



irig106lib is an open source library for reading and writing IRIG 106 Chapter 10 
format files.  The libary supports the Microsoft Visual C 6.0 compiler and 
compiles into a Win32 DLL, and the GNU GCC compiler under Linux and DJGPP and 
compiles into a static library.


Using the library
-----------------

Compile up using your favorite compiler suite. For MSVC 6 use the "irig106.dsw"
work space.  For MSVC .NET use the "irig106.sln" solution file.  For GCC (on Linux
or DJGPP) use "make".

Reading files involves opening the file, reading a data packet header, 
optionally read the data packet (which may contain multiple data messages), 
decode the data packet, and then loop back and read the next header.  The 
routines for handling data packets are in "irig106ch10".  Routines for decoding 
each data packet type are contained in their own source code modules.  For 
example, 1553 decoding is contained in "i106_decode_1553f1".  Below is a 
simplified example of message processing...

    enI106Ch10Open(&iI106Ch10Handle, szInFile, I106_READ);

    while (1==1) 
        {

        enStatus = enI106Ch10ReadNextHeader(iI106Ch10Handle, &suI106Hdr);

        if (enStatus == I106_EOF) return;

        enStatus = enI106Ch10ReadData(iI106Ch10Handle, &ulBuffSize, pvBuff);

        switch (suI106Hdr.ubyDataType)
            {

            case I106CH10_DTYPE_1553_FMT_1 :    // 0x19

                enStatus = enI106_Decode_First1553F1(&suI106Hdr, pvBuff, &su1553Msg);
                while (enStatus == I106_OK)
                    {
                    Do some processing...
                    enStatus = enI106_Decode_Next1553F1(&su1553Msg);
                    }
                break;
            default:
                break;
            } // end switch on message type

        }  // End while

    enI106Ch10Close(iI106Ch10Handle);


Modules
-------

irig106ch10 - The main source module containing routines for opening, reading, 
writing, and closing data files are contained in "irig106ch10.c".  Other 
software modules are provided to handle the various IRIG 106 Ch 10 message 
formats.

i106_time - Routines to convert between clock time and IRIG 106 time counts

i106_decode_tmats - Decode a TMATS data message into a tree structure for
easy interpretation.

i106_decode_time - Decode IRIG time messages and provide routines for
converting relative time count values to IRIG referenced time.

i106_decode_1553f1 - Decode 1553 Format 1 messages.


ToDo
----

Implement support for index records.

Implement seek() based on time.

Implement video decoder

Parse more TMATS fields

Provide better, more automatic ways to keep time in sync
