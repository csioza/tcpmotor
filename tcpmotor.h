#pragma once
//c++
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream> 
#include <string>
#include <map>
#include <vector>
#include <stdbool.h>
#include <cstring>
#include <atomic>
#include <thread>
//linux
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
//
#include "concurrentqueue.h"

namespace dcore {

#define MAX_RECBUFF_SIZE    8192
#define MAX_PACKET_SIZE     8192
#define INVALID             -1 
#define MAX_EVENT_NUM       256
#define INVALID_SOCKET      -1

typedef char                int8;
typedef short               int16;
typedef int                 int32;
typedef long long           int64;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

typedef void (*callback)(void*);

#pragma pack(1)
struct packet  //用于线程的收发队列里
{
    uint16 len;//64k
    char data[0];
};
#pragma pack()
///////////////////////////////////////////////////////////////////////////////
//handler class
///////////////////////////////////////////////////////////////////////////////
class TcpRecvHandler
{
public:
    TcpRecvHandler();
    ~TcpRecvHandler();
    virtual void OnRecv(std::string callback_ip, int callback_port, char* content, int contentLen) {}
};
class TcpSendHandler
{
public:
    TcpSendHandler();
    ~TcpSendHandler();
    virtual void OnSuccess() {}
    virtual void OnFailure() {}
};
///////////////////////////////////////////////////////////////////////////////
//socket util
///////////////////////////////////////////////////////////////////////////////
class SocketUtil
{
public:
    static std::string MakeKeyByIpPort(const std::string &ip, int port)
    {
        return ip + ":" + std::to_string(port);
    }
    static int CreateBind(const std::string &ip, int port)
    {
        int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sfd == INVALID_SOCKET)
        {
            printf("invalid socket !");
            return sfd;
        }
        //绑定端口 
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        //serverAddr.sin_addr.s_addr = inet_addr(ip);
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_port = htons(port);
        //绑定  
        int iErrorMsg = bind(sfd, (sockaddr*)&serverAddr, sizeof(serverAddr));  
        if (iErrorMsg < 0)  
        {  
            //绑定失败  
            printf("bind failed with error : %d", iErrorMsg); 
            return -2;  
        }
        printf("bind success: port[%d], fd[%d]", port, sfd);
        return sfd;
    }
    static int Nonblock(int sfd)
    {
        int flags;
        //得到文件状态标志
        flags = fcntl (sfd, F_GETFL, 0);
        if (flags == INVALID)
        {
            printf("fcntl");
            return INVALID;
        }
        //设置文件状态标志
        flags |= O_NONBLOCK;
        if (fcntl (sfd, F_SETFL, flags) == INVALID)
        {
            printf("fcntl");
            return INVALID;
        }
        return 0;
    }
    static int Connect(int sfd, const std::string &ip, int port)
    {
        struct sockaddr_in server_addr; 
        memset(&server_addr,0,sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        server_addr.sin_port = htons(port);

        if(connect(sfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
        {
            printf("connect error");
            return -2;
        }
        if (Nonblock(sfd) < 0)
        {
            printf("nonblock error !");
            return -3;
        }
        //printf("connected to server(ip=%s, port=%d, fd=%d)", ip, port, sfd);
        return sfd;
    }
    static int CreateBindListen(const std::string &ip, int port, bool isBlock/* = false*/)
    {
        int sfd = CreateBind(ip, port);
        if (!isBlock && Nonblock(sfd) == INVALID)
        {
            printf("nonblock");
            abort();
        }
        if (listen(sfd, SOMAXCONN) == INVALID)
        {
            printf("listen");
            abort();
        }
        return sfd;
    }
    static int Setsockopt(int sfd)
    {
        int rcvBuff = 524288;//512k
        if (0 != setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, (const char *)&rcvBuff, sizeof(rcvBuff)))
        {
            printf("set rcvBuff failed!");
            //closesocket(sfd);
            close(sfd);
            return INVALID;
        }
        int sndBuff = 524288;//512k
        if (0 != setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, (const char *)&sndBuff, sizeof(sndBuff)))
        {
            printf("set sndBuff failed!");
            //closesocket(sfd);
            close(sfd);
            return INVALID;
        }
        int nOpt = 1;
        if (0 != setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (const char *)&nOpt, sizeof(nOpt)))
        {
            printf("set tcp_nodelay failed!");
            //closesocket(sfd);
            close(sfd);
            return INVALID;
        }
        struct linger s_linger;
        s_linger.l_onoff = 1;
        s_linger.l_linger = 0;
        if (0 != setsockopt(sfd, SOL_SOCKET, SO_LINGER, (const char *)&s_linger, sizeof(s_linger)))
        {
            printf("set linger failed!");
            //closesocket(sfd);
            close(sfd);
            return INVALID;
        }
        ////如果在发送数据的时，希望不经历由系统缓冲区到socket缓冲区的拷贝而影响 
        //int nZero = 0;
        //setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
        ////同上在recv()完成上述功能(默认情况是将socket缓冲区的内容拷贝到系统缓冲区) 
        //nZero = 0;
        //setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, (char *)&nZero, sizeof(nZero));
        return 0;
    }
    
};
///////////////////////////////////////////////////////////////////////////////
//main class
///////////////////////////////////////////////////////////////////////////////
class TcpMotor;

