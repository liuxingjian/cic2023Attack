# Makefile for CICIoT2023 DDoS Project

CC = g++
CFLAGS = -std=c++11 -I./include
LDFLAGS = -lssh -pthread

# Targets
all: master attacks

master: src/master/master.cpp
	$(CC) $(CFLAGS) src/master/master.cpp -o master_controller $(LDFLAGS)

attacks: src/attacks/udp_flood.cpp src/attacks/httpflood.cpp src/attacks/sqli_attacker.cpp src/attacks/command_attacker.cpp
	$(CC) $(CFLAGS) src/attacks/udp_flood.cpp -o udp_flood -pthread
	$(CC) $(CFLAGS) src/attacks/httpflood.cpp -o httpflood -pthread
	$(CC) $(CFLAGS) src/attacks/sqli_attacker.cpp -o sqli_flood -lcurl -lpthread
	$(CC) $(CFLAGS) src/attacks/command_attacker.cpp -o cmd_flood -lcurl -lpthread

clean:
	rm -f master_controller udp_flood httpflood sqli_flood cmd_flood
