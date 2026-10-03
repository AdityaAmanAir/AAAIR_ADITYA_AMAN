#include <portfolio/controller/ApiController.hpp>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <map>
#include <set>
#include <sstream>
#include <vector>

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

static std::string getRelevantPortfolioData(const nlohmann::json& data, const std::string& question) {
    const auto profile = data.value("profile", nlohmann::json::object());
    std::string profileContext = "Name: " + profile.value("name", std::string("Unknown"));
    const int birthYear = profile.value("birth_year", 0);
    if (birthYear > 0) profileContext += ". Birth year: " + std::to_string(birthYear);

    std::set<std::string> queryWords;
    static const std::set<std::string> ignoredWords = {
        "what", "who", "when", "where", "why", "how", "is", "are", "was", "were",
        "the", "and", "for", "with", "from", "about", "this", "that", "name",
        "tell", "please", "can", "could", "would", "you", "your", "me", "my", "his", "her",
        "answer", "one", "word", "listed", "did", "does"
    };
    std::istringstream questionStream(question);
    std::string word;
    while (questionStream >> word) {
        for (char& character : word) {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
            if (!std::isalnum(static_cast<unsigned char>(character))) character = ' ';
        }
        std::istringstream wordStream(word);
        std::string token;
        while (wordStream >> token) {
            if (token.size() >= 3 && ignoredWords.count(token) == 0) queryWords.insert(token);
        }
    }

    std::vector<std::pair<int, nlohmann::json>> rankedSections;
    const auto sections = data.value("sections", nlohmann::json::array());
    if (sections.is_array()) {
        for (const auto& section : sections) {
            std::string searchable = section.dump();
            std::transform(searchable.begin(), searchable.end(), searchable.begin(), [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
            int score = 0;
            for (const auto& token : queryWords) {
                if (searchable.find(token) != std::string::npos) ++score;
            }
            if (score > 0) rankedSections.emplace_back(score, section);
        }
    }

    std::sort(rankedSections.begin(), rankedSections.end(), [](const auto& left, const auto& right) {
        return left.first > right.first;
    });

    const auto appendEntry = [](std::string& output, const nlohmann::json& entry) {
        static const std::vector<std::string> fields = {
            "institution", "degree", "period", "name", "title", "detail",
            "description", "exam", "score", "issuer", "date", "year",
            "highlight", "level"
        };
        bool hasFacts = false;
        for (const auto& field : fields) {
            if (!entry.contains(field) || (!entry[field].is_string() && !entry[field].is_number())) continue;
            const std::string value = entry[field].is_string() ? entry[field].get<std::string>() : entry[field].dump();
            if (value.empty()) continue;
            if (!hasFacts) output += "- ";
            std::string label = field;
            std::replace(label.begin(), label.end(), '_', ' ');
            output += label + ": " + value + "; ";
            hasFacts = true;
        }
        if (hasFacts) output += "\n";
    };

    std::string normalizedQuestion = question;
    std::transform(normalizedQuestion.begin(), normalizedQuestion.end(), normalizedQuestion.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    static const std::vector<std::string> profileTerms = {
        "name", "aditya", "age", "born", "birth", "contact", "email", "phone", "who is he", "who am i"
    };
    const bool asksAboutProfile = std::any_of(profileTerms.begin(), profileTerms.end(), [&](const std::string& term) {
        return normalizedQuestion.find(term) != std::string::npos;
    });
    std::string context = rankedSections.empty() && asksAboutProfile ? profileContext : std::string{};
    const size_t maxContextSize = 200;
    int selectedSections = 0;
    for (const auto& ranked : rankedSections) {
            if (selectedSections == 1 || context.size() >= maxContextSize) break;
        std::string section = "Section: " + ranked.second.value("title", std::string("Untitled")) + "\n";
        const auto entries = ranked.second.value("data", nlohmann::json::array());
        if (entries.is_array()) {
            for (const auto& entry : entries) appendEntry(section, entry);
        }
        const auto subsections = ranked.second.value("sub_sections", nlohmann::json::array());
        if (subsections.is_array()) {
            for (const auto& subsection : subsections) {
                section += "Subsection: " + subsection.value("title", std::string("Untitled")) + "\n";
                const auto subsectionEntries = subsection.value("data", nlohmann::json::array());
                if (subsectionEntries.is_array()) {
                    for (const auto& entry : subsectionEntries) appendEntry(section, entry);
                }
            }
        }
        const size_t available = maxContextSize - context.size();
        if (section.size() > available) {
            size_t end = available;
            while (end > 0 && end < section.size() &&
                   (static_cast<unsigned char>(section[end]) & 0xC0) == 0x80) {
                --end;
            }
            section.resize(end);
        }
        context += "\n" + section;
        ++selectedSections;
    }
    return context;
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
    svr.Get("/api/activity/:platform", [](const httplib::Request& req, httplib::Response& res) {
        const auto platform = req.path_params.at("platform");
        httplib::Client client(platform == "leetcode" ? "https://leetcode.com" : "https://codeforces.com");
        client.set_connection_timeout(5);
        client.set_read_timeout(10);
        nlohmann::json output = nlohmann::json::array();

        if (platform == "leetcode") {
            httplib::Client leetcode("https://leetcode-api-pied.vercel.app");
            leetcode.set_connection_timeout(5);
            leetcode.set_read_timeout(10);
            auto response = leetcode.Get("/user/AdityaAmanAir/calendar");
            if (response && response->status == 200) {
                try {
                    auto data = nlohmann::json::parse(response->body);
                    const auto& calendar = data.at("submissionCalendar");
                    if (calendar.is_object()) {
                        for (const auto& entry : calendar.items()) {
                            output.push_back({
                                {"timestamp", std::stoll(entry.key())},
                                {"count", entry.value()}
                            });
                        }
                    }
                } catch (...) {
                    output = nlohmann::json::array();
                }
            }
        } else {
            auto response = client.Get("/api/user/info?handle=AdityaAmanAir");
            if (response && response->status == 200) {
                try {
                    auto data = nlohmann::json::parse(response->body);
                    std::map<std::string, int> daily;
                    const auto submissions = data.value("result", nlohmann::json::array());
                    if (submissions.is_array()) {
                        for (const auto& submission : submissions) {
                            const auto timestamp = submission.value("creationTimeSeconds", 0L);
                            std::time_t time = static_cast<std::time_t>(timestamp);
                            std::tm date{};
                            gmtime_r(&time, &date);
                            char key[11];
                            std::strftime(key, sizeof(key), "%Y-%m-%d", &date);
                            daily[key]++;
                        }
                    }
                    for (const auto& entry : daily) output.push_back({{"date", entry.first}, {"count", entry.second}});
                } catch (...) {
                    output = nlohmann::json::array();
                }
            }
        }

        if (output.empty()) {
            res.status = 502;
            res.set_content("{\"error\":\"Activity data unavailable\"}", "application/json");
            return;
        }
        res.set_content(output.dump(), "application/json");
    });

    svr.Get("/api/github/contributions", [](const httplib::Request&, httplib::Response& res) {
        httplib::Client client("https://github.com");
        client.set_follow_location(true);
        client.set_connection_timeout(5);
        client.set_read_timeout(10);
        client.set_default_headers({{"User-Agent", "AA-Portfolio/1.0"}});

        auto graph = client.Get("/users/AdityaAmanAir/contributions");
        if (!graph || graph->status != 200) {
            res.status = 502;
            res.set_content("GitHub contribution graph unavailable", "text/plain");
            return;
        }

        const auto table_start = graph->body.find("<table");
        const auto table_end = graph->body.find("</table>", table_start);
        if (table_start == std::string::npos || table_end == std::string::npos) {
            res.status = 502;
            res.set_content("GitHub contribution graph unavailable", "text/plain");
            return;
        }

        res.set_content(graph->body.substr(table_start, table_end - table_start + 8), "text/html");
        res.set_header("Cache-Control", "public, max-age=900");
    });

    svr.Options("/api/(.*)", [](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        res.status = 204;
    });

    svr.Get("/api/data", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        if (!isAuthorized(req)) {
            res.status = 403;
            res.set_content("{\"error\":\"Forbidden\", \"code\":\"403-AUTH-01\"}", "application/json");
            return;
        }
        res.set_content(dataService_.getData().dump(), "application/json");
    });

    svr.Get("/api/stats", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);

        auto fullData = dataService_.getData();
        nlohmann::json handles = nlohmann::json::object();

        if (fullData.contains("profile") && fullData["profile"].contains("social")) {
            for (const auto& s : fullData["profile"]["social"]) {
                std::string platform = s.value("platform", "");
                std::string url = s.value("url", "");
                if (platform.empty() || url.empty()) continue;

                while (!url.empty() && url.back() == '/') url.pop_back();
                const auto slash = url.find_last_of('/');
                std::string username = slash == std::string::npos ? url : url.substr(slash + 1);
                std::transform(platform.begin(), platform.end(), platform.begin(), ::tolower);
                handles[platform] = username;
            }
        }

        res.set_content(statsService_.getAllStats(handles).dump(), "application/json");
    });

    svr.Get("/api/section/:id", [this](const httplib::Request& req, httplib::Response& res) {
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

    svr.Post("/api/ai/chat", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(req, res);
        try {
            const auto body = nlohmann::json::parse(req.body);
            const std::string question = body.value("question", std::string{});
            if (question.find_first_not_of(" \t\r\n") == std::string::npos || question.size() > 500) {
                res.status = 400;
                res.set_content("{\"error\":\"Enter a question up to 500 characters.\"}", "application/json");
                return;
            }

            const std::string context = getRelevantPortfolioData(dataService_.getData(), question);
            const std::string prompt = context.empty()
                ? question
                : "Site facts: " + context + "\nQuestion: " + question + "\nAnswer briefly.";
            nlohmann::json requestBody = {
                {"model", "google_gemma-3-1b-it-qat-IQ4_XS"},
                {"messages", nlohmann::json::array({
                    {{"role", "user"}, {"content", prompt}}
                })},
                {"temperature", 0.2},
                {"max_tokens", 96},
                {"stream", false}
            };

            httplib::Client modelClient("127.0.0.1", 8081);
            modelClient.set_connection_timeout(2);
            modelClient.set_read_timeout(120);
            const auto modelResponse = modelClient.Post("/v1/chat/completions", requestBody.dump(), "application/json");
            if (!modelResponse || modelResponse->status != 200) {
                res.status = 503;
                res.set_content("{\"error\":\"The local AI model is unavailable.\"}", "application/json");
                return;
            }

            const auto answerData = nlohmann::json::parse(modelResponse->body);
            const std::string answer = answerData.at("choices").at(0).at("message").at("content").get<std::string>();
            if (answer.find_first_not_of(" \t\r\n") == std::string::npos) {
                res.set_content(nlohmann::json{{"answer", "I couldn't generate a reply. Please rephrase and try again."}}.dump(), "application/json");
                return;
            }
            res.set_content(nlohmann::json{{"answer", answer}}.dump(), "application/json");
        } catch (const std::exception&) {
            res.status = 400;
            res.set_content("{\"error\":\"Invalid chat request.\"}", "application/json");
        }
    });

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
        } catch (const std::exception&) {
            res.status = 400;
            res.set_content("{\"error\":\"Invalid request body\"}", "application/json");
        }
    });

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
        } catch (const std::exception&) {
            res.status = 400;
            res.set_content("{\"error\":\"Invalid request body\"}", "application/json");
        }
    });
}

} // namespace controller
} // namespace portfolio
