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

Server::Server(const std::string& host, int port, const std::string& data_path)
    : host_(host), port_(port), data_path_(data_path),
      thread_count_(std::max(4u, std::thread::hardware_concurrency())) {}

bool Server::start() {
    // ── Dual-Stack (IPv4 & IPv6) Socket Configuration ──
    svr_.set_socket_options([](socket_t sock) {
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#ifdef IPV6_V6ONLY
        int v6only = 0;
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&v6only, sizeof(v6only));
#endif
    });

    // ── Multi-threading ──
    svr_.new_task_queue = [this] {
        return new httplib::ThreadPool(thread_count_);
    };

    // ── Security middleware ──
    svr_.set_pre_routing_handler(SecurityMiddleware::preRouting);
    svr_.set_post_routing_handler(SecurityMiddleware::postRouting);

    // ── Error handlers ──
    svr_.set_error_handler([](const httplib::Request&, httplib::Response& res) {
        std::string body = "{\"error\":\"" + std::to_string(res.status) + "\", \"message\":\"Resource not found or server error\"}";
        res.set_content(body, "application/json");
    });

    // ── Register API routes ──
    data_service_.load(data_path_);
    controller::ApiController apiController(data_service_, stats_service_);
    apiController.registerRoutes(svr_);

    svr_.Get("/resume/download", [](const httplib::Request& req, httplib::Response& res) {
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
    svr_.set_mount_point("/resume/", "./DataBase/resume");
    // Serving both root and frontend for compatibility
    svr_.set_mount_point("/", "./frontend");
    svr_.set_mount_point("/static", "./static");

    if (!svr_.bind_to_port(host_, port_)) {
        std::cerr << "[Server] Failed to bind http://" << host_ << ":" << port_ << std::endl;
        if (port_ < 1024) {
            std::cerr << "[Server] Ports below 1024 may require elevated privileges; try: sudo ./build/server" << std::endl;
        }
        return false;
    }

    std::cout << "[Server] Listening on http://" << host_ << ":" << port_ << " (threads: " << thread_count_ << ")" << std::endl;
    return svr_.listen_after_bind();
}

} // namespace core
} // namespace portfolio
