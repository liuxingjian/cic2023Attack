# Makefile for CICIoT2023 DDoS Project

CC = g++
CFLAGS = -std=c++11 -I./include
LDFLAGS = -lssh -pthread

# Targets
all: master attacks

master: src/master/master.cpp
	$(CC) $(CFLAGS) src/master/master.cpp -o master_controller $(LDFLAGS)

attacks: src/attacks/udp_flood.cpp src/attacks/slowloris.cpp src/attacks/httpflood.cpp
	$(CC) $(CFLAGS) src/attacks/udp_flood.cpp -o udp_flood -pthread
	$(CC) $(CFLAGS) src/attacks/slowloris.cpp -o slowloris -pthread
	$(CC) $(CFLAGS) src/attacks/httpflood.cpp -o httpflood -pthread

clean:
	rm -f master_controller udp_flood slowloris httpflood
