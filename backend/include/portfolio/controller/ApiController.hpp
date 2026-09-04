#include <portfolio/service/GenericDataService.hpp>
#include <portfolio/service/StatsService.hpp>

namespace portfolio {
namespace controller {

class ApiController {
public:
    ApiController(const service::GenericDataService& dataService, const service::StatsService& statsService);
    void registerRoutes(httplib::Server& svr) const;

private:
    const service::GenericDataService& dataService_;
    const service::StatsService& statsService_;
    
    static void setCorsHeaders(const httplib::Request& req, httplib::Response& res);
};

} // namespace controller
} // namespace portfolio
