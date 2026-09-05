#include <portfolio/service/StatsService.hpp>
#include <iostream>
#include <regex>
#include <set>

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

double acceptanceRate(const nlohmann::json& accepted, const nlohmann::json& total,
                      const std::string& difficulty) {
    int acceptedSubmissions = 0;
    int totalSubmissions = 0;
    for (const auto& row : accepted) {
        if (row.is_object() && row.value("difficulty", "") == difficulty) {
            acceptedSubmissions = row.value("submissions", 0);
            break;
        }
    }
    for (const auto& row : total) {
        if (row.is_object() && row.value("difficulty", "") == difficulty) {
            totalSubmissions = row.value("submissions", 0);
            break;
        }
    }
    return totalSubmissions == 0 ? 0.0 : 100.0 * acceptedSubmissions / totalSubmissions;
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
    auto data = fetchExternalJson("https://leetcode-api-pied.vercel.app", "/user/" + username);
    if (!data.is_object() || !data.contains("submitStats")) {
        return unavailable("LeetCode", username, "https://leetcode.com/u/" + username + "/");
    }
    try {
        const auto& rows = data.at("submitStats").at("acSubmissionNum");
        const auto& totalRows = data.at("submitStats").at("totalSubmissionNum");
        nlohmann::json result = {
            {"platform", "LeetCode"}, {"username", username}, {"available", true},
            {"profile", "https://leetcode.com/u/" + username + "/"},
            {"totalSolved", countDifficulty(rows, "All")},
            {"easySolved", countDifficulty(rows, "Easy")},
            {"mediumSolved", countDifficulty(rows, "Medium")},
            {"hardSolved", countDifficulty(rows, "Hard")},
            {"ranking", data["profile"].value("ranking", 0)},
            {"acceptanceRate", acceptanceRate(rows, totalRows, "All")},
            {"easyAcceptance", acceptanceRate(rows, totalRows, "Easy")},
            {"mediumAcceptance", acceptanceRate(rows, totalRows, "Medium")},
            {"hardAcceptance", acceptanceRate(rows, totalRows, "Hard")}
        };
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
    const auto ratingRows = ratings.value("result", nlohmann::json::array());
    const auto submissionRows = submissions.value("result", nlohmann::json::array());
    std::set<std::string> solvedProblems;
    int acceptedSubmissions = 0;
    for (const auto& submission : submissionRows) {
        if (submission.value("verdict", "") == "OK") {
            const auto problem = submission.value("problem", nlohmann::json::object());
            solvedProblems.insert(std::to_string(problem.value("contestId", 0)) + ":" + problem.value("index", ""));
            ++acceptedSubmissions;
        }
    }
    return {
        {"platform", "Codeforces"}, {"username", username}, {"available", true},
        {"profile", "https://codeforces.com/profile/" + username},
        {"rating", user.value("rating", 0)}, {"maxRating", user.value("maxRating", 0)},
        {"rank", user.value("rank", "unrated")}, {"maxRank", user.value("maxRank", "unrated")},
        {"contests", ratingRows.is_array() ? static_cast<int>(ratingRows.size()) : 0},
        {"submissions", submissionRows.is_array() ? static_cast<int>(submissionRows.size()) : 0},
        {"solved", static_cast<int>(solvedProblems.size())},
        {"acceptedSubmissions", acceptedSubmissions}
    };
}

nlohmann::json StatsService::getCodeChefStats(const std::string& username) const {
    httplib::Client client("https://www.codechef.com");
    client.set_follow_location(true);
    client.set_connection_timeout(5);
    client.set_read_timeout(10);
    auto response = client.Get("/users/" + username);
    if (!response || response->status != 200) {
        return unavailable("CodeChef", username, "https://www.codechef.com/users/" + username);
    }
    std::smatch solvedMatch;
    std::smatch ratingMatch;
    std::smatch contestsMatch;
    const std::regex solvedPattern(R"(Total Problems Solved:\s*([0-9]+))");
    const std::regex ratingPattern(R"(<div class="rating-number">\s*([0-9]+))");
    const std::regex contestsPattern(R"(Contests \(([0-9]+)\))");
    if (!std::regex_search(response->body, solvedMatch, solvedPattern) ||
        !std::regex_search(response->body, ratingMatch, ratingPattern)) {
        return unavailable("CodeChef", username, "https://www.codechef.com/users/" + username);
    }
    const int contests = std::regex_search(response->body, contestsMatch, contestsPattern)
        ? std::stoi(contestsMatch[1].str()) : 0;
    return {
        {"platform", "CodeChef"}, {"username", username}, {"available", true},
        {"profile", "https://www.codechef.com/users/" + username},
        {"solved", std::stoi(solvedMatch[1].str())},
        {"rating", std::stoi(ratingMatch[1].str())},
        {"contests", contests}
    };
}

nlohmann::json StatsService::getAllStats(const nlohmann::json& handles) const {
    const std::string username = "AdityaAmanAir";
    nlohmann::json output = nlohmann::json::object();
    output["leetcode"] = getLeetCodeStats(handles.value("leetcode", username));
    output["github"] = getGitHubStats(handles.value("github", username));
    output["codeforces"] = getCodeforcesStats(handles.value("codeforces", username));
    output["codechef"] = getCodeChefStats("adityaamanair");
    int totalSolved = 0;
    int totalContests = 0;
    nlohmann::json sources = nlohmann::json::array();
    for (const auto& key : {"leetcode", "codeforces", "codechef"}) {
        if (output[key].value("available", false)) {
            totalSolved += output[key].value("solved", output[key].value("totalSolved", 0));
            totalContests += output[key].value("contests", 0);
            sources.push_back(output[key].value("platform", key));
        }
    }
    output["summary"] = {
        {"totalSolved", totalSolved},
        {"totalContests", totalContests},
        {"sources", sources},
        {"excluded", {"Kaggle", "HackerRank", "HackerEarth"}}
    };
    return output;
}

} // namespace service
} // namespace portfolio
