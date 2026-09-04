#pragma once
#include <httplib.h>
#include <portfolio/controller/ApiController.hpp>
#include <portfolio/service/GenericDataService.hpp>
#include <portfolio/core/SecurityMiddleware.hpp>
#include <thread>

namespace portfolio {
namespace core {

class Server {
public:
    Server(const std::string& host, int port, const std::string& data_path);
    void start();

private:
    std::string host_;
    int port_;
    std::string data_path_;
    httplib::Server svr_;
    service::GenericDataService data_service_;
    service::StatsService stats_service_;
    int thread_count_;
};

} // namespace core
} // namespace portfolio
