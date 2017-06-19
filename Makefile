CC	= gcc
CXX = g++
CFLAGS	= -Wall -g -I/usr/include/ncurses
CXXFLAGS = -Wall -g -I/usr/include/ncurses
LDFLAGS	= -lncurses -pthread
PROGS	= othello

all: $(PROGS)
	rm *.o

othello: sample.o othello.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $<

clean:
	rm -f *.o *~ $(PROGS)

