#ifndef ATTACK_NODE_H
#define ATTACK_NODE_H

#include <string>
#include <vector>

struct AttackNode {
    std::string ip;
    std::string username;
    std::string password;
};

enum class AttackCategory {
    DDOS,
    DOS,
    RECON,
    WEB_BASED,
    BRUTE_FORCE,
    SPOOFING,
    MIRAI
};

enum class AttackType {
    // DDoS
    DDOS_ACK_FRAGMENTATION,
    DDOS_UDP_FLOOD,
    DDOS_SLOWLORIS,
    DDOS_ICMP_FLOOD,
    DDOS_RST_FIN_FLOOD,
    DDOS_PSH_ACK_FLOOD,
    DDOS_HTTP_FLOOD,
    DDOS_UDP_FRAGMENTATION,
    DDOS_ICMP_FRAGMENTATION,
    DDOS_TCP_FLOOD,
    DDOS_SYN_FLOOD,
    DDOS_SYNONYMOUS_IP_FLOOD,

    // DoS
    DOS_TCP_FLOOD,
    DOS_HTTP_FLOOD,
    DOS_SYN_FLOOD,
    DOS_UDP_FLOOD,

    // Recon
    RECON_PING_SWEEP,
    RECON_OS_SCAN,
    RECON_VULNERABILITY_SCAN,
    RECON_PORT_SCAN,
    RECON_HOST_DISCOVERY,

    // Web-Based
    WEB_SQL_INJECTION,
    WEB_COMMAND_INJECTION,
    WEB_BACKDOOR_MALWARE,
    WEB_UPLOADING_ATTACK,
    WEB_XSS,
    WEB_BROWSER_HIJACKING,

    // Brute Force
    BRUTE_DICTIONARY,

    // Spoofing
    SPOOF_ARP,
    SPOOF_DNS,

    // Mirai
    MIRAI_GREIP_FLOOD,
    MIRAI_GREETH_FLOOD,
    MIRAI_UDP_PLAIN
};

#endif // ATTACK_NODE_H