class Link
{
public:
    Link() : mFd(0), mHead(0), mEnd(0), mMotor(nullptr)
    {
        memset(mBuff, 0, MAX_RECBUFF_SIZE);
    }
    virtual ~Link() {}
    virtual void OnRecv() {}
    //   
    int         mFd;
    int         mHead;
    int         mEnd;
    char        mBuff[MAX_RECBUFF_SIZE];
    std::string mIp;//远端ip
    int         mPort;//远端port
    TcpMotor*   mMotor;
};

class SocketLink : public Link
{
public:
    SocketLink() {}
    virtual ~SocketLink() {}
    virtual void OnRecv();
};

class AcceptLink : public Link
{
public:
    AcceptLink() {}
    virtual ~AcceptLink() {}
    virtual void OnRecv();
};

class TcpMotor
{
public:
    TcpMotor(int port) : mPort(port), mIsRunning(false)
    {
        mEpollFd        = epoll_create(256);
        mEvents         = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAX_EVENT_NUM);
    }
    ~TcpMotor()
    {
        delete mEvents;
    }
    //
    void Start()
    {
        mThread = std::thread(&TcpMotor::Run, this);
    }
    void Run()
    {
        mIsRunning      = true;
        Link *link      = new AcceptLink();
        link->mFd       = SocketUtil::CreateBindListen("", mPort, false);
        link->mMotor    = this;
        AddLink(link);
        Loop();
    }
    void Stop()
    {
        mIsRunning = false;
        if (mThread.joinable()) {
            mThread.join();
        }
    }
    void Loop()
    {
        while (mIsRunning)
        {
            int cnt = Wait(0);
            for (int i = 0; i < cnt; ++i)
            {
                if (mEvents[i].events & EPOLLIN)
                {
                    Link *link = (Link *)mEvents[i].data.ptr;
                    if (link)
                        link->OnRecv();
                }
            }
            //TODO 发送数据
        }
    }
    //
    void SetRecvHandler(TcpRecvHandler* recv) { mRecvHandler = recv; }
    void SetSendHandler(TcpSendHandler* send) { mSendHandler = send; }
    void RecvHandler(std::string callback_ip, int callback_port, void* content, int contentLen)
    {
        mRecvHandler->OnRecv(callback_ip, callback_port, (char *)content, contentLen); 
    }
    //
    void Send(std::string ip, int port, void* data, int len, void* ctx)
    {

    }
    //
    int AddLink(Link *link)
    {
        std::string key = SocketUtil::MakeKeyByIpPort(link->mIp, link->mPort);
        auto it = mIpPortLink.find(key);
        if (it == mIpPortLink.end())
            mIpPortLink[key] = link;
        struct epoll_event event;
        event.events    = EPOLLIN | EPOLLET;
        event.data.fd   = link->mFd;
        event.data.ptr  = (void *)link;
        return epoll_ctl(mEpollFd, EPOLL_CTL_ADD, link->mFd, &event);
    }
    int DelLink(Link *link)
    {
        close(link->mFd);
        return epoll_ctl(mEpollFd, EPOLL_CTL_DEL, link->mFd, NULL);
    }
    int ModLink(Link *link, bool isWrite)
    {
        struct epoll_event event;
        event.events    = EPOLLIN | (isWrite ? EPOLLOUT : 0);
        event.data.ptr  = (void *)link;
        return epoll_ctl(mEpollFd, EPOLL_CTL_MOD, link->mFd, &event);
    }
    int Wait(int timeout)
    {
        return epoll_wait(mEpollFd, mEvents, MAX_EVENT_NUM, timeout);
    }
private:
    int                     mPort;
    bool                    mIsRunning;
    TcpRecvHandler*         mRecvHandler;
    TcpSendHandler*         mSendHandler;
    std::map<std::string, Link*> mIpPortLink;//key: ip:port
    //
    int                     mEpollFd;
    struct epoll_event      *mEvents;
    //
    std::thread             mThread;
    //
    moodycamel::ConcurrentQueue<packet*> mRecvQueue;
    moodycamel::ConcurrentQueue<packet*> mSendQueue;
};

