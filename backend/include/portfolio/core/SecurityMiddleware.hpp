#pragma once
#include <httplib.h>
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace portfolio {
namespace core {

class SecurityMiddleware {
public:
    // Pre-routing handler: validates requests before they reach controllers
    static httplib::Server::HandlerResponse preRouting(const httplib::Request& req, httplib::Response& res) {
        // Path traversal protection
        if (req.path.find("..") != std::string::npos ||
            req.path.find("//") != std::string::npos ||
            req.path.find("\\") != std::string::npos) {
            res.status = 403;
            res.set_content("{\"error\":\"Forbidden\"}", "application/json");
            logRequest(req, res, 0);
            return httplib::Server::HandlerResponse::Handled;
        }

        // Block suspicious query strings
        if (req.has_param("exec") || req.has_param("cmd") || req.has_param("eval")) {
            res.status = 400;
            res.set_content("{\"error\":\"Bad Request\"}", "application/json");
            logRequest(req, res, 0);
            return httplib::Server::HandlerResponse::Handled;
        }

        return httplib::Server::HandlerResponse::Unhandled;
    }

    // Post-routing handler: sets security headers on every response
    static void postRouting(const httplib::Request& req, httplib::Response& res) {
        (void)req;
        // Prevent MIME type sniffing
        res.set_header("X-Content-Type-Options", "nosniff");

        // Prevent clickjacking
        res.set_header("X-Frame-Options", "DENY");

        // XSS protection (legacy browsers)
        res.set_header("X-XSS-Protection", "1; mode=block");

        // Content Security Policy
        res.set_header("Content-Security-Policy",
            "default-src 'self'; "
            "script-src 'self' 'unsafe-inline' 'unsafe-eval'; "
            "style-src 'self' https://fonts.googleapis.com; "
            "font-src 'self' https://fonts.gstatic.com; "
            "img-src 'self' https://leetcode-stats-api.herokuapp.com https://github-readme-stats.vercel.app https://i.ibb.co data:; "
            "connect-src 'self' https://script.google.com https://script.googleusercontent.com; "
            "frame-ancestors 'none'"
        );

        // Referrer policy (relaxed slightly for Chrome dev compatibility)
        res.set_header("Referrer-Policy", "no-referrer-when-downgrade");

        // Permissions policy (restrict sensitive APIs)
        res.set_header("Permissions-Policy",
            "camera=(), microphone=(), geolocation=(), payment=()"
        );

        // Cache control: no-store for instant live updates on CSS/JS/HTML changes
        res.set_header("Cache-Control", "no-cache, no-store, must-revalidate");
    }

    // Request logging
    static void logRequest(const httplib::Request& req, const httplib::Response& res, int duration_ms) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");

        std::cout << "[" << oss.str() << "] "
                  << req.method << " " << req.path
                  << " → " << res.status
                  << " (" << duration_ms << "ms)"
                  << std::endl;
    }
};

} // namespace core
} // namespace portfolio
