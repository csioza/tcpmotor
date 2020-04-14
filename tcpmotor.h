#pragma once
//c++
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream> 
#include <string>
#include <unordered_map>
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
#include <sys/time.h>
//
#include "concurrentqueue.h"

namespace dcore {

#define MAX_PACKET_SIZE     8192
#define MAX_RECBUFF_SIZE    (MAX_PACKET_SIZE * 3)
#define INVALID             -1 
#define MAX_EVENT_NUM       1024
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
struct Packet  //用于线程的收发队列里
{
    uint16 len;//64k, 总包长度，包含len长度
    char data[0];
};
#pragma pack()

class TimeUtil
{
public:
    static uint64 NowTimeMs() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
    static uint64 NowTimeUs() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }
};
std::string RandomString(int len)
{
    char c[len + 1];
    srand(TimeUtil::NowTimeUs());
    for (int i = 0; i < len; i++) {
        switch (rand() % 3) {
        case 1:
            c[i] = 'A' + rand() % 26;
            break;
        case 2:
            c[i] = 'a' + rand() % 26;
            break;
        default:
            c[i] = '0' + rand() % 10;
            break;
        }
    }
    c[len] = '\0';
    std::string result = c;
    return result;
}
///////////////////////////////////////////////////////////////////////////////
//handler class
///////////////////////////////////////////////////////////////////////////////
class TcpRecvHandler
{
public:
    TcpRecvHandler() : sumCount(0), us0_499(0), us500_999(0),
            ms10_19(0), ms20_29(0), ms30_39(0), ms40_49(0),
            ms50_59(0), ms60_69(0), ms70_79(0), ms80_89(0),
            ms90_99(0), ms100X(0), sumSub(0), average(0)
    {
    }
    ~TcpRecvHandler() {}
    virtual void OnRecv(std::string callback_ip, int callback_port, char* content, int contentLen) 
    {
        static int64 last = TimeUtil::NowTimeUs();
        std::string data(content, content + contentLen);
        std::string time(content, content + 16);
        int64 now = TimeUtil::NowTimeUs();
        sumCount++;
        int64 sub = now - stoll(time);
        if (sub < 500)
            us0_499++;
        else if (sub < 1000)
            us500_999++;
        else if (sub < 10000)
            ms1_9++;
        else if (sub < 20000)
            ms10_19++;
        else if (sub < 30000)
            ms20_29++;
        else if (sub < 40000)
            ms30_39++;
        else if (sub < 50000)
            ms40_49++;
        else if (sub < 60000)
            ms50_59++;
        else if (sub < 70000)
            ms60_69++;
        else if (sub < 80000)
            ms70_79++;
        else if (sub < 90000)
            ms80_89++;
        else if (sub < 100000)
            ms90_99++;
        else
            ms100X++;
        sumSub += sub;
        if (sumCount != 0 && sumCount % 10000 == 0)
        {
            average = sumSub / sumCount;
            int64 qps = sumCount * 1000000 / (now - last);
            printf("contentLen[%d],\n qps      [%ld]\n us0_499  [%ld]\n us500_999[%ld]\n ms1_9    [%ld]\n ms10_19  [%ld]\n ms20_29  [%ld]\n ms30_39  [%ld]\n ms40_49  [%ld]\n ms50_59  [%ld]\n ms60_69  [%ld]\n ms70_79  [%ld]\n ms80_89  [%ld]\n ms90_99  [%ld]\n ms100X   [%ld]\n average  [%ld]\n sumCount [%ld]\n" ,
                /*data.c_str(), */contentLen, qps,
                us0_499, us500_999, ms1_9, ms10_19, ms20_29, ms30_39, ms40_49,ms50_59, ms60_69, ms70_79, ms80_89, ms90_99, ms100X,
                average, sumCount);

        }
        // printf("TcpRecvHandler::OnRecv from ip[%s], port[%d], content[%s], contentLen[%d], sub[%d]\n",
        //         callback_ip.c_str(), callback_port, data.c_str(), contentLen, now - stoll(data));
    }
    int64 sumCount;
    int64 us0_499;
    int64 us500_999;
    int64 ms1_9;
    int64 ms10_19;
    int64 ms20_29;
    int64 ms30_39;
    int64 ms40_49;
    int64 ms50_59;
    int64 ms60_69;
    int64 ms70_79;
    int64 ms80_89;
    int64 ms90_99;
    int64 ms100X;
    int64 sumSub;
    int64 average;
};
class TcpSendHandler
{
public:
    TcpSendHandler() {}
    ~TcpSendHandler() {}
    virtual void OnSuccess(std::string ip, int port, void* ctx) 
    {
        printf("TcpSendHandler::OnSuccess to ip[%s], port[%d]\n", ip.c_str(), port);
    }
    virtual void OnFailure(std::string ip, int port, void* ctx) 
    {
        printf("TcpSendHandler::OnFailure to ip[%s], port[%d]\n", ip.c_str(), port);
    }
};
class RecvPacket
{
public:
    RecvPacket(std::string ip, int port, char* data, int len) : mIp(ip), mPort(port)
    {
        mData = std::string(data, data + len);
    }
    ~RecvPacket() {}
    std::string     mData;
    std::string     mIp;
    int             mPort;
};
class SendPacket
{
public:
    SendPacket(std::string ip, int port, void* data, int len, void* ctx) : mIp(ip), mPort(port), mCtx(ctx)
    {
        mLen            = len + sizeof(Packet);
        mData           = (char *)malloc(mLen);
        Packet *packet  = (Packet *)mData;
        packet->len     = mLen;
        memcpy(packet->data, (char *)data, len);
    }
    ~SendPacket()
    {
        delete mData;
    }
    int             mLen;
    char            *mData;
    std::string     mIp;
    int             mPort;
    void            *mCtx;
};
///////////////////////////////////////////////////////////////////////////////
//socket util
///////////////////////////////////////////////////////////////////////////////
class SocketUtil
{
public:
    static bool GetHostInfo(std::string& hostName, std::string& Ip) {
        char name[256];
        gethostname(name, sizeof(name));
        hostName = name;
        struct hostent* host = gethostbyname(name);
        char ipStr[32];
        const char* ret = inet_ntop(host->h_addrtype, host->h_addr_list[0], ipStr, sizeof(ipStr));
        if (NULL == ret) {
            std::cout << "hostname transform to ip failed\n" << std::endl;
            return false;
        }
        Ip = ipStr;
        return true;
    }
    static std::string MakeKeyByIpPort(const std::string &ip, int port)
    {
        return ip + ":" + std::to_string(port);
    }
    static int CreateBind(const std::string &ip, int port)
    {
        int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sfd == INVALID_SOCKET)
        {
            std::cout << "invalid socket !\n" << std::endl;
            return sfd;
        }
        //绑定端口 
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
        serverAddr.sin_port = htons(port);
        //绑定  
        int iErrorMsg = bind(sfd, (sockaddr*)&serverAddr, sizeof(serverAddr));  
        if (iErrorMsg < 0)  
        {  
            //绑定失败  
            printf("bind failed with error : %d\n", iErrorMsg); 
            return -2;  
        }
        return sfd;
    }
    static int Nonblock(int sfd)
    {
        int flags;
        //得到文件状态标志
        flags = fcntl (sfd, F_GETFL, 0);
        if (flags == INVALID)
        {
            printf("fcntl failed\n");
            return INVALID;
        }
        //设置文件状态标志
        flags |= O_NONBLOCK;
        if (fcntl (sfd, F_SETFL, flags) == INVALID)
        {
            printf("fcntl failed\n");
            return INVALID;
        }
        return 0;
    }
    static int Connect(const std::string &ip, int port)
    {
        int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sfd == INVALID_SOCKET)
        {
            std::cout << "invalid socket !\n" << std::endl;
            return sfd;
        }
        struct sockaddr_in server_addr; 
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family      = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        server_addr.sin_port        = htons(port);

        if(connect(sfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            printf("connect error\n");
            return -2;
        }
        if (Nonblock(sfd) < 0)
        {
            printf("nonblock error !\n");
            return -3;
        }
        printf("connected to server(ip=%s, port=%d, fd=%d)\n", ip.c_str(), port, sfd);
        return sfd;
    }
    static int CreateBindListen(const std::string &ip, int port, bool isBlock/* = false*/)
    {
        int sfd = CreateBind(ip, port);
        if (!isBlock && Nonblock(sfd) == INVALID)
        {
            printf("nonblock error\n");
            abort();
        }
        if (listen(sfd, SOMAXCONN) == INVALID)
        {
            printf("listen error\n");
            abort();
        }
        printf("CreateBindListen: ip[%s], prot[%d], fd[%d]\n", ip.c_str(), port, sfd);
        return sfd;
    }
    static int Setsockopt(int sfd)
    {
        int rcvBuff = 524288;//512k
        if (0 != setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, (const char *)&rcvBuff, sizeof(rcvBuff)))
        {
            printf("set rcvBuff failed!\n");
            close(sfd);
            return INVALID;
        }
        int sndBuff = 524288;//512k
        if (0 != setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, (const char *)&sndBuff, sizeof(sndBuff)))
        {
            printf("set sndBuff failed!\n");
            close(sfd);
            return INVALID;
        }
        int nOpt = 1;
        if (0 != setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (const char *)&nOpt, sizeof(nOpt)))
        {
            printf("set tcp_nodelay failed!\n");
            close(sfd);
            return INVALID;
        }
        struct linger s_linger;
        s_linger.l_onoff = 1;
        s_linger.l_linger = 0;
        if (0 != setsockopt(sfd, SOL_SOCKET, SO_LINGER, (const char *)&s_linger, sizeof(s_linger)))
        {
            printf("set linger failed!\n");
            close(sfd);
            return INVALID;
        }
        //如果在发送数据的时，希望不经历由系统缓冲区到socket缓冲区的拷贝而影响 
        int nZero = 0;
        setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
        //同上在recv()完成上述功能(默认情况是将socket缓冲区的内容拷贝到系统缓冲区) 
        nZero = 0;
        setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, (char *)&nZero, sizeof(nZero));
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
    void Run()
    {
        mIsRunning      = true;
        Link *link      = new AcceptLink();
        std::string hostname;
        std::string localip;
        SocketUtil::GetHostInfo(hostname, localip);
        link->mFd       = SocketUtil::CreateBindListen(localip, mPort, false);
        link->mMotor    = this;
        AddLink(link, false);
        std::cout << "TcpMotor running!" << std::endl;
        mThread = std::thread(&TcpMotor::Loop, this);
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
            cnt  = cnt > 0 ? cnt : 0;
            cnt += SendHandler(cnt + 1);
            if (cnt > 0) usleep(1);
            //cnt > 0 ? usleep(1) : usleep(10);
            //usleep(2);
        }
    }
    void SetRecvHandler(TcpRecvHandler* recv) { mRecvHandler = recv; }
    void SetSendHandler(TcpSendHandler* send) { mSendHandler = send; }
    void RecvHandler(std::string callback_ip, int callback_port, void* content, int contentLen)
    {
        mRecvHandler->OnRecv(callback_ip, callback_port, (char *)content, contentLen); 
    }
    int SendHandler(int num)
    {
        int send_num = 0;
        for (int i = 0; i < num; ++i)
        {
            SendPacket *packet = nullptr;
            bool result = mSendQueue.try_dequeue(packet);
            if (!result || !packet)
                return send_num;
            std::string key = SocketUtil::MakeKeyByIpPort(packet->mIp, packet->mPort);
            Link *link = nullptr;
            auto it = mIpPortLink.find(key);
            if (it == mIpPortLink.end())
            {
                int sfd = SocketUtil::Connect(packet->mIp, packet->mPort);
                if (sfd < 0 || SocketUtil::Nonblock(sfd) != 0 || SocketUtil::Setsockopt(sfd) != 0)
                    continue;
                link                = new SocketLink();
                link->mFd           = sfd;
                link->mIp           = packet->mIp;
                link->mPort         = packet->mPort;
                link->mMotor        = this;
                mIpPortLink[key]    = link;
                AddLink(link, true);
            }
            else
                link = it->second;
            if (!link)
                continue;
            int fail_num = 0;
            int want_len = packet->mLen;
            int gone_len = 0;
            while (want_len > 0)
            {
                int n = send(link->mFd, packet->mData + gone_len, want_len, MSG_NOSIGNAL);
                if (n >= 0)
                {
                    gone_len += n;
                    want_len -= n;
                    //if (want_len > 0)
                        //printf("Sent Half Packet: send to ip[%s], port[%5d], fd[%5d], gone_len[%2d], want_len[%2d], n[%2d]\n", 
                        //    link->mIp.c_str(), link->mPort, link->mFd, gone_len, want_len, n);
                }
                else
                {
                    //printf("Sent failed Packet: send to ip[%s], port[%5d], fd[%5d], gone_len[%2d], want_len[%2d], n[%2d]\n", 
                    //    link->mIp.c_str(), link->mPort, link->mFd, gone_len, want_len, n);
                    if (fail_num++ < 3)
                    {
                        //printf("Sent failed Packet: send to ip[%s], port[%5d], fd[%5d], gone_len[%2d], want_len[%2d], n[%2d], fail_num[%d]\n", 
                        //    link->mIp.c_str(), link->mPort, link->mFd, gone_len, want_len, n, fail_num);
                        usleep(1);
                    }
                    else
                    {
                        DelLink(link);
                        break;
                    }
                }
            }
            ++send_num;
            delete packet;
        }
        return send_num;
    }
    void Send(std::string ip, int port, void* data, int len, void* ctx)
    {
        SendPacket *packet = new SendPacket(ip, port, data, len, ctx);
        bool r = mSendQueue.enqueue(packet);
    }
    int AddLink(Link *link, bool isForce)
    {
        if (!isForce)
        {
            std::string key = SocketUtil::MakeKeyByIpPort(link->mIp, link->mPort);
            auto it = mIpPortLink.find(key);
            if (it == mIpPortLink.end())
                mIpPortLink[key] = link;
        }
        struct epoll_event event;
        event.events    = EPOLLIN | EPOLLET;
        event.data.fd   = link->mFd;
        event.data.ptr  = (void *)link;
        int result = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, link->mFd, &event);
        std::cout << "AddLink mFd=" << std::to_string(link->mFd) << ", result=" << std::to_string(result) << std::endl;
        return result;
    }
    int DelLink(Link *link)
    {
        int result = epoll_ctl(mEpollFd, EPOLL_CTL_DEL, link->mFd, NULL);//TODO 删除失败情况
        std::string key = SocketUtil::MakeKeyByIpPort(link->mIp, link->mPort);
        auto it = mIpPortLink.find(key);
        if (it != mIpPortLink.end())
            mIpPortLink.erase(it);
        std::cout << "DelLink mFd=" << std::to_string(link->mFd) << ", result=" << std::to_string(result) << std::endl;
        close(link->mFd);
        delete link;
        return result;
    }
    int ModLink(Link *link, bool isWrite)
    {
        struct epoll_event event;
        event.events    = EPOLLIN | (isWrite ? EPOLLOUT : 0);
        event.data.ptr  = (void *)link;
        int result = epoll_ctl(mEpollFd, EPOLL_CTL_MOD, link->mFd, &event);
        std::cout << "ModLink mFd=" << std::to_string(link->mFd) << ", result=" << std::to_string(result) << std::endl;
        return result;
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
    std::unordered_map<std::string, Link*> mIpPortLink;//key: ip:port
    //
    int                     mEpollFd;
    struct epoll_event      *mEvents;
    //
    std::thread             mThread;
    //
    //moodycamel::ConcurrentQueue<Packet*> mRecvQueue;
    moodycamel::ConcurrentQueue<SendPacket*> mSendQueue;
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
                done = 1;
            else
            {
                printf("err recv count:%d errno:%d\n",count,errno);
                done = 2;
            }
        }
        else if (count == 0)
            done = 1;
        else
        {
            mEnd += count;
            //printf("SocketLink::OnRecv count:%d mHead:%d mEnd:%d\n", count, mHead, mEnd);
            while (mHead < mEnd)
            {
                if (mHead + sizeof(Packet) < MAX_RECBUFF_SIZE)
                {
                    Packet *packet = (Packet *)(mBuff + mHead);
                    if (packet && packet->len < MAX_PACKET_SIZE)
                    {
                        if (mHead + packet->len > mEnd) // 断包 
                        {
                            if (mHead + packet->len > MAX_RECBUFF_SIZE)
                            {
                                mEnd -= mHead;
                                memcpy(mBuff, mBuff + mHead, mEnd);
                                mHead = 0;
                            }
                            break;
                        }
                        else
                        {
                            // 消费
                            mMotor->RecvHandler(mIp, mPort, (void *)packet->data, packet->len - sizeof(Packet));
                            mHead += packet->len;
                        }
                    }
                    else //错误包 
                    {
                        mHead = 0;
                        mEnd  = 0;
                        done  = 2;
                        break;
                    }
                }
                else
                {
                    mEnd -= mHead;
                    memcpy(mBuff, mBuff + mHead, mEnd);
                    mHead = 0;
                    break;
                }
            }
        }
        //
        if (mEnd == MAX_RECBUFF_SIZE)
        {
            if (mHead < MAX_RECBUFF_SIZE)
            {
                mEnd -= mHead;
                memcpy(mBuff, mBuff + mHead, mEnd);
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
            break;
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
            printf("all have accepted\n");
            break;
        }
        SocketUtil::Setsockopt(socket_fd);
        char ip[NI_MAXHOST], port[NI_MAXSERV];
        if (getnameinfo((struct sockaddr *)&addr, in_len, ip, sizeof ip, port, sizeof port, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
        {
            std::cout << "Accepted connection (ip=" << ip << ", port=" << port << ", socket_fd=" << std::to_string(socket_fd) << ")" << std::endl;
            continue;
        }
        if (SocketUtil::Nonblock(socket_fd) < 0)
            continue;
        std::cout << "Accepted connection (ip=" << ip << ", port=" << port << ", socket_fd=" << std::to_string(socket_fd) << ")" << std::endl;    
        Link *new_link      = new SocketLink();
        new_link->mFd       = socket_fd;
        new_link->mMotor    = mMotor;
        new_link->mIp       = ip;
        new_link->mPort     = atoi(port);
        mMotor->AddLink(new_link, false);
    }
}

} //namespace dcore
