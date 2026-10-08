#pragma once
#include <httplib.h>
#include <portfolio/controller/ApiController.hpp>
#include <portfolio/service/GenericDataService.hpp>
#include <portfolio/core/SecurityMiddleware.hpp>
#include <memory>
#include <thread>

namespace portfolio {
namespace core {

class Server {
public:
        Server(const std::string& host, int port, const std::string& data_path,
            const std::string& certificate_path = {}, const std::string& private_key_path = {});
    bool start();

private:
    std::string host_;
    int port_;
    std::string data_path_;
    std::string certificate_path_;
    std::string private_key_path_;
    std::unique_ptr<httplib::Server> svr_;
    service::GenericDataService data_service_;
    service::StatsService stats_service_;
    int thread_count_;
};

} // namespace core
} // namespace portfolio
