#pragma once
#include <json.hpp>
#include <string>
#include <httplib.h>

namespace portfolio {
namespace service {

class StatsService {
public:
    nlohmann::json getLeetCodeStats(const std::string& username) const;
    nlohmann::json getGitHubStats(const std::string& username) const;
    nlohmann::json getCodeforcesStats(const std::string& username) const;
    
    // Aggregate stats for the frontend
    nlohmann::json getAllStats(const nlohmann::json& handles) const;

private:
    nlohmann::json fetchExternalJson(const std::string& host, const std::string& path) const;
};

} // namespace service
} // namespace portfolio
