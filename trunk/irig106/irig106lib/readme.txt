IRIG106LIB
----------

Copyright (c) 2006 Irig106.org
Created by Bob Baggerman
bob.baggerman@gtri.gatech.edu

$RCSfile: readme.txt,v $
$Date: 2006-01-14 23:03:05 $
$Revision: 1.1 $

irig106lib is an open source library for reading and writing IRIG 106 Chapter 10 
format files.  The libary supports the Microsoft Visual C 6.0 compiler and 
compiles into a Win32 DLL, and the GNU GCC compiler under Linux and DJGPP and 
compiles into a static library.

irig106ch10 - The main source module containing routines for opening, reading, 
writing, and closing data files are contained in "irig106ch10.c".  Other 
software modules are provided to handle the various IRIG 106 Ch 10 message 
formats.

i106_decode_tmats - Decode a TMATS data message into a tree structure for
easy interpretation.

i106_decode_time - Decode IRIG time messages and provide routines for
converting relative time count values to IRIG referenced time.

i106_decode_1553f1 - Decode 1553 Format 1 messages.

