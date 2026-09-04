#pragma once
#include <json.hpp>
#include <string>
#include <mutex>

namespace portfolio {
namespace service {

class GenericDataService {
public:
    GenericDataService() = default;
    explicit GenericDataService(const std::string& data_path);
    
    void load(const std::string& data_path);
    nlohmann::json getData() const;
    nlohmann::json getSection(const std::string& section_id) const;
    bool refreshData();

private:
    std::string data_path_;
    nlohmann::json cached_data_;
    mutable std::mutex data_mutex_;
};

} // namespace service
} // namespace portfolio
