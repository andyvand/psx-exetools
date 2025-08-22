CC = gcc
CXX = g++
CFLAGS = -Os -g0
CXXFLAGS = -Os -g0
LDFLAGS = -g0

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

elf2cpe: elf2cpe.o
	$(CC) $(LDFLAGS) -o $@ $<

cpe2psx: cpe2psx.o
	$(CXX) $(LDFLAGS) -o $@ $<

all: elf2cpe cpe2psx

clean:
	rm -f *.o elf2cpe cpe2psx

