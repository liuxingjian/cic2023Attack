# Distributed Network Attack Simulation & Security Validation System
# 分布式网络攻击模拟与安全验证系统

[English](#english) | [中文](#chinese)

---

<a name="english"></a>
## English Version

### 📖 Overview

A professional C++ distributed framework designed to simulate large-scale network attacks based on the **CICIoT2023** dataset. This system enables security researchers to generate realistic attack traffic for training and validating Intrusion Detection Systems (IDS).

### 🚀 Key Features

- **Distributed Master-Worker Architecture**: A centralized controller manages multiple distributed attack nodes via SSH, supporting synchronized large-scale simulation.
- **High-Performance Attack Engines**:
  - **Protocol Layer**: Custom multi-threaded engines for UDP/TCP/ICMP floods using raw sockets/standard socket API.
  - **Application Layer**: Integrated simulators for HTTP Flooding, SQL Injection (SQLi), Command Injection, and XSS attacks using `libcurl`.
- **Extensive Attack Coverage**: Replicates 30+ attack types defined in the CICIoT2023 paper, including DDoS, DoS, Reconnaissance, Brute Force, and Mirai-style floods.
- **Real-time Monitoring**: Master node provides real-time feedback and output collection from all worker nodes.

### 🛠 Tech Stack

- **Language**: C++11
- **Networking**: Socket API, TCP/UDP, HTTP
- **Libraries**: 
  - `libssh`: For remote node orchestration and command execution.
  - `libcurl`: For simulating web-based application layer attacks.
  - `pthread`: For high-concurrency packet generation.
- **Environment**: Linux (Ubuntu recommended)

### 📂 Project Structure

```text
├── include/
│   └── attack_node.h              # Data structures for nodes and attack types
├── src/
│   ├── master/
│   │   └── master.cpp             # Central controller logic (SSH-based)
│   └── attacks/
│       ├── udp_flood.cpp          # High-concurrency UDP/TCP flood engine
│       ├── httpflood.cpp          # Multi-threaded HTTP flood tool
│       ├── sqli_attacker.cpp      # Web-based SQL Injection simulator
│       ├── command_attacker.cpp   # Command Injection simulation module
│       ├── xss_attacker.cpp       # XSS attack simulator
│       ├── backdoor_flood.cpp     # Backdoor attack module
│       ├── beef_attack.cpp        # BeEF integration module
│       └── uploading_attack.cpp   # File upload vulnerability simulator
├── scripts/                       # Auxiliary scripts
└── Makefile                       # Automated build system
```

### ⚙️ Installation & Usage

#### Prerequisites
Install dependencies on the Master node:
```bash
sudo apt-get install libssh-dev libcurl4-openssl-dev g++ make
```

#### Compilation
Build the system using the provided Makefile or manual commands:

1. **Build Master & Attack Tools**:
   ```bash
   # Build the main controller
   g++ -I./include src/master/master.cpp -o master_controller -lssh -pthread

   # Build custom attack modules
   g++ src/attacks/udp_flood.cpp -o udp_flood -pthread
   g++ src/attacks/httpflood.cpp -o http_flood -lcurl -pthread
   g++ src/attacks/sqli_attacker.cpp -o sqli_flood -lcurl -lpthread
   ```

#### Execution
1. Ensure all worker nodes are accessible via SSH.
2. Configure worker IPs and credentials in the master controller.
3. Launch the controller:
   ```bash
   ./master_controller
   ```
4. Select the attack category and type from the interactive menu.

### 🔧 Tools & References

This project integrates and references multiple security testing tools:

49. **hping3** - Network packet crafting tool  
    Tools, K. hping3 Package Description. 2019.  
    Available: https://www.kali.org/tools/hping3 (accessed on 19 June 2023)

50. **UDPFlood** - UDP flooding tool  
    EPC-MSU. UDPFlood. 2023.  
    Available: https://github.com/EPC-MSU/udp-flood (accessed on 19 June 2023)

51. **Slowloris** - Low-bandwidth HTTP DoS attack tool  
    Yaltirakli, G. Slowloris. 2015.  
    Available: https://github.com/gkbrk/slowloris (accessed on 19 June 2023)

52. **Golang-HTTPFlood** - HTTP flooding tool written in Go  
    Golang-HTTPFlood. 2020.  
    Available: https://github.com/Leeon123/golang-httpflood (accessed on 19 June 2023)

53. **Nmap** - Network security scanner  
    Lyon, G. Nmap Security Scanner. 2014.  
    Available: http://nmap.org/ (accessed on 22 June 2023)

54. **Fping** - ICMP ping utility  
    Tools, K. Fping. 2023.  
    Available: https://fping.org/ (accessed on 19 June 2023)

55. **Vulscan** - Vulnerability scanning module for Nmap  
    SCIP. Vulscan. 2023.  
    Available: https://github.com/scipag/vulscan (accessed on 19 June 2023)

56. **DVWA** - Damn Vulnerable Web Application  
    DVWA. Damn Vulnerable Web Application. 2017.  
    Available: https://github.com/digininja/DVWA (accessed on 19 June 2023)

57. **Remot3d** - Remote exploitation framework  
    KeepWannabe. Remot3d—An Easy Way To Exploiting. 2020.  
    Available: https://github.com/KeepWannabe/Remot3d (accessed on 19 June 2023)

58. **BeEF** - The Browser Exploitation Framework  
    BEEF. The Browser Exploitation Framework. 2023.  
    Available: https://beefproject.com (accessed on 19 June 2023)

59. **Hydra** - Network authentication cracker  
    Maciejak, D. Hydra. 2023.  
    Available: https://github.com/vanhauser-thc/thc-hydra (accessed on 19 June 2023)

60. **Ettercap** - Network sniffer/interceptor  
    Ornaghi, A.; Valleri, M. Ettercap. 2005.  
    Available: https://www.ettercap-project.org/ (accessed on 19 June 2023)

61. **Mirai Botnet** - IoT botnet source code  
    Gamblin, J. Mirai BotNet. 2017.  
    Available: https://github.com/jgamblin/Mirai-Source-Code (accessed on 19 June 2023)

### ⚠️ Disclaimer

This project is intended for **educational and research purposes only**. It should only be used in controlled laboratory environments or against systems you have explicit permission to test. Unauthorized access or disruption of computer systems is illegal and punishable by law.

### 📄 License
This project is licensed under the MIT License.

---

<a name="chinese"></a>
## 中文版本

### 📖 项目概述

这是一个基于 **CICIoT2023** 数据集设计的专业级 C++ 分布式框架，用于模拟大规模网络攻击。该系统能够帮助安全研究人员生成真实的攻击流量，用于训练和验证入侵检测系统（IDS）。

### 🚀 核心特性

- **分布式主从架构**：中心控制器通过 SSH 管理多个分布式攻击节点，支持同步的大规模模拟。
- **高性能攻击引擎**：
  - **协议层**：使用原始套接字/标准套接字 API 构建的自定义多线程 UDP/TCP/ICMP 洪泛引擎。
  - **应用层**：使用 `libcurl` 集成的 HTTP 洪泛、SQL 注入（SQLi）、命令注入和 XSS 攻击模拟器。
- **广泛的攻击覆盖**：复现 CICIoT2023 论文中定义的 30 多种攻击类型，包括 DDoS、DoS、侦察、暴力破解和 Mirai 风格的洪泛攻击。
- **实时监控**：主节点提供所有工作节点的实时反馈和输出收集。

### 🛠 技术栈

- **语言**：C++11
- **网络技术**：Socket API、TCP/UDP、HTTP
- **依赖库**：
  - `libssh`：用于远程节点编排和命令执行。
  - `libcurl`：用于模拟基于 Web 的应用层攻击。
  - `pthread`：用于高并发数据包生成。
- **运行环境**：Linux（推荐 Ubuntu）

### 📂 项目结构

```text
├── include/
│   └── attack_node.h              # 节点和攻击类型的数据结构
├── src/
│   ├── master/
│   │   └── master.cpp             # 中心控制器逻辑（基于SSH）
│   └── attacks/
│       ├── udp_flood.cpp          # 高并发 UDP/TCP 洪泛引擎
│       ├── httpflood.cpp          # 多线程 HTTP 洪泛工具
│       ├── sqli_attacker.cpp      # 基于 Web 的 SQL 注入模拟器
│       ├── command_attacker.cpp   # 命令注入模拟模块
│       ├── xss_attacker.cpp       # XSS 攻击模拟器
│       ├── backdoor_flood.cpp     # 后门攻击模块
│       ├── beef_attack.cpp        # BeEF 集成模块
│       └── uploading_attack.cpp   # 文件上传漏洞模拟器
├── scripts/                       # 辅助脚本
└── Makefile                       # 自动化构建系统
```

### ⚙️ 安装与使用

#### 前置条件
在主节点上安装依赖：
```bash
sudo apt-get install libssh-dev libcurl4-openssl-dev g++ make
```

#### 编译
使用提供的 Makefile 或手动命令构建系统：

1. **构建主控制器和攻击工具**：
   ```bash
   # 构建主控制器
   g++ -I./include src/master/master.cpp -o master_controller -lssh -pthread

   # 构建自定义攻击模块
   g++ src/attacks/udp_flood.cpp -o udp_flood -pthread
   g++ src/attacks/httpflood.cpp -o http_flood -lcurl -pthread
   g++ src/attacks/sqli_attacker.cpp -o sqli_flood -lcurl -lpthread
   ```

#### 执行
1. 确保所有工作节点可通过 SSH 访问。
2. 在主控制器中配置工作节点的 IP 和凭证。
3. 启动控制器：
   ```bash
   ./master_controller
   ```
4. 从交互菜单中选择攻击类别和类型。

### 🔧 工具与参考文献

本项目集成并参考了多种安全测试工具：

49. **hping3** - 网络数据包构造工具  
    Tools, K. hping3 Package Description. 2019.  
    可用链接：https://www.kali.org/tools/hping3（访问日期：2023年6月19日）

50. **UDPFlood** - UDP 洪泛工具  
    EPC-MSU. UDPFlood. 2023.  
    可用链接：https://github.com/EPC-MSU/udp-flood（访问日期：2023年6月19日）

51. **Slowloris** - 低带宽 HTTP DoS 攻击工具  
    Yaltirakli, G. Slowloris. 2015.  
    可用链接：https://github.com/gkbrk/slowloris（访问日期：2023年6月19日）

52. **Golang-HTTPFlood** - 使用 Go 编写的 HTTP 洪泛工具  
    Golang-HTTPFlood. 2020.  
    可用链接：https://github.com/Leeon123/golang-httpflood（访问日期：2023年6月19日）

53. **Nmap** - 网络安全扫描器  
    Lyon, G. Nmap Security Scanner. 2014.  
    可用链接：http://nmap.org/（访问日期：2023年6月22日）

54. **Fping** - ICMP ping 实用工具  
    Tools, K. Fping. 2023.  
    可用链接：https://fping.org/（访问日期：2023年6月19日）

55. **Vulscan** - Nmap 的漏洞扫描模块  
    SCIP. Vulscan. 2023.  
    可用链接：https://github.com/scipag/vulscan（访问日期：2023年6月19日）

56. **DVWA** - 非常脆弱的 Web 应用程序  
    DVWA. Damn Vulnerable Web Application. 2017.  
    可用链接：https://github.com/digininja/DVWA（访问日期：2023年6月19日）

57. **Remot3d** - 远程利用框架  
    KeepWannabe. Remot3d—An Easy Way To Exploiting. 2020.  
    可用链接：https://github.com/KeepWannabe/Remot3d（访问日期：2023年6月19日）

58. **BeEF** - 浏览器渗透框架  
    BEEF. The Browser Exploitation Framework. 2023.  
    可用链接：https://beefproject.com（访问日期：2023年6月19日）

59. **Hydra** - 网络认证破解工具  
    Maciejak, D. Hydra. 2023.  
    可用链接：https://github.com/vanhauser-thc/thc-hydra（访问日期：2023年6月19日）

60. **Ettercap** - 网络嗅探/拦截工具  
    Ornaghi, A.; Valleri, M. Ettercap. 2005.  
    可用链接：https://www.ettercap-project.org/（访问日期：2023年6月19日）

61. **Mirai 僵尸网络** - IoT 僵尸网络源代码  
    Gamblin, J. Mirai BotNet. 2017.  
    可用链接：https://github.com/jgamblin/Mirai-Source-Code（访问日期：2023年6月19日）

### ⚠️ 免责声明

本项目仅用于**教育和研究目的**。该系统只能在受控的实验室环境中使用，或仅针对您拥有明确测试许可的系统。未经授权访问或破坏计算机系统是违法行为，将受到法律制裁。

### 📄 许可证
本项目采用 MIT 许可证。
