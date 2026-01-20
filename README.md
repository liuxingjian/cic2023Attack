# Distributed Network Attack Simulation & Security Validation System

A professional C++ distributed framework designed to simulate large-scale network attacks based on the **CICIoT2023** dataset. This system enables security researchers to generate realistic attack traffic for training and validating Intrusion Detection Systems (IDS).

## 🚀 Key Features

- **Distributed Master-Worker Architecture**: A centralized controller manages multiple distributed attack nodes via SSH, supporting synchronized large-scale simulation.
- **High-Performance Attack Engines**:
  - **Protocol Layer**: Custom multi-threaded engines for UDP/TCP/ICMP floods using raw sockets/standard socket API.
  - **Application Layer**: Integrated simulators for HTTP Flooding, SQL Injection (SQLi), and Command Injection using `libcurl`.
- **Extensive Attack Coverage**: Replicates 30+ attack types defined in the CICIoT2023 paper, including DDoS, DoS, Reconnaissance, Brute Force, and Mirai-style floods.
- **Real-time Monitoring**: Master node provides real-time feedback and output collection from all worker nodes.

## 🛠 Tech Stack

- **Language**: C++11
- **Networking**: Socket API, TCP/UDP, HTTP
- **Libraries**: 
  - `libssh`: For remote node orchestration and command execution.
  - `libcurl`: For simulating web-based application layer attacks.
  - `pthread`: For high-concurrency packet generation.
- **Environment**: Linux (Ubuntu recommended)

## 📂 Project Structure

```text
├── include/
│   └── attack_node.h      # Data structures for nodes and attack types
├── src/
│   ├── master/
│   │   └── master.cpp     # Central controller logic (SSH-based)
│   └── attacks/
│       ├── udp_flood.cpp  # High-concurrency UDP/TCP flood engine
│       ├── httpflood.cpp  # Multi-threaded HTTP flood tool
│       ├── sqli_attacker.cpp # Web-based SQL Injection simulator
│       └── command_attacker.cpp # Command Injection simulation module
└── Makefile               # Automated build system
```

## ⚙️ Installation & Usage

### Prerequisites
- Install dependencies on the Master node:
  ```bash
  sudo apt-get install libssh-dev libcurl4-openssl-dev g++ make
  ```

### Compilation
Build the system using the provided Makefile or manual commands:

1. **Build Master & Attack Tools**:
   ```bash
   # Build the main controller
   g++ -I./include src/master/master.cpp -o master_controller -lssh -pthread

   # Build custom attack modules
   g++ src/attacks/udp_flood.cpp -o udp_flood -pthread
   g++ src/attacks/sqli_attacker.cpp -o sqli_flood -lcurl -lpthread
   ```

### Execution
1. Ensure all worker nodes are accessible via SSH.
2. Configure worker IPs and credentials in `src/master/master.cpp`.
3. Launch the controller:
   ```bash
   ./master_controller
   ```
4. Select the attack category and type from the interactive menu.

## ⚠️ Disclaimer

This project is intended for **educational and research purposes only**. It should only be used in controlled laboratory environments or against systems you have explicit permission to test. Unauthorized access or disruption of computer systems is illegal.

## 📄 License
This project is licensed under the MIT License.