void SocketLink::OnRecv()
{
    int done = 0;
    while (1)
    {
        int count = recv(mFd, mBuff + mEnd, MAX_RECBUFF_SIZE - mEnd, 0);
        if (count == -1)
        {
            if (errno == 0 || errno == EAGAIN)
                done = 1;//go back to the main loop.
            else
            {
                printf("err recv count:%d errno:%d",count,errno);
                done = 2;//close the cocket.
            }
        }
        else if (count == 0)
            done = 1;//go back to the main loop.
        else
        {
            mEnd += count;
            while (mHead < mEnd)
            {
                if (mHead + sizeof(packet) < MAX_RECBUFF_SIZE)
                {
                    packet *node = (packet *)(mBuff + mHead);
                    if (node && node->len < MAX_PACKET_SIZE)
                    {
                        if (mHead + node->len > mEnd) // 断包 
                        {
                            if (mHead + node->len > MAX_RECBUFF_SIZE)
                            {
                                mEnd -= mHead;
                                memcpy(mBuff, mBuff + mHead, mEnd);
                                mHead = 0;
                            }
                            break;// Goto while(1) 
                        }
                        else
                        {
                            // 消费
                            mMotor->RecvHandler(mIp, mPort, node, node->len);
                            mHead += node->len;
                        }
                    }
                    else //错误包 
                    {
                        mHead = 0;
                        mEnd  = 0;
                        done  = 2;
                        break;// Goto while(1) 
                    }
                }
                else
                {
                    mEnd -= mHead;
                    memcpy(mBuff, mBuff + mHead, mEnd); //
                    mHead = 0;
                    break;// Goto while(1) 
                }
            }
        }
        //
        if (mEnd == MAX_RECBUFF_SIZE)
        {
            if (mHead < MAX_RECBUFF_SIZE)
            {
                mEnd -= mHead;
                memcpy(mBuff, mBuff + mHead, mEnd); //
                mHead = 0;
            }
            else if (mHead == MAX_RECBUFF_SIZE)
            {
                mHead = 0;
                mEnd  = 0;
            }
            else
                done = 2;
        }
        //
        if (done == 1)
            break;//Go back to the main loop.
        else if (done == 2)
        {
            mMotor->DelLink(this);
            break;
        }
    }
}

void AcceptLink::OnRecv()
{
    while (1)
    {
        struct sockaddr_in addr;
        socklen_t in_len    = sizeof(struct sockaddr_in);
        int socket_fd       = accept(mFd, (struct sockaddr *)&addr, &in_len);
        if (socket_fd == INVALID_SOCKET)
        {
            // if (errno == EAGAIN || errno == EWOULDBLOCK)
            printf("accept");
            break;
        }
        SocketUtil::Setsockopt(socket_fd);
        char ip[NI_MAXHOST], port[NI_MAXSERV];
        if (getnameinfo((struct sockaddr *)&addr, in_len, ip, sizeof ip, port, sizeof port, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
        {
            std::cout << "Accepted connection socket_nonblock (host=" << ip << ", port=" << port << ", socket_fd=" << std::to_string(socket_fd) << ")" << std::endl;
            continue;
        }
        std::cout << "Accepted connection socket_nonblock (host=" << ip << ", port=" << port << ", socket_fd=" << std::to_string(socket_fd) << ")" << std::endl;    
        if (SocketUtil::Nonblock(socket_fd) < 0)
            continue;
        //
        Link *new_link      = new SocketLink();
        new_link->mFd       = socket_fd;
        new_link->mMotor    = mMotor;
        new_link->mIp       = ip;
        new_link->mPort     = atoi(port);
        mMotor->AddLink(new_link);
    }
}

// void OnSend(void *vp)
// {
//     Service *service = (Service *)vp;
//     if (!service)
//         return;
//     static char * node = (char *)malloc(MAX_PACKET_SIZE);
//     if (!node || !service->send_ring->TryGetNode(node) == 0)
//         return;
//     if (service->send_ring->TryGetNode(node) == 0)
//     {
//         netnode *snode = (netnode *)node;
//         Link *link = (Link *)(snode->link);
//         if (!snode || !link)
//             return;
//         int want_len = snode->len;
//         int gone_len = 0;
//         while (want_len > 0)
//         {
//             int n = send(link->fd, node + SIZE_OF_POINT + gone_len, want_len, MSG_NOSIGNAL);
//             if (n >= 0)
//             {
//                 gone_len += n;
//                 want_len -= n;
//                 if (want_len > 0)
//                 {
//                     packet *pack = (packet *)(node + SIZE_OF_POINT);
//                     if (pack)
//                         printf("Sent Half Packet: id[%6d], gid[%3d], gone_len[%2d], want_len[%2d], n[%2d]", pack->id, pack->gid, gone_len, want_len, n);
//                     sleep(1);
//                 }
//             }
//             else
//             {
//                 packet *pack = (packet *)(node + SIZE_OF_POINT);
//                 if (pack)
//                     printf("Sent Packet: id[%6d], gid[%3d], gone_len[%2d], want_len[%2d], n[%2d]", pack->id, pack->gid, gone_len, want_len, n);
//                 close_cnt(link);
//                 break;
//             }
//         }
//     }
// }

} //namespace dcore
