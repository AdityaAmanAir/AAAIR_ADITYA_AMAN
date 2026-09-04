#pragma once
#include <httplib.h>
#include <portfolio/service/PortfolioService.hpp>

namespace portfolio {
namespace controller {

class PortfolioController {
public:
    explicit PortfolioController(const service::PortfolioService& service);
    void registerRoutes(httplib::Server& svr) const;

private:
    const service::PortfolioService& service_;
    static void setCorsHeaders(const httplib::Request& req, httplib::Response& res);
};

} // namespace controller
} // namespace portfolio
