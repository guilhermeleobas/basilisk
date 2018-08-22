
CC     = $(HOME)/Programs/llvm61/build/bin/clang
CFLAGS = -g -I$(IDIR) -disable-inlining

all:
	$(CC) collect.c -c -o collect.o $(CFLAGS)
	
clean:
	rm -f *.o *.exe
