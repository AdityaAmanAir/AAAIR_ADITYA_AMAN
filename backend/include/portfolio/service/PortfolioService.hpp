#pragma once
#include <portfolio/model/PortfolioData.hpp>
#include <json.hpp>

namespace portfolio {
namespace service {

class PortfolioService {
public:
    nlohmann::json getProfile() const;
    nlohmann::json getEducation() const;
    nlohmann::json getCertificates() const;
    nlohmann::json getAwards() const;
};

} // namespace service
} // namespace portfolio
