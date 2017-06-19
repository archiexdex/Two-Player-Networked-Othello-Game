CC	= gcc
CXX = g++
CFLAGS	= -Wall -g -I/usr/include/ncurses
CXXFLAGS = -Wall -g -I/usr/include/ncurses
LDFLAGS	= -lncurses -pthread
PROGS	= sample

all: $(PROGS)

sample: sample.o othello.o
	$(CC) -o $@ $^ $(LDFLAGS)
	rm sample.o

%.o: %.c
	$(CC) -c $(CFLAGS) $<

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $<

clean:
	rm -f *.o *~ $(PROGS)

