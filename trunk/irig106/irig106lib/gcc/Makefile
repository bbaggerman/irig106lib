# ---------------------------------------------------------------------------
# Makefile - A makefile for the IRIG 106 Chapter 10 library
#
# $RCSfile: Makefile,v $
# $Date: 2006-10-08 14:44:58 $
# $Revision: 1.2 $
# ---------------------------------------------------------------------------


# IRIG 106 Ch 10 Library
# ----------------------

libirig106.a: irig106ch10.o i106_time.o i106_decode_time.o i106_decode_1553f1.o i106_decode_tmats.o
	ar rc $@ $?

irig106ch10.o: irig106ch10.c irig106ch10.h

i106_time.o: i106_time.c i106_time.h irig106ch10.h

i106_decode_time.o: i106_decode_time.c i106_decode_time.h irig106ch10.h

i106_decode_1553f1.o: i106_decode_1553f1.c i106_decode_1553f1.h irig106ch10.h

i106_decode_tmats.o: i106_decode_tmats.c i106_decode_tmats.h irig106ch10.h

