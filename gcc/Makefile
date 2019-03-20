# ---------------------------------------------------------------------------
# Makefile - A makefile for the IRIG 106 Chapter 10 library
# ---------------------------------------------------------------------------

ifeq ($(GCC),)
GCC := gcc
endif

CFLAGS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -ggdb -fpack-struct=1 -fPIC

SRC_DIR=../src

# IRIG 106 Ch 10 Library
# ----------------------

libirig106.so: irig106ch10.o i106_time.o i106_decode_time.o i106_decode_tmats.o i106_index.o i106_decode_index.o i106_decode_1553f1.o i106_decode_16pp194.o i106_decode_video.o i106_decode_ethernet.o i106_decode_arinc429.o i106_decode_tmats.o i106_decode_tmats_g.o i106_decode_tmats_p.o i106_decode_tmats_r.o
	$(GCC) $(CFLAGS) -shared -o $@ $?

libirig106.a: irig106ch10.o i106_time.o i106_decode_time.o i106_decode_tmats.o i106_index.o i106_decode_index.o i106_decode_1553f1.o i106_decode_16pp194.o i106_decode_video.o i106_decode_ethernet.o i106_decode_arinc429.o i106_decode_tmats.o i106_decode_tmats_g.o i106_decode_tmats_p.o i106_decode_tmats_r.o
	ar rc $@ $?

irig106ch10.o: $(SRC_DIR)/irig106ch10.c $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/irig106ch10.c

i106_time.o: $(SRC_DIR)/i106_time.c $(SRC_DIR)/i106_time.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_time.c

i106_index.o: $(SRC_DIR)/i106_index.c $(SRC_DIR)/i106_index.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_index.c

i106_decode_time.o: $(SRC_DIR)/i106_decode_time.c $(SRC_DIR)/i106_decode_time.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_time.c

i106_decode_index.o: $(SRC_DIR)/i106_decode_index.c $(SRC_DIR)/i106_decode_index.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_index.c

i106_decode_1553f1.o: $(SRC_DIR)/i106_decode_1553f1.c $(SRC_DIR)/i106_decode_1553f1.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_1553f1.c

i106_decode_tmats.o: $(SRC_DIR)/i106_decode_tmats.c $(SRC_DIR)/i106_decode_tmats.h $(SRC_DIR)/irig106ch10.h $(SRC_DIR)/i106_decode_tmats_g.o $(SRC_DIR)/i106_decode_tmats_p.o $(SRC_DIR)/i106_decode_tmats_r.o
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats.c

i106_decode_tmats_g.o: $(SRC_DIR)/i106_decode_tmats_g.c $(SRC_DIR)/i106_decode_tmats_g.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats_g.c

i106_decode_tmats_p.o: $(SRC_DIR)/i106_decode_tmats_p.c $(SRC_DIR)/i106_decode_tmats_p.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats_p.c

i106_decode_tmats_r.o: $(SRC_DIR)/i106_decode_tmats_r.c $(SRC_DIR)/i106_decode_tmats_r.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats_r.c

i106_decode_uart.o: $(SRC_DIR)/i106_decode_uart.c
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_uart.c

i106_decode_video.o: $(SRC_DIR)/i106_decode_video.c
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_video.c

i106_decode_discrete.o: $(SRC_DIR)/i106_decode_discrete.c
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_discrete.c

i106_decode_ethernet.o: $(SRC_DIR)/i106_decode_ethernet.c
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_ethernet.c

i106_decode_arinc429.o: $(SRC_DIR)/i106_decode_arinc429.c
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_arinc429.c

i106_decode_16pp194.o: $(SRC_DIR)/i106_decode_16pp194.c
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_16pp194.c

i106_data_stream.o: $(SRC_DIR)/i106_data_stream.c
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_data_stream.c

i106_decode_pcmf1.o: $(SRC_DIR)/i106_decode_pcmf1.c
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_pcmf1.c

i106_decode_analogf1.o: $(SRC_DIR)/i106_decode_analogf1.c
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_analogf1.c

i106_decode_can.o: $(SRC_DIR)/i106_decode_can.c
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_can.c

clean:
	rm --force $(SRC_DIR)/*.o $(SRC_DIR)/*.so $(SRC_DIR)/*.a
