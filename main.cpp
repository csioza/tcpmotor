#include "tcpmotor.h"

//server
int main()
{

    dcore::TcpMotor *server = new dcore::TcpMotor(11111);
    dcore::TcpRecvHandler * handler = new dcore::TcpRecvHandler();
    server->SetRecvHandler(handler);
    server->Run();


    dcore::TcpMotor *client = new dcore::TcpMotor(11112);
    client->Run();

    usleep(1000000);
    for (int i = 0; i < 10000000; ++i)
    {
		std::string data = "hellohellohellohellohellohellohellohellohellohellohellohellohellohellohellohelloh";
		client->Send("172.17.0.3", 11111, (char*)data.data(), data.size(), 0);
		//usleep(1);
    }

    while(true)
        usleep(10000000);

    return 0;
}