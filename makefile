CC = gcc
OBJSS = drone8.c
CFLAGS = -g -Wall
LIBS = -lm

all: drone8

drone8: $(OBJSS)
	$(CC) $(CFLAGS) -o $@ $(OBJSS) $(LIBS)

clean:
	rm -f drone8
