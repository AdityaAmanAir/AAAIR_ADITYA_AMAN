#include <portfolio/controller/PortfolioController.hpp>

namespace portfolio {
namespace controller {

PortfolioController::PortfolioController(const service::PortfolioService& service)
    : service_(service) {}

void PortfolioController::setCorsHeaders(const httplib::Request& req, httplib::Response& res) {
    // Same-origin CORS: only allow requests from our own host
    if (req.has_header("Origin")) {
        const auto& origin = req.get_header_value("Origin");
        // Allow localhost origins for development
        if (origin.find("localhost") != std::string::npos ||
            origin.find("127.0.0.1") != std::string::npos) {
            res.set_header("Access-Control-Allow-Origin", origin);
        }
    }
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Access-Control-Max-Age", "86400");
}

void PortfolioController::registerRoutes(httplib::Server& svr) const {
    // ── OPTIONS preflight handler ──
    svr.Options("/api/(.*)", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        res.status = 204;
    });

    svr.Get("/api/profile", [&](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        res.set_content(service_.getProfile().dump(), "application/json");
    });

    svr.Get("/api/education", [&](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        res.set_content(service_.getEducation().dump(), "application/json");
    });

    svr.Get("/api/certificates", [&](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        res.set_content(service_.getCertificates().dump(), "application/json");
    });

    svr.Get("/api/awards", [&](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        res.set_content(service_.getAwards().dump(), "application/json");
    });
}

} // namespace controller
} // namespace portfolio
