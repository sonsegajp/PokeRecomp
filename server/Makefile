CC = gcc
CFLAGS = -O2 -Wall
LDFLAGS = -lws2_32

ifeq ($(OS),Windows_NT)
	TARGET = PokeRecomp_Server.exe
else
	TARGET = PokeRecomp_Server
	LDFLAGS =
endif

$(TARGET): server.c protocol.h
	$(CC) $(CFLAGS) -o $@ server.c $(LDFLAGS)

clean:
	rm -f $(TARGET)
