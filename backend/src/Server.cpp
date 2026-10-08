#include <portfolio/core/Server.hpp>
#include <portfolio/service/PortfolioService.hpp>
#include <portfolio/controller/PortfolioController.hpp>
#include <portfolio/core/SecurityMiddleware.hpp>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <thread>

namespace portfolio {
namespace core {

Server::Server(const std::string& host, int port, const std::string& data_path,
               const std::string& certificate_path, const std::string& private_key_path)
    : host_(host), port_(port), data_path_(data_path),
      certificate_path_(certificate_path), private_key_path_(private_key_path),
      thread_count_(std::max(4u, std::thread::hardware_concurrency())) {}

bool Server::start() {
    if (certificate_path_.empty() != private_key_path_.empty()) {
        std::cerr << "[Server] Both TLS_CERT_PATH and TLS_KEY_PATH must be configured for HTTPS" << std::endl;
        return false;
    }

    if (!certificate_path_.empty()) {
        svr_ = std::make_unique<httplib::SSLServer>(certificate_path_.c_str(), private_key_path_.c_str());
        if (!svr_->is_valid()) {
            std::cerr << "[Server] Failed to load TLS certificate or private key" << std::endl;
            return false;
        }
    } else {
        svr_ = std::make_unique<httplib::Server>();
    }
    auto& server = *svr_;

    // ── Dual-Stack (IPv4 & IPv6) Socket Configuration ──
    server.set_socket_options([](socket_t sock) {
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#ifdef IPV6_V6ONLY
        int v6only = 0;
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&v6only, sizeof(v6only));
#endif
    });

    // ── Multi-threading ──
    server.new_task_queue = [this] {
        return new httplib::ThreadPool(thread_count_);
    };

    // ── Security middleware ──
    server.set_pre_routing_handler(SecurityMiddleware::preRouting);
    server.set_post_routing_handler(SecurityMiddleware::postRouting);

    // ── Error handlers ──
    server.set_error_handler([](const httplib::Request&, httplib::Response& res) {
        std::string body = "{\"error\":\"" + std::to_string(res.status) + "\", \"message\":\"Resource not found or server error\"}";
        res.set_content(body, "application/json");
    });

    // ── Register API routes ──
    data_service_.load(data_path_);
    controller::ApiController apiController(data_service_, stats_service_);
    apiController.registerRoutes(server);

    server.Get("/resume/download", [](const httplib::Request& req, httplib::Response& res) {
        const std::filesystem::path file_name(req.get_param_value("file"));
        std::string extension = file_name.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });

        if (file_name.empty() || file_name.filename() != file_name ||
            (extension != ".pdf" && extension != ".docx")) {
            res.status = httplib::StatusCode::BadRequest_400;
            res.set_content("A PDF or DOCX filename is required.", "text/plain");
            return;
        }

        const std::filesystem::path pdf_path = std::filesystem::path("./DataBase/resume") / file_name;
        std::error_code file_error;
        if (!std::filesystem::is_regular_file(pdf_path, file_error)) {
            res.status = httplib::StatusCode::NotFound_404;
            res.set_content("PDF not found.", "text/plain");
            return;
        }

        std::string download_name = file_name.filename().string();
        for (char& character : download_name) {
            const auto value = static_cast<unsigned char>(character);
            if (!std::isalnum(value) && character != '.' && character != '-' && character != '_') {
                character = '_';
            }
        }
        res.set_header("Content-Disposition", "attachment; filename=\"" + download_name + "\"");
        const std::string content_type = extension == ".pdf"
            ? "application/pdf"
            : "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        res.set_file_content(pdf_path.string(), content_type);
    });

    // ── Serve static files ──
    server.set_mount_point("/resume/", "./DataBase/resume");
    // Serving both root and frontend for compatibility
    server.set_mount_point("/", "./frontend");
    server.set_mount_point("/static", "./static");

    if (!server.bind_to_port(host_, port_)) {
        std::cerr << "[Server] Failed to bind " << (certificate_path_.empty() ? "http://" : "https://")
                  << host_ << ":" << port_ << std::endl;
        if (port_ < 1024) {
            std::cerr << "[Server] Ports below 1024 may require elevated privileges; try: sudo ./build/server" << std::endl;
        }
        return false;
    }

    std::cout << "[Server] Listening on " << (certificate_path_.empty() ? "http://" : "https://")
              << host_ << ":" << port_ << " (threads: " << thread_count_ << ")" << std::endl;
    return server.listen_after_bind();
}

} // namespace core
} // namespace portfolio
