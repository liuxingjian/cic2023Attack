# ========================================
# Makefile for Distributed Network Attack Simulation System
# Based on CICIoT2023 Dataset
# ========================================

# Compiler and flags
CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -O2 -I./include
LDFLAGS = -lssh -pthread
CURL_FLAGS = -lcurl -lpthread

# Directories
SRC_DIR = src
MASTER_DIR = $(SRC_DIR)/master
ATTACK_DIR = $(SRC_DIR)/attacks
BUILD_DIR = build
BIN_DIR = bin

# Output binaries
MASTER_BIN = $(BIN_DIR)/master_controller
UDP_FLOOD_BIN = $(BIN_DIR)/udp_flood
HTTP_FLOOD_BIN = $(BIN_DIR)/http_flood
SQLI_BIN = $(BIN_DIR)/sqli_attacker
CMD_INJ_BIN = $(BIN_DIR)/command_injector
XSS_BIN = $(BIN_DIR)/xss_attacker
BACKDOOR_BIN = $(BIN_DIR)/backdoor_flood
BEEF_BIN = $(BIN_DIR)/beef_attack
UPLOAD_BIN = $(BIN_DIR)/uploading_attack

# All binaries
ALL_BINS = $(MASTER_BIN) $(UDP_FLOOD_BIN) $(HTTP_FLOOD_BIN) $(SQLI_BIN) \
           $(CMD_INJ_BIN) $(XSS_BIN) $(BACKDOOR_BIN) $(BEEF_BIN) $(UPLOAD_BIN)

# ========================================
# Main targets
# ========================================

.PHONY: all master attacks clean help dirs install

all: dirs master attacks

# Create necessary directories
dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BUILD_DIR)

# ========================================
# Master controller
# ========================================

master: dirs $(MASTER_BIN)

$(MASTER_BIN): $(MASTER_DIR)/master.cpp
	@echo "Building master controller..."
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
	@echo "✓ Master controller compiled successfully"

# ========================================
# Attack modules
# ========================================

attacks: dirs $(UDP_FLOOD_BIN) $(HTTP_FLOOD_BIN) $(SQLI_BIN) \
         $(CMD_INJ_BIN) $(XSS_BIN) $(BACKDOOR_BIN) $(BEEF_BIN) $(UPLOAD_BIN)

# UDP/TCP Flood Attack
$(UDP_FLOOD_BIN): $(ATTACK_DIR)/udp_flood.cpp
	@echo "Building UDP flood module..."
	$(CC) $(CFLAGS) $< -o $@ -pthread
	@echo "✓ UDP flood module compiled"

# HTTP Flood Attack
$(HTTP_FLOOD_BIN): $(ATTACK_DIR)/httpflood.cpp
	@echo "Building HTTP flood module..."
	$(CC) $(CFLAGS) $< -o $@ $(CURL_FLAGS)
	@echo "✓ HTTP flood module compiled"

# SQL Injection Attack
$(SQLI_BIN): $(ATTACK_DIR)/sqli_attacker.cpp
	@echo "Building SQL injection module..."
	$(CC) $(CFLAGS) $< -o $@ $(CURL_FLAGS)
	@echo "✓ SQL injection module compiled"

# Command Injection Attack
$(CMD_INJ_BIN): $(ATTACK_DIR)/command_attacker.cpp
	@echo "Building command injection module..."
	$(CC) $(CFLAGS) $< -o $@ $(CURL_FLAGS)
	@echo "✓ Command injection module compiled"

# XSS Attack
$(XSS_BIN): $(ATTACK_DIR)/xss_attacker.cpp
	@echo "Building XSS attack module..."
	$(CC) $(CFLAGS) $< -o $@ $(CURL_FLAGS)
	@echo "✓ XSS attack module compiled"

# Backdoor Flood Attack
$(BACKDOOR_BIN): $(ATTACK_DIR)/backdoor_flood.cpp
	@echo "Building backdoor flood module..."
	$(CC) $(CFLAGS) $< -o $@ -pthread
	@echo "✓ Backdoor flood module compiled"

# BeEF Attack
$(BEEF_BIN): $(ATTACK_DIR)/beef_attack.cpp
	@echo "Building BeEF attack module..."
	$(CC) $(CFLAGS) $< -o $@ $(CURL_FLAGS)
	@echo "✓ BeEF attack module compiled"

# File Upload Attack
$(UPLOAD_BIN): $(ATTACK_DIR)/uploading_attack.cpp
	@echo "Building file upload attack module..."
	$(CC) $(CFLAGS) $< -o $@ $(CURL_FLAGS)
	@echo "✓ File upload attack module compiled"

# ========================================
# Utility targets
# ========================================

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BIN_DIR)
	@rm -rf $(BUILD_DIR)
	@rm -f *.o
	@echo "✓ Clean completed"

# Clean and rebuild
rebuild: clean all

# Install to system (requires sudo)
install: all
	@echo "Installing binaries to /usr/local/bin..."
	@sudo cp $(ALL_BINS) /usr/local/bin/
	@echo "✓ Installation completed"

# Uninstall from system
uninstall:
	@echo "Removing installed binaries..."
	@sudo rm -f /usr/local/bin/master_controller
	@sudo rm -f /usr/local/bin/udp_flood
	@sudo rm -f /usr/local/bin/http_flood
	@sudo rm -f /usr/local/bin/sqli_attacker
	@sudo rm -f /usr/local/bin/command_injector
	@sudo rm -f /usr/local/bin/xss_attacker
	@sudo rm -f /usr/local/bin/backdoor_flood
	@sudo rm -f /usr/local/bin/beef_attack
	@sudo rm -f /usr/local/bin/uploading_attack
	@echo "✓ Uninstallation completed"

# Show help information
help:
	@echo "=========================================="
	@echo "Distributed Network Attack Simulation System"
	@echo "=========================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build all components (default)"
	@echo "  master    - Build master controller only"
	@echo "  attacks   - Build all attack modules"
	@echo "  clean     - Remove all build artifacts"
	@echo "  rebuild   - Clean and rebuild everything"
	@echo "  install   - Install binaries to /usr/local/bin (requires sudo)"
	@echo "  uninstall - Remove installed binaries (requires sudo)"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Individual attack modules:"
	@echo "  $(UDP_FLOOD_BIN)    - UDP/TCP flood"
	@echo "  $(HTTP_FLOOD_BIN)   - HTTP flood"
	@echo "  $(SQLI_BIN)         - SQL injection"
	@echo "  $(CMD_INJ_BIN)      - Command injection"
	@echo "  $(XSS_BIN)          - XSS attack"
	@echo "  $(BACKDOOR_BIN)     - Backdoor flood"
	@echo "  $(BEEF_BIN)         - BeEF attack"
	@echo "  $(UPLOAD_BIN)       - File upload attack"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              - Build everything"
	@echo "  make master       - Build master controller only"
	@echo "  make clean        - Clean all builds"
	@echo "  make help         - Show this message"
	@echo "=========================================="

# Check dependencies
check-deps:
	@echo "Checking required dependencies..."
	@command -v $(CC) >/dev/null 2>&1 || { echo "✗ g++ not found"; exit 1; }
	@echo "✓ g++ found"
	@pkg-config --exists libssh && echo "✓ libssh found" || echo "✗ libssh not found (install libssh-dev)"
	@pkg-config --exists libcurl && echo "✓ libcurl found" || echo "✗ libcurl not found (install libcurl4-openssl-dev)"
	@echo "Dependency check completed"
