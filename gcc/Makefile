

libirig106.a: irig106ch10.o i106_decode_time.o i106_decode_1553f1.o i106_decode_tmats.o
	ar rc $@ $?

irig106ch10.o: irig106ch10.c irig106ch10.h

i106_decode_time.o: i106_decode_time.c i106_decode_time.h irig106ch10.h

i106_decode_1553f1.o: i106_decode_1553f1.c i106_decode_1553f1.h irig106ch10.h

i106_decode_tmats.o: i106_decode_tmats.c i106_decode_tmats.h irig106ch10.h

i106_test_read.o: i106_test_read.c
