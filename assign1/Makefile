#
all: pass_init pass_mp pass_tbb pass_cilk

default: pass_init pass_mp pass_tbb pass_cilk

pass_mp:

pass_tbb:

pass_cilk:

pass_init: pass_init.c
	gcc -std=c99 -o pass_init pass_init.c -lcrypto -fopenmp

clean:
	rm *.o pass_init
