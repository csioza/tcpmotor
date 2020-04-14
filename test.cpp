#include "tcpmotor.h"

int main()
{
    dcore::TcpMotor *server = new dcore::TcpMotor(11111);
    dcore::TcpRecvHandler * handler = new dcore::TcpRecvHandler();
    server->SetRecvHandler(handler);
    server->Run();

    const int max = 5;
    dcore::TcpMotor* client[max];
    for (int i = 0; i < max; ++i)
    {
        client[i] = new dcore::TcpMotor(11112 + i);
        client[i]->Run();
    }

    std::string hostname;
    std::string localip;
    dcore::SocketUtil::GetHostInfo(hostname, localip);
    std::string tail = dcore::RandomString(1000);
    for (int i = 0; i < 10000000; ++i)
    {
        for (int j = 0; j < max; ++j)
        {
            std::string data = std::to_string(dcore::TimeUtil::NowTimeUs()) + tail;
            client[j]->Send(localip, 11111, (char*)data.data(), data.size(), 0);
        }
        usleep(200);
    }

    while(true)
        usleep(10000000);

    return 0;
}