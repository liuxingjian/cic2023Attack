# CICIoT2023 DDoS Simulation Project

This project aims to replicate the attack scenarios described in the CICIoT2023 dataset paper.

## Directory Structure

- `include/`: Header files (shared structures).
- `src/master/`: Master control logic (SSH-based command distribution).
- `src/attacks/`: Custom C++ attack implementations (e.g., `udp_flood`).
- `Makefile`: Build instructions.

## Prerequisites

- **Master Machine**: Windows/Linux with `libssh` and `g++`.
- **Attack Nodes**: Linux with `hping3`, `nmap`, `slowloris`, `fping`, `hydra`, `ettercap`.

## How to use

1. Compile the custom tools and distribute them to the attack nodes:
   ```bash
   g++ src/attacks/udp_flood.cpp -o udp_flood -pthread
   ```
2. Compile the master controller:
   ```bash
   g++ -I./include src/master/master.cpp -o master_controller -lssh -pthread
   ```
3. Run the controller and select the attack type.

## Attacks Covered (from CICIoT2023 Table 2)

- **DDoS/DoS**: hping3, udp-flood, slowloris.
- **Recon**: nmap, fping.
- **Brute Force**: hydra.
- **Spoofing**: ettercap.
