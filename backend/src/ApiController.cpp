#include <portfolio/controller/ApiController.hpp>
#include <cstdlib>

namespace portfolio {
namespace controller {

ApiController::ApiController(const service::GenericDataService& dataService, const service::StatsService& statsService)
    : dataService_(dataService), statsService_(statsService) {}

void ApiController::setCorsHeaders(const httplib::Request& req, httplib::Response& res) {
    if (req.has_header("Origin")) {
        res.set_header("Access-Control-Allow-Origin", req.get_header_value("Origin"));
    } else {
        res.set_header("Access-Control-Allow-Origin", "*");
    }
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
    res.set_header("Access-Control-Max-Age", "86400");
    
    // Integrity Headers
    res.set_header("X-Content-Type-Options", "nosniff");
    res.set_header("X-Frame-Options", "DENY");
}

bool isAuthorized(const httplib::Request& req) {
    // Rudimentary check: request must come from the same host or have the correct Referer
    if (req.has_header("Referer")) {
        auto referer = req.get_header_value("Referer");
        if (referer.find("localhost") != std::string::npos || referer.find("127.0.0.1") != std::string::npos) {
            return true;
        }
    }
    // Fallback for direct browser access (which we want to discourage but allow for dev)
    return true; 
}

static bool sendPayloadToScript(const std::string& url, const std::string& jsonStr) {
    std::cout << "[Proxy] Forwarding to Google Apps Script: " << url << std::endl;
    std::cout << "[Proxy] Payload: " << jsonStr << std::endl;
    std::string cmd = "curl -s -L -X POST \"" + url + "\" -H \"Content-Type: application/json\" --data-binary @-";
    FILE* p = popen(cmd.c_str(), "w");
    if (!p) {
        std::cerr << "[Proxy] Error: popen failed" << std::endl;
        return false;
    }
    fwrite(jsonStr.c_str(), 1, jsonStr.size(), p);
    int status = pclose(p);
    std::cout << "[Proxy] Forwarding completed with code: " << status << std::endl;
    return (status == 0);
}

void ApiController::registerRoutes(httplib::Server& svr) const {
    // ── OPTIONS preflight handler ──
    svr.Options("/api/(.*)", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        res.status = 204;
    });

    // ── Get All Data (The Full Dataset) ──
    svr.Get("/api/data", [&](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        if (!isAuthorized(req)) {
            res.status = 403;
            res.set_content("{\"error\":\"Forbidden\", \"code\":\"403-AUTH-01\"}", "application/json");
            return;
        }
        res.set_content(dataService_.getData().dump(), "application/json");
    });

    // ── Get Live Coding Stats ──
    svr.Get("/api/stats", [&](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        
        auto fullData = dataService_.getData();
        nlohmann::json handles = nlohmann::json::object();
        
        // Extract handles from social links in data.json
        if (fullData.contains("profile") && fullData["profile"].contains("social")) {
            for (const auto& s : fullData["profile"]["social"]) {
                std::string platform = s.value("platform", "");
                std::string url = s.value("url", "");
                if (platform.empty() || url.empty()) continue;
                
                std::string username = url.substr(url.find_last_of("/") + 1);
                std::transform(platform.begin(), platform.end(), platform.begin(), ::tolower);
                handles[platform] = username;
            }
        }
        
        res.set_content(statsService_.getAllStats(handles).dump(), "application/json");
    });

    // ── Get Specific Section ──
    svr.Get("/api/section/:id", [&](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        if (!isAuthorized(req)) {
            res.status = 403;
            res.set_content("{\"error\":\"Forbidden\"}", "application/json");
            return;
        }
        auto id = req.path_params.at("id");
        auto section = dataService_.getSection(id);
        if (section.empty()) {
            res.status = 404;
            res.set_content("{\"error\":\"Section not found\"}", "application/json");
        } else {
            res.set_content(section.dump(), "application/json");
        }
    });

    // ── Contact / Inquiry Proxy Handler ──
    svr.Post("/api/contact", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        try {
            auto bodyJson = nlohmann::json::parse(req.body);
            std::string name = bodyJson.value("NAME", bodyJson.value("name", ""));
            std::string email = bodyJson.value("EMAIL", bodyJson.value("email", ""));
            std::string datetime = bodyJson.value("DATE&TIME", bodyJson.value("datetime", ""));
            std::string subject = bodyJson.value("SUBJECT", bodyJson.value("subject", ""));
            std::string bodyText = bodyJson.value("BODY", bodyJson.value("body", ""));

            std::string scriptUrl = "https://script.google.com/macros/s/AKfycbzkbvZEsIZLdeOcoxNywyMVpl1mV4pz-ihFdL8wYbnGNVzOH9Bwa6ipa36abDEjJu4Ftg/exec";
            
            nlohmann::json payload;
            payload["NAME"] = name;
            payload["EMAIL"] = email;
            payload["DATE&TIME"] = datetime;
            payload["SUBJECT"] = subject;
            payload["BODY"] = bodyText;

            sendPayloadToScript(scriptUrl, payload.dump());

            res.set_content("{\"status\":\"success\", \"message\":\"Query forwarded successfully\"}", "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content("{\"error\":\"Invalid request body\"}", "application/json");
        }
    });

    // ── Comment Proxy Handler ──
    svr.Post("/api/comment", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        try {
            auto bodyJson = nlohmann::json::parse(req.body);
            std::string name = bodyJson.value("NAME", bodyJson.value("name", ""));
            std::string position = bodyJson.value("Position With Institution", bodyJson.value("POSITION_WITH_INSTITUTION", bodyJson.value("position", "")));
            std::string gender = bodyJson.value("Gender", bodyJson.value("GENDER", bodyJson.value("gender", "")));
            std::string comment = bodyJson.value("Comment", bodyJson.value("COMMENT", bodyJson.value("comment", "")));

            std::string scriptUrl = "https://script.google.com/macros/s/AKfycbzb9XEjjUmMhJw2I9CXIDCfsuB-mmkq_T4dUGNx9xCiysDnFQCrs-U9s6uea8nJqKLF-g/exec";

            nlohmann::json payload;
            payload["NAME"] = name;
            payload["Position With Institution"] = position;
            payload["POSITION_WITH_INSTITUTION"] = position;
            payload["Gender"] = gender;
            payload["GENDER"] = gender;
            payload["Comment"] = comment;
            payload["COMMENT"] = comment;

            sendPayloadToScript(scriptUrl, payload.dump());

            res.set_content("{\"status\":\"success\", \"message\":\"Comment forwarded successfully\"}", "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content("{\"error\":\"Invalid request body\"}", "application/json");
        }
    });

}

} // namespace controller
} // namespace portfolio
