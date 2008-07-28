# ---------------------------------------------------------------------------
# Makefile - A makefile for the IRIG 106 Chapter 10 library
# ---------------------------------------------------------------------------

CFLAGS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE

SRC_DIR=../src

# IRIG 106 Ch 10 Library
# ----------------------

libirig106.a: irig106cl.o irig106ch10.o i106_time.o i106_decode_time.o i106_decode_1553f1.o i106_decode_tmats.o i106_decode_uart.o i106_decode_video.o
	ar rc $@ $?

irig106cl.o: $(SRC_DIR)/irig106cl.cpp
	gcc $(CFLAGS) -c $(SRC_DIR)/irig106cl.cpp

irig106ch10.o: $(SRC_DIR)/irig106ch10.c $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/irig106ch10.c

i106_time.o: $(SRC_DIR)/i106_time.c $(SRC_DIR)/i106_time.h $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_time.c

i106_decode_time.o: $(SRC_DIR)/i106_decode_time.c $(SRC_DIR)/i106_decode_time.h $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_time.c

i106_decode_1553f1.o: $(SRC_DIR)/i106_decode_1553f1.c $(SRC_DIR)/i106_decode_1553f1.h $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_1553f1.c

i106_decode_tmats.o: $(SRC_DIR)/i106_decode_tmats.c $(SRC_DIR)/i106_decode_tmats.h $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats.c

i106_decode_uart.o: $(SRC_DIR)/i106_decode_uart.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_uart.c

i106_decode_video.o: $(SRC_DIR)/i106_decode_video.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_video.c
