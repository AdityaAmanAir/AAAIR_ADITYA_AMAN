#include <portfolio/core/Server.hpp>
#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cctype>

// Load .env file into process environment variables
static void loadEnv(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[Env] Could not open " << path << " - skipping" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);
        // strip surrounding whitespace
        while (!key.empty() && isspace((unsigned char)key.front())) key.erase(key.begin());
        while (!key.empty() && isspace((unsigned char)key.back()))  key.pop_back();
        while (!val.empty() && isspace((unsigned char)val.front())) val.erase(val.begin());
        while (!val.empty() && isspace((unsigned char)val.back()))  val.pop_back();
        setenv(key.c_str(), val.c_str(), 1); // 1 = overwrite existing env vars
    }
    std::cout << "[Env] Loaded " << path << std::endl;
}

int main() {
    // Load environment variables from .env before anything else
    loadEnv(".env");

    const std::string host = "0.0.0.0";
    const std::string data_path = "data.json";

    std::cout << "[Main] Initializing Generic Backend..." << std::endl;

    // HTTP Server thread
    std::thread http_thread([&]() {
        portfolio::core::Server http_server(host, 80, data_path);
        http_server.start();
    });

    http_thread.join();

    return 0;
}
