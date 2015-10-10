#include "platform.h"

#ifdef PLATFORM_WINDOWS
#include <vld.h>
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <iostream>
#include <unordered_map>

#include "SocketLibFunction.h"
#include "ox_file.h"
#include "WrapLog.h"
#include "Backend.h"
#include "Client.h"

using namespace std;

string sharding_function;

bool sharding_key(const char* str, int len, int& serverID)
{
    serverID = 0;//lua_tinker::call<int>(L, sharding_function.c_str(), string(str, len));   /*ʹ��string����strû�н�����*/
    return true;
}

#if defined(_MSC_VER)
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

int     app_kbhit(void)
{
#if defined(_MSC_VER)
    return _kbhit();
#else
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
#endif
}

int main()
{
    {
        int listenPort;         /*����������ļ����˿�*/
        ox_socket_init();
        std::vector<std::tuple<int, string, int>> backendConfigs;
        {
            listenPort = 9999;

            backendConfigs.push_back(std::make_tuple(0, "127.0.0.1", 6379));
        }

        //WrapLog::PTR gDailyLogger = std::make_shared<WrapLog>();

        spdlog::set_level(spdlog::level::info);

        ox_dir_create("logs");
        ox_dir_create("logs/DBProxyServer");
        //gDailyLogger->setFile("", "logs/DBProxyServer/daily");

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

            //gDailyLogger->info("connec db server id:{}, address: {}:{}", id, ip, port);
            sock fd = ox_socket_connect(ip.c_str(), port);
            auto bserver = std::make_shared<BackendLogicSession>();
            bserver->setID(id);
            WrapAddNetSession(server, fd, make_shared<BackendExtNetSession>(bserver), -1);
        }

       // gDailyLogger->info("listen proxy port:{}", listenPort);
        /*�����������������*/
        server->getListenThread().startListen(listenPort, nullptr, nullptr, [&](int fd){
            WrapAddNetSession(server, fd, make_shared<ClientExtNetSession>(std::make_shared<ClientLogicSession>()), -1);
        });

        //gDailyLogger->warn("db proxy server start!");

        while (true)
        {
            if (app_kbhit())
            {
                break;
            }

            mainLoop.loop(1);
            /*  ���������߳�Ͷ�ݹ�������Ϣ */
            procNet2LogicMsgList();
            server->getService()->flushCachePackectList();
        }
        server->getService()->closeService();
        syncNet2LogicMsgList(mainLoop);
        procNet2LogicMsgList();
    }

    return 0;
}
