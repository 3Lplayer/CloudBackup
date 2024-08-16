#include "file_util.hpp"
#include "conf.hpp"
#include "data.hpp"
#include "hotspot.hpp"
#include "server.hpp"
#include <thread>

cloud::DataManager dm;

//非热点文件管理
void Hotspot()
{
    cloud::Hotspot hot;
    hot.Handle();
}

//服务端
void Service()
{
    cloud::Service server;
    server.Handle();
}

int main(int argc, char *argv[])
{
    std::thread service_t(Service);
    std::thread hotspot_t(Hotspot);

    service_t.join();
    hotspot_t.join();
    return 0;
}