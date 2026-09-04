#include <portfolio/service/GenericDataService.hpp>
#include <fstream>
#include <iostream>

namespace portfolio {
namespace service {

GenericDataService::GenericDataService(const std::string& data_path) {
    load(data_path);
}

void GenericDataService::load(const std::string& data_path) {
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        data_path_ = data_path;
    }
    if (!refreshData()) {
        std::cerr << "[GenericDataService] Warning: Could not load data from " << data_path << std::endl;
        std::lock_guard<std::mutex> lock(data_mutex_);
        cached_data_ = nlohmann::json::object();
    }
}

nlohmann::json GenericDataService::getData() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    return cached_data_;
}

nlohmann::json GenericDataService::getSection(const std::string& section_id) const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    if (cached_data_.contains("sections") && cached_data_["sections"].is_array()) {
        for (const auto& section : cached_data_["sections"]) {
            if (section.contains("id") && section["id"] == section_id) {
                return section;
            }
        }
    }
    return nlohmann::json::object();
}

bool GenericDataService::refreshData() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    std::ifstream file(data_path_);
    if (!file.is_open()) {
        return false;
    }
    try {
        file >> cached_data_;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[GenericDataService] JSON Parse Error: " << e.what() << std::endl;
        return false;
    }
}

} // namespace service
} // namespace portfolio
