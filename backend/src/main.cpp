#include <portfolio/core/Server.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <thread>

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
    const char* http_port_value = std::getenv("HTTP_PORT");
    const char* certificate_value = std::getenv("TLS_CERT_PATH");
    const char* private_key_value = std::getenv("TLS_KEY_PATH");
    const char* https_host_value = std::getenv("HTTPS_HOST");
    const std::string certificate_path = certificate_value ? certificate_value : "";
    const std::string private_key_path = private_key_value ? private_key_value : "";
    const std::string https_host = https_host_value ? https_host_value : "";
    const bool https_enabled = !certificate_path.empty() && !private_key_path.empty();
    if (certificate_path.empty() != private_key_path.empty()) {
        std::cerr << "[Main] Set both TLS_CERT_PATH and TLS_KEY_PATH to enable HTTPS" << std::endl;
        return 1;
    }
    if (https_enabled && https_host.empty()) {
        std::cerr << "[Main] Set HTTPS_HOST to the canonical hostname to enable HTTP-to-HTTPS redirects" << std::endl;
        return 1;
    }
    const int port = port_value ? std::stoi(port_value) : (https_enabled ? 443 : 80);
    const int http_port = http_port_value ? std::stoi(http_port_value) : 80;
    const std::string data_path = "data.json";

    std::cout << "[Main] Initializing Generic Backend..." << std::endl;

    if (https_enabled) {
        httplib::Server redirect_server;
        redirect_server.set_pre_routing_handler([https_host](const httplib::Request& request, httplib::Response& response) {
            const std::string target = request.target.empty() ? "/" : request.target;
            response.set_redirect("https://" + https_host + target, httplib::StatusCode::PermanentRedirect_308);
            return httplib::Server::HandlerResponse::Handled;
        });

        if (!redirect_server.bind_to_port(host, http_port)) {
            std::cerr << "[Main] Failed to bind HTTP redirect listener on " << host << ":" << http_port << std::endl;
            return 1;
        }

        std::cout << "[Main] Redirecting HTTP on port " << http_port << " to https://" << https_host << std::endl;
        std::thread redirect_thread([&redirect_server]() {
            redirect_server.listen_after_bind();
        });

        portfolio::core::Server server(host, port, data_path, certificate_path, private_key_path);
        const bool server_started = server.start();
        redirect_server.stop();
        redirect_thread.join();
        return server_started ? 0 : 1;
    }

    portfolio::core::Server server(host, port, data_path, certificate_path, private_key_path);
    return server.start() ? 0 : 1;
}
