#include <portfolio/core/Server.hpp>
#include <portfolio/service/PortfolioService.hpp>
#include <portfolio/controller/PortfolioController.hpp>
#include <portfolio/core/SecurityMiddleware.hpp>
#include <iostream>
#include <thread>

namespace portfolio {
namespace core {

Server::Server(const std::string& host, int port, const std::string& data_path)
    : host_(host), port_(port), data_path_(data_path),
      thread_count_(std::max(4u, std::thread::hardware_concurrency())) {}

void Server::start() {
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

    // ── Serve static files ──
    // Serving both root and frontend for compatibility
    svr_.set_mount_point("/", "./frontend");
    svr_.set_mount_point("/static", "./static");

    std::cout << "[Server] Listening on http://" << host_ << ":" << port_ << " (threads: " << thread_count_ << ")" << std::endl;

    svr_.listen(host_.c_str(), port_);
}

} // namespace core
} // namespace portfolio
