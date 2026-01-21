/*
 * BeEF Browser Hijacking Traffic Simulator
 * Simulation: Mimics a "Zombie" browser communicating with a BeEF Server
 * Traffic Feature: Periodic HTTP GET/POST polling (Heartbeat)
 * Dependency: libcurl
 * Compile: g++ beef_hijack_flood.cpp -o beef_hijack_flood -lcurl -lpthread
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <curl/curl.h>

// === 配置区域 ===
// 注意：在真实 BeEF 攻击中，流量是指向 BeEF 服务器的。
// 为了复现论文流量，您可以把 Target 设为您的收集端 IP。
const std::string TARGET_IP = "192.168.0.230";
const std::string BASE_URL = "http://" + TARGET_IP;

const int THREAD_COUNT = 10;     // 模拟 10 个被劫持的浏览器 tab
const int ATTACK_DURATION = 60;  // 持续 60 秒

// === 全局控制 ===
std::atomic<bool> is_running(true);
std::atomic<long> total_heartbeats(0);

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

// 1. 模拟加载 hook.js (BeEF 的入口特征)
void simulate_load_hook(CURL *curl) {
    std::string url = BASE_URL + "/hook.js";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_perform(curl);
}

// 2. 模拟心跳轮询 (Command Polling)
// BeEF 默认使用 /dh?browser_token=... 来获取指令
void simulate_polling(CURL *curl, const std::string& session_token) {
    // 构造典型的 BeEF 轮询 URL
    // 参数通常包含 browser (Session ID) 和 wait (轮询等待时间)
    std::string url = BASE_URL + "/dh?browser=" + session_token + "&wait=5";
    
    // 构造一些伪造的浏览器指纹数据 (作为 POST 发送，模拟回传数据)
    std::string fingerprint = "{\"results\":{\"browser_name\":\"Chrome\",\"version\":\"120.0\",\"os\":\"Windows\"}}";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fingerprint.c_str());
    
    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        total_heartbeats++;
    }
}

void zombie_worker(int thread_id) {
    CURL *curl = curl_easy_init();
    if (!curl) return;

    // 为每个线程生成一个假的 BeEF Session Token
    std::string session_token = "sess_" + std::to_string(thread_id) + "_rnd" + std::to_string(rand());

    // 设置通用的 HTTP 伪装头部
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json; charset=UTF-8");
    headers = curl_slist_append(headers, "X-Beef-Hook: 1"); // 某些版本可能有此特征
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    // 第一步：加载 Hook 文件 (模拟浏览器刚打开中毒页面)
    if (is_running) {
        simulate_load_hook(curl);
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // 第二步：进入死循环，发送心跳包 (模拟浏览器保持连接)
    while (is_running) {
        simulate_polling(curl, session_token);
        
        // BeEF 的心跳通常是几秒一次，为了产生密集流量，我们这里设为 100ms
        // 如果要完全拟真，可以设为 1000ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    srand(time(NULL));

    std::cout << "=== BeEF Browser Hijacking Simulation Started ===" << std::endl;
    std::cout << "Target (Fake BeEF Server): " << BASE_URL << std::endl;
    std::cout << "Simulating " << THREAD_COUNT << " hooked browsers..." << std::endl;

    std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(zombie_worker, i + 1);
    }

    std::this_thread::sleep_for(std::chrono::seconds(ATTACK_DURATION));
    is_running = false;

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    std::cout << "\n=== Finished ===" << std::endl;
    std::cout << "Total Heartbeats Sent: " << total_heartbeats.load() << std::endl;
    std::cout << "Avg RPS:               " << (total_heartbeats.load() / ATTACK_DURATION) << std::endl;

    curl_global_cleanup();
    return 0;
}