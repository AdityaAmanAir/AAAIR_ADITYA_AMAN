#include <portfolio/service/StatsService.hpp>
#include <iostream>

namespace portfolio {
namespace service {

namespace {

nlohmann::json unavailable(const std::string& platform, const std::string& username,
                           const std::string& profile) {
    return {{"platform", platform}, {"username", username}, {"profile", profile}, {"available", false}};
}

int countDifficulty(const nlohmann::json& rows, const std::string& difficulty) {
    if (!rows.is_array()) return 0;
    for (const auto& row : rows) {
        if (row.is_object() && row.value("difficulty", "") == difficulty) {
            return row.value("count", 0);
        }
    }
    return 0;
}

}

nlohmann::json StatsService::fetchExternalJson(const std::string& host, const std::string& path) const {
    httplib::Client client(host);
    client.set_follow_location(true);
    client.set_connection_timeout(5);
    client.set_read_timeout(10);
    auto response = client.Get(path);
    if (!response || response->status != 200) return nlohmann::json::object();
    try {
        return nlohmann::json::parse(response->body);
    } catch (const std::exception& error) {
        std::cerr << "[StatsService] JSON error from " << host << ": " << error.what() << std::endl;
        return nlohmann::json::object();
    }
}

nlohmann::json StatsService::getLeetCodeStats(const std::string& username) const {
    const std::string query = R"({"query":"query userProfile($username: String!) { matchedUser(username: $username) { username profile { ranking } submitStats { acSubmissionNum { difficulty count submissions } } userContestRanking { rating globalRanking } } }","variables":{"username":")" + username + R"("}})";
    httplib::Client client("https://leetcode.com");
    client.set_connection_timeout(5);
    client.set_read_timeout(10);
    auto response = client.Post("/graphql", query, "application/json");
    if (!response || response->status != 200) return unavailable("LeetCode", username, "https://leetcode.com/u/" + username + "/");
    try {
        auto data = nlohmann::json::parse(response->body);
        const auto& user = data.at("data").at("matchedUser");
        if (user.is_null()) return unavailable("LeetCode", username, "https://leetcode.com/u/" + username + "/");
        const auto rows = user["submitStats"]["acSubmissionNum"];
        nlohmann::json result = {
            {"platform", "LeetCode"}, {"username", username}, {"available", true},
            {"profile", "https://leetcode.com/u/" + username + "/"},
            {"totalSolved", countDifficulty(rows, "All")},
            {"easySolved", countDifficulty(rows, "Easy")},
            {"mediumSolved", countDifficulty(rows, "Medium")},
            {"hardSolved", countDifficulty(rows, "Hard")},
            {"ranking", user["profile"].value("ranking", 0)}
        };
        if (user.contains("userContestRanking") && !user["userContestRanking"].is_null()) {
            result["contestRating"] = user["userContestRanking"].value("rating", 0.0);
            result["contestRank"] = user["userContestRanking"].value("globalRanking", 0);
        }
        return result;
    } catch (const std::exception& error) {
        std::cerr << "[StatsService] LeetCode response error: " << error.what() << std::endl;
        return unavailable("LeetCode", username, "https://leetcode.com/u/" + username + "/");
    }
}

nlohmann::json StatsService::getGitHubStats(const std::string& username) const {
    auto data = fetchExternalJson("https://api.github.com", "/users/" + username);
    if (!data.is_object() || !data.contains("login")) return unavailable("GitHub", username, "https://github.com/" + username);
    auto events = fetchExternalJson("https://api.github.com", "/users/" + username + "/events/public");
    return {
        {"platform", "GitHub"}, {"username", username}, {"available", true},
        {"profile", "https://github.com/" + username},
        {"publicRepos", data.value("public_repos", 0)},
        {"followers", data.value("followers", 0)},
        {"following", data.value("following", 0)},
        {"recentEvents", events.is_array() ? static_cast<int>(events.size()) : 0}
    };
}

nlohmann::json StatsService::getCodeforcesStats(const std::string& username) const {
    auto userData = fetchExternalJson("https://codeforces.com", "/api/user.info?handles=" + username);
    if (!userData.is_object() || userData.value("status", "") != "OK" ||
        !userData.contains("result") || !userData["result"].is_array() || userData["result"].empty()) {
        return unavailable("Codeforces", username, "https://codeforces.com/profile/" + username);
    }
    const auto& user = userData["result"][0];
    auto ratings = fetchExternalJson("https://codeforces.com", "/api/user.rating?handle=" + username);
    auto submissions = fetchExternalJson("https://codeforces.com", "/api/user.status?handle=" + username + "&from=1&count=10000");
    return {
        {"platform", "Codeforces"}, {"username", username}, {"available", true},
        {"profile", "https://codeforces.com/profile/" + username},
        {"rating", user.value("rating", 0)}, {"maxRating", user.value("maxRating", 0)},
        {"rank", user.value("rank", "unrated")}, {"maxRank", user.value("maxRank", "unrated")},
        {"contests", ratings.is_array() ? static_cast<int>(ratings.size()) : 0},
        {"submissions", submissions.is_array() ? static_cast<int>(submissions.size()) : 0}
    };
}

nlohmann::json StatsService::getAllStats(const nlohmann::json& handles) const {
    const std::string username = "AdityaAmanAir";
    nlohmann::json output = nlohmann::json::object();
    output["leetcode"] = getLeetCodeStats(handles.value("leetcode", username));
    output["github"] = getGitHubStats(handles.value("github", username));
    output["codeforces"] = getCodeforcesStats(handles.value("codeforces", username));
    output["kaggle"] = unavailable("Kaggle", username, "https://www.kaggle.com/" + username);
    output["hackerrank"] = unavailable("HackerRank", username, "https://www.hackerrank.com/profile/" + username);
    output["hackerearth"] = unavailable("HackerEarth", username, "https://www.hackerearth.com/@" + username + "/");
    return output;
}

} // namespace service
} // namespace portfolio
