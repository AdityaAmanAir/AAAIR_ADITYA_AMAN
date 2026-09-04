#include <portfolio/service/StatsService.hpp>
#include <iostream>

namespace portfolio {
namespace service {

nlohmann::json StatsService::fetchExternalJson(const std::string& host, const std::string& path) const {
    httplib::Client cli(host);
    cli.set_follow_location(true);
    cli.set_connection_timeout(5); // 5 seconds timeout
    
    auto res = cli.Get(path);
    if (res && res->status == 200) {
        try {
            return nlohmann::json::parse(res->body);
        } catch (const std::exception& e) {
            std::cerr << "[StatsService] Parse Error: " << e.what() << " from " << host << path << std::endl;
        }
    } else {
        std::cerr << "[StatsService] HTTP Error: " << (res ? std::to_string(res->status) : "Failed to connect") 
                  << " for " << host << path << std::endl;
    }
    return nlohmann::json::object();
}

nlohmann::json StatsService::getLeetCodeStats(const std::string& username) const {
    auto data = fetchExternalJson("https://leetcode-stats-api.herokuapp.com", "/" + username);
    if (data.empty() || data["status"] != "success") return nlohmann::json::object();
    
    return {
        {"totalSolved", data["totalSolved"]},
        {"easySolved", data["easySolved"]},
        {"mediumSolved", data["mediumSolved"]},
        {"hardSolved", data["hardSolved"]},
        {"acceptanceRate", data["acceptanceRate"]},
        {"ranking", data["ranking"]}
    };
}

nlohmann::json StatsService::getGitHubStats(const std::string& username) const {
    auto data = fetchExternalJson("https://api.github.com", "/users/" + username);
    if (data.empty()) return nlohmann::json::object();
    
    return {
        {"public_repos", data["public_repos"]},
        {"followers", data["followers"]},
        {"following", data["following"]},
        {"created_at", data["created_at"]}
    };
}

nlohmann::json StatsService::getCodeforcesStats(const std::string& username) const {
    auto data = fetchExternalJson("https://codeforces.com", "/api/user.info?handles=" + username);
    if (data.empty() || data["status"] != "OK") return nlohmann::json::object();
    
    auto info = data["result"][0];
    return {
        {"rating", info.value("rating", 0)},
        {"maxRating", info.value("maxRating", 0)},
        {"rank", info.value("rank", "unranked")},
        {"maxRank", info.value("maxRank", "unranked")}
    };
}

nlohmann::json StatsService::getAllStats(const nlohmann::json& handles) const {
    nlohmann::json output = nlohmann::json::object();
    
    if (handles.contains("leetcode")) output["leetcode"] = getLeetCodeStats(handles["leetcode"]);
    if (handles.contains("github")) output["github"] = getGitHubStats(handles["github"]);
    if (handles.contains("codeforces")) output["codeforces"] = getCodeforcesStats(handles["codeforces"]);
    
    return output;
}

} // namespace service
} // namespace portfolio
