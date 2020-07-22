#include "tcpmotor.h"
#include "gtest/gtest.h"

std::vector<std::string> recv_vec;
class TestTcpRecvHandler : public dcore::TcpRecvHandler
{
public:
    TestTcpRecvHandler() {}
    virtual ~TestTcpRecvHandler() {}
    virtual void OnRecv(std::string callback_ip, int callback_port, char* content, int contentLen) 
    {
        std::string data(content, content + contentLen);
        recv_vec.push_back(data);
    }
};

//=============================================================================
// TEST(TcpMotor, SendOnePacketToSelf) 
// {
//     recv_vec.clear();
//     dcore::TcpMotor *server = new dcore::TcpMotor(11110, 1);
//     dcore::TcpRecvHandler * handler = new TestTcpRecvHandler();
//     server->SetRecvHandler(handler);
//     server->Run();
//     std::string data = dcore::RandomString(100);
//     server->Send(dcore::SocketUtil::GetLocalIp(), 11110, data.c_str(), data.size(), NULL);
//     usleep(10000);
//     EXPECT_EQ(1, recv_vec.size());
//     EXPECT_EQ(data, recv_vec[0]);
//     server->Stop();
//     delete server;
// }
TEST(TcpMotor, SendOnePacket) 
{
    recv_vec.clear();
    dcore::TcpMotor *server = new dcore::TcpMotor(11111, 1);
    dcore::TcpMotor *client = new dcore::TcpMotor(11112, 1);
    dcore::TcpRecvHandler * handler = new TestTcpRecvHandler();
    server->SetRecvHandler(handler);
    server->Run();
    client->Run();
    std::string data = dcore::RandomString(2048);
    client->Send(dcore::SocketUtil::GetLocalIp(), 11111, data.c_str(), data.size(), NULL);
    usleep(100000);
    EXPECT_EQ(1, recv_vec.size());
    EXPECT_EQ(data, recv_vec[0]);
    client->Stop();
    server->Stop();
    delete server;
    delete client;
}
TEST(TcpMotor, SendOnePacket2) 
{
    recv_vec.clear();
    dcore::TcpMotor *server = new dcore::TcpMotor(11113, 2);
    dcore::TcpMotor *client = new dcore::TcpMotor(11114, 2);
    dcore::TcpRecvHandler * handler = new TestTcpRecvHandler();
    server->SetRecvHandler(handler);
    server->Run();
    client->Run();
    std::string data = dcore::RandomString(2048);
    client->Send(dcore::SocketUtil::GetLocalIp(), 11113, data.c_str(), data.size(), NULL);
    usleep(100000);
    EXPECT_EQ(1, recv_vec.size());
    EXPECT_EQ(data, recv_vec[0]);
    client->Stop();
    server->Stop();
    delete server;
    delete client;
}
TEST(TcpMotor, SendOnePacket3) 
{
    recv_vec.clear();
    dcore::TcpMotor *server = new dcore::TcpMotor(11115, 3);
    dcore::TcpMotor *client = new dcore::TcpMotor(11116, 3);
    dcore::TcpRecvHandler * handler = new TestTcpRecvHandler();
    server->SetRecvHandler(handler);
    server->Run();
    client->Run();
    std::string data = dcore::RandomString(2048);
    client->Send(dcore::SocketUtil::GetLocalIp(), 11115, data.c_str(), data.size(), NULL);
    usleep(100000);
    EXPECT_EQ(1, recv_vec.size());
    EXPECT_EQ(data, recv_vec[0]);
    client->Stop();
    server->Stop();
    delete server;
    delete client;
}

//=============================================================================
int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}