#include <portfolio/core/Server.hpp>
#include <iostream>
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
    const char* port_value = std::getenv("PORT");
    const char* certificate_value = std::getenv("TLS_CERT_PATH");
    const char* private_key_value = std::getenv("TLS_KEY_PATH");
    const std::string certificate_path = certificate_value ? certificate_value : "";
    const std::string private_key_path = private_key_value ? private_key_value : "";
    const bool https_enabled = !certificate_path.empty() && !private_key_path.empty();
    if (certificate_path.empty() != private_key_path.empty()) {
        std::cerr << "[Main] Set both TLS_CERT_PATH and TLS_KEY_PATH to enable HTTPS" << std::endl;
        return 1;
    }
    const int port = port_value ? std::stoi(port_value) : (https_enabled ? 443 : 80);
    const std::string data_path = "data.json";

    std::cout << "[Main] Initializing Generic Backend..." << std::endl;

    portfolio::core::Server server(host, port, data_path, certificate_path, private_key_path);
    return server.start() ? 0 : 1;
}
