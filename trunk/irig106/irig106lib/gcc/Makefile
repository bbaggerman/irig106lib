# ---------------------------------------------------------------------------
# Makefile - A makefile for the IRIG 106 Chapter 10 library
# ---------------------------------------------------------------------------

CFLAGS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -ggdb -fpack-struct=1

SRC_DIR=../src

# IRIG 106 Ch 10 Library
# ----------------------

libirig106.a: irig106ch10.o i106_time.o i106_index.o i106_decode_time.o i106_decode_1553f1.o i106_decode_tmats.o i106_decode_index.o i106_decode_uart.o i106_decode_video.o i106_decode_discrete.o i106_decode_ethernet.o i106_decode_arinc429.o i106_data_stream.o i106_decode_pcmf1.o i106_decode_analogf1.o i106_decode_can.o
	ar rc $@ $?

irig106ch10.o: $(SRC_DIR)/irig106ch10.c $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/irig106ch10.c

i106_time.o: $(SRC_DIR)/i106_time.c $(SRC_DIR)/i106_time.h $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_time.c

i106_index.o: $(SRC_DIR)/i106_index.c $(SRC_DIR)/i106_index.h $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_index.c

i106_decode_time.o: $(SRC_DIR)/i106_decode_time.c $(SRC_DIR)/i106_decode_time.h $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_time.c

i106_decode_index.o: $(SRC_DIR)/i106_decode_index.c $(SRC_DIR)/i106_decode_index.h $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_index.c

i106_decode_1553f1.o: $(SRC_DIR)/i106_decode_1553f1.c $(SRC_DIR)/i106_decode_1553f1.h $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_1553f1.c

i106_decode_tmats.o: $(SRC_DIR)/i106_decode_tmats.c $(SRC_DIR)/i106_decode_tmats.h $(SRC_DIR)/irig106ch10.h
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats.c

i106_decode_uart.o: $(SRC_DIR)/i106_decode_uart.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_uart.c

i106_decode_video.o: $(SRC_DIR)/i106_decode_video.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_video.c

i106_decode_discrete.o: $(SRC_DIR)/i106_decode_discrete.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_discrete.c

i106_decode_ethernet.o: $(SRC_DIR)/i106_decode_ethernet.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_ethernet.c

i106_decode_arinc429.o: $(SRC_DIR)/i106_decode_arinc429.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_arinc429.c

i106_data_stream.o: $(SRC_DIR)/i106_data_stream.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_data_stream.c

i106_decode_pcmf1.o: $(SRC_DIR)/i106_decode_pcmf1.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_pcmf1.c

i106_decode_analogf1.o: $(SRC_DIR)/i106_decode_analogf1.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_analogf1.c

i106_decode_can.o: $(SRC_DIR)/i106_decode_can.c
	gcc $(CFLAGS) -c $(SRC_DIR)/i106_decode_can.c

clean:
	rm *.o libirig106.a
