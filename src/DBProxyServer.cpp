#include <iostream>
#include <unordered_map>

#include "socketlibfunction.h"
#include "ox_file.h"
#include "WrapLog.h"
#include "lua_readtable.h"
#include "Backend.h"
#include "Client.h"
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "luaconf.h"
};
#include "lua_tinker.h"

using namespace std;

WrapLog::PTR gDailyLogger;
struct lua_State* L = nullptr;
string sharding_function;

bool sharding_key(const char* str, int len, int& serverID)
{
    serverID = lua_tinker::call<int>(L, sharding_function.c_str(), string(str, len));   /*ʹ��string����strû�н�����*/
    return true;
}

int main()
{
    int listenPort;         /*����������ļ����˿�*/
    
    std::vector<std::tuple<int, string, int>> backendConfigs;
    {
        struct msvalue_s config(true);
        L = luaL_newstate();
        luaopen_base(L);
        luaL_openlibs(L);
        /*TODO::����������ָ������·��*/
        lua_tinker::dofile(L, "Config.lua");
        aux_readluatable_byname(L, "ProxyConfig", &config);

        map<string, msvalue_s*>& allconfig = *config._map;
        listenPort = atoi(allconfig["listenPort"]->_str.c_str());
        sharding_function = allconfig["sharding_function"]->_str;

        map<string, msvalue_s*>& backends = *allconfig["backends"]->_map;

        for (auto& v : backends)
        {
            map<string, msvalue_s*>& oneBackend = *(v.second)->_map;
            int id = atoi(oneBackend["id"]->_str.c_str());
            string dbServerIP = oneBackend["ip"]->_str;
            int port = atoi(oneBackend["port"]->_str.c_str());
            backendConfigs.push_back(std::make_tuple(id, dbServerIP, port));
        }
    }

    gDailyLogger = std::make_shared<WrapLog>();

    spdlog::set_level(spdlog::level::info);

    ox_dir_create("logs");
    ox_dir_create("logs/DBProxyServer");
    gDailyLogger->setFile("", "logs/DBProxyServer/daily");

    EventLoop mainLoop;

    WrapServer::PTR server = std::make_shared<WrapServer>();

    /*���������߳�*/
    server->startWorkThread(1, [&](EventLoop&){
        syncNet2LogicMsgList(mainLoop);
    });

    /*�������ݿ������*/
    for (auto& v : backendConfigs)
    {
        int id = std::get<0>(v);
        string ip = std::get<1>(v);
        int port = std::get<2>(v);

        gDailyLogger->info("connec db server id:{}, address: {}:{}", id, ip, port);
        sock fd = ox_socket_connect(ip.c_str(), port);
        auto bserver = std::make_shared<BackendLogicSession>();
        bserver->setID(id);
        WrapAddNetSession(server, fd, make_shared<BackendExtNetSession>(bserver), -1);
    }

    gDailyLogger->info("listen proxy port:{}", listenPort);
    /*�����������������*/
    server->getListenThread().startListen(listenPort, nullptr, nullptr, [&](int fd){
        WrapAddNetSession(server, fd, make_shared<ClientExtNetSession>(std::make_shared<ClientLogicSession>()), -1);
    });

    gDailyLogger->warn("db proxy server start!");

    while (true)
    {
        mainLoop.loop(1);
        /*  ���������߳�Ͷ�ݹ�������Ϣ */
        procNet2LogicMsgList();
        server->getService()->flushCachePackectList();
    }

    std::cin.get();

    lua_close(L);
    L = nullptr;

    return 0;
}