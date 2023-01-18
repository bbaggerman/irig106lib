# ---------------------------------------------------------------------------
# Makefile - A makefile for the IRIG 106 Chapter 10 library
# ---------------------------------------------------------------------------

ifeq ($(GCC),)
GCC := gcc
endif

CFLAGS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -ggdb -fPIC -DIRIG_NETWORKING -DSHA256ENABLE -Wall -Wno-address-of-packed-member

SRC_DIR=../src

OBJS=irig106ch10.o i106_time.o i106_index.o i106_data_stream.o \
     i106_decode_tmats.o i106_decode_tmats_b.o i106_decode_tmats_c.o i106_decode_tmats_d.o i106_decode_tmats_g.o i106_decode_tmats_m.o i106_decode_tmats_p.o i106_decode_tmats_r.o sha-256.o \
     i106_decode_time.o i106_decode_index.o i106_decode_1553f1.o i106_decode_16pp194.o i106_decode_video.o i106_decode_ethernet.o i106_decode_arinc429.o \
     i106_decode_uart.o i106_decode_discrete.o i106_decode_can.o i106_decode_pcmf1.o 

# IRIG 106 Ch 10 Library
# ----------------------

.PHONY: all
all: libirig106.a libirig106.so

libirig106.so: $(OBJS)
	$(GCC) $(CFLAGS) -shared -o $@ $?

libirig106.a: $(OBJS)
	ar rc $@ $?

irig106ch10.o: $(SRC_DIR)/irig106ch10.c $(SRC_DIR)/irig106ch10.h \
               $(SRC_DIR)/i106_time.h   $(SRC_DIR)/i106_data_stream.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/irig106ch10.c

i106_time.o:   $(SRC_DIR)/i106_time.c   $(SRC_DIR)/i106_time.h \
               $(SRC_DIR)/irig106ch10.h $(SRC_DIR)/i106_decode_time.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_time.c

i106_index.o:  $(SRC_DIR)/i106_index.c  $(SRC_DIR)/i106_index.h        \
               $(SRC_DIR)/irig106ch10.h $(SRC_DIR)/i106_decode_tmats.h \
               $(SRC_DIR)/i106_decode_time.h $(SRC_DIR)/i106_decode_index.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_index.c

i106_decode_time.o: $(SRC_DIR)/i106_decode_time.c $(SRC_DIR)/i106_decode_time.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_time.c

i106_decode_index.o: $(SRC_DIR)/i106_decode_index.c $(SRC_DIR)/i106_decode_index.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_index.c

i106_decode_1553f1.o: $(SRC_DIR)/i106_decode_1553f1.c $(SRC_DIR)/i106_decode_1553f1.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_1553f1.c

i106_decode_tmats.o:   $(SRC_DIR)/i106_decode_tmats.c   $(SRC_DIR)/i106_decode_tmats.h        \
                       $(SRC_DIR)/i106_decode_tmats_b.h $(SRC_DIR)/i106_decode_tmats_c.h      \
                       $(SRC_DIR)/i106_decode_tmats_d.h $(SRC_DIR)/i106_decode_tmats_g.h      \
                       $(SRC_DIR)/i106_decode_tmats_m.h $(SRC_DIR)/i106_decode_tmats_p.h      \
                       $(SRC_DIR)/i106_decode_tmats_r.h                                       \
                       $(SRC_DIR)/irig106ch10.h         $(SRC_DIR)/i106_decode_tmats_common.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats.c

i106_decode_tmats_g.o: $(SRC_DIR)/i106_decode_tmats_g.c $(SRC_DIR)/i106_decode_tmats_g.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats_g.c

i106_decode_tmats_p.o: $(SRC_DIR)/i106_decode_tmats_p.c $(SRC_DIR)/i106_decode_tmats_p.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats_p.c

i106_decode_tmats_r.o: $(SRC_DIR)/i106_decode_tmats_r.c $(SRC_DIR)/i106_decode_tmats_r.h $(SRC_DIR)/irig106ch10.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats_r.c

i106_decode_tmats_b.o: $(SRC_DIR)/i106_decode_tmats_b.c $(SRC_DIR)/i106_decode_tmats_b.h      \
                       $(SRC_DIR)/irig106ch10.h         $(SRC_DIR)/i106_decode_tmats_common.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats_b.c

i106_decode_tmats_c.o: $(SRC_DIR)/i106_decode_tmats_c.c $(SRC_DIR)/i106_decode_tmats_c.h      \
                       $(SRC_DIR)/irig106ch10.h         $(SRC_DIR)/i106_decode_tmats_common.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats_c.c

i106_decode_tmats_d.o: $(SRC_DIR)/i106_decode_tmats_d.c $(SRC_DIR)/i106_decode_tmats_d.h      \
                       $(SRC_DIR)/i106_decode_tmats_c.h                                       \
                       $(SRC_DIR)/irig106ch10.h         $(SRC_DIR)/i106_decode_tmats_common.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats_d.c

i106_decode_tmats_m.o: $(SRC_DIR)/i106_decode_tmats_m.c $(SRC_DIR)/i106_decode_tmats_m.h      \
                       $(SRC_DIR)/irig106ch10.h         $(SRC_DIR)/i106_decode_tmats_common.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/i106_decode_tmats_m.c


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

sha-256.o: $(SRC_DIR)/sha-256.c $(SRC_DIR)/sha-256.h
	$(GCC) $(CFLAGS) -c $(SRC_DIR)/sha-256.c

clean:
	rm --force *.o *.so *.a
