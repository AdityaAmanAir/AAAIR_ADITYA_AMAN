#pragma once
#include <string>
#include <vector>
#include <json.hpp>

namespace portfolio {
namespace model {

using json = nlohmann::json;

struct Skill {
    std::string name;
};

struct Education {
    std::string institution;
    std::string degree;
    std::string period;
    std::string roll;
    std::vector<std::string> skills;
};

struct Certificate {
    std::string title;
    std::string issuer;
};

struct Award {
    std::string title;
    std::string description;
};

} // namespace model
} // namespace portfolio
