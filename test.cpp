#include "tcpmotor.h"

//server
int main()
{
    dcore::TcpMotor *server = new dcore::TcpMotor(11111);
    dcore::TcpRecvHandler * handler = new dcore::TcpRecvHandler();
    server->SetRecvHandler(handler);
    server->Run();

    dcore::TcpMotor *client1 = new dcore::TcpMotor(11112);
    client1->Run();
    dcore::TcpMotor *client2 = new dcore::TcpMotor(11113);
    client2->Run();
    dcore::TcpMotor *client3 = new dcore::TcpMotor(11114);
    client3->Run();
    dcore::TcpMotor *client4 = new dcore::TcpMotor(11115);
    client4->Run();
    dcore::TcpMotor *client5 = new dcore::TcpMotor(11116);
    client5->Run();

    usleep(1000000);
    for (int i = 0; i < 10000000; ++i)
    {
		std::string data1 = std::to_string(dcore::TimeUtil::NowTimeUs()); //"hellohellohellohellohellohellohellohellohellohellohellohellohellohellohellohelloh";
		client1->Send("172.17.0.3", 11111, (char*)data1.data(), data1.size(), 0);
		std::string data2 = std::to_string(dcore::TimeUtil::NowTimeUs());
		client2->Send("172.17.0.3", 11111, (char*)data2.data(), data2.size(), 0);
		std::string data3 = std::to_string(dcore::TimeUtil::NowTimeUs());
		client3->Send("172.17.0.3", 11111, (char*)data3.data(), data3.size(), 0);
		std::string data4 = std::to_string(dcore::TimeUtil::NowTimeUs());
		client4->Send("172.17.0.3", 11111, (char*)data4.data(), data4.size(), 0);
		std::string data5 = std::to_string(dcore::TimeUtil::NowTimeUs());
		client5->Send("172.17.0.3", 11111, (char*)data5.data(), data5.size(), 0);
		usleep(500);
    }

    while(true)
        usleep(10000000);

    return 0;
}