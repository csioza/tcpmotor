#pragma once
//c++
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream> 
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <stdbool.h>
#include <cstring>
#include <atomic>
#include <thread>
//linux
#include <sys/epoll.h>
#include <sys/eventfd.h>
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
#include <signal.h>
//

#define USE_MATRIX_QUEUE
#ifdef USE_MATRIX_QUEUE
#include "matrixqueue.h"
#else
#include "concurrentqueue.h"
#endif

typedef char                int8;
typedef short               int16;
typedef int                 int32;
typedef long long           int64;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

namespace dcore {

#define MAX_TRIGGER_NUM         100
#define MAX_PACKET_SIZE         8192
#define MAX_RECBUFF_SIZE        (MAX_PACKET_SIZE * 3)
#define INVALID                 -1 
#define MAX_EVENT_NUM           1024
#define MAX_QUEUE_BIAS          100
#define INVALID_SOCKET          -1
#define INT_64_MAX              0x7fffffffffffffff
#define LINK_ACTIVE_TIMEOUT     3600//秒
#define EPOLL_WAIT_TIMEOUT      10000//毫秒
#define MAX_MATRIX_THREAD       32
#define MAX_MATRIX_QUEUE_SIZE   50000

#pragma pack(1)
struct Packet  //用于线程的收发队列里
{
    uint32 len;//总包长度，包含len长度
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
    static uint64 NowTimeS() {
        return time(NULL);
    }
    // static uint64 NowTimeClick() {
    //     return click();
    // }
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
int Mod(uint64 key, uint64 mod)
{
    return key % mod;
}
///////////////////////////////////////////////////////////////////////////////
//handler class
///////////////////////////////////////////////////////////////////////////////
class TcpRecvHandler
{
public:
    TcpRecvHandler() {}
    virtual ~TcpRecvHandler() {}
    virtual void OnRecv(std::string callback_ip, int callback_port, char* content, int contentLen) = 0;
};
class TcpSendHandler
{
public:
    TcpSendHandler() {}
    virtual ~TcpSendHandler() {}
    virtual void OnSuccess(std::string ip, int port, void* ctx) = 0;
    virtual void OnFailure(std::string ip, int port, void* ctx) = 0;
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
    SendPacket(std::string ip, int port, const char* data, int len, void* ctx) : mIp(ip), mPort(port), mCtx(ctx)
    {
        mLen            = len + sizeof(Packet);
        mData           = (char *)malloc(mLen);
        Packet *packet  = (Packet *)mData;
        packet->len     = mLen;
        memcpy(packet->data, data, len);
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
    static bool GetHostInfo(std::string& hostName, std::string& Ip) 
    {
        char name[256];
        gethostname(name, sizeof(name));
        hostName = name;
        struct hostent* host = gethostbyname(name);
        char ipStr[32];
        const char* ret = inet_ntop(host->h_addrtype, host->h_addr_list[0], ipStr, sizeof(ipStr));
        if (NULL == ret) 
        {
            std::cout << "hostname transform to ip failed\n" << std::endl;
            return false;
        }
        Ip = ipStr;
        return true;
    }
    static uint64 MakeKeyByIpPort(const std::string &ip, int port)
    {
        struct in_addr ip_addr;
        if (0 == inet_aton(ip.c_str(), &ip_addr))
        {
            printf("MakeKeyIpPort fail, ip[%s], port[%d]\n", ip.c_str(), port);
            return 0;
        }
        return ((uint64)ip_addr.s_addr << 32) | (uint64)port;
    }
    static int CreateBind(const std::string &ip, int port)
    {
        int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sfd == INVALID_SOCKET)
        {
            std::cout << "invalid socket !\n" << std::endl;
            return sfd;
        }
        sockaddr_in serverAddr;//绑定端口 
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
        serverAddr.sin_port = htons(port);
        int iErrorMsg = bind(sfd, (sockaddr*)&serverAddr, sizeof(serverAddr));  
        if (iErrorMsg < 0)  //绑定失败  
        {  
            printf("bind failed with error : %d\n", iErrorMsg); 
            return -2;  
        }
        return sfd;
    }
    static int Nonblock(int sfd)
    {
        int flags;
        flags = fcntl (sfd, F_GETFL, 0);//得到文件状态标志
        if (flags == INVALID)
        {
            printf("fcntl failed\n");
            return INVALID;
        }
        flags |= O_NONBLOCK;//设置文件状态标志
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
            printf("connect error, ip[%s], port[%d]\n", ip.c_str(), port);
            return -2;
        }
        if (Nonblock(sfd) < 0)
        {
            printf("nonblock error !\n");
            return -3;
        }
        //printf("connected to server(ip=%s, port=%d, fd=%d)\n", ip.c_str(), port, sfd);
        return sfd;
    }
    static int CreateBindListen(const std::string &ip, int port, bool isBlock)
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
        int rcvBuff = 524288;//512k 设置0表示不经历由系统缓冲区到socket缓冲区的拷贝而影响，本程序实验证明为0时发送超过1024B会有大量发送失败的情况
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
        return 0;
    }
};
///////////////////////////////////////////////////////////////////////////////
//main class
///////////////////////////////////////////////////////////////////////////////
class TcpMotor;
class Trigger;
enum LinkType
{
    LINK_TYPE_SOCKET = 1,
    LINK_TYPE_ACCEPT = 2,
    LINK_TYPE_EVENTS = 3,
};
class Link
{
public:
    Link() : mFd(0), mLastActiveTime(0), mPort(0), mKey(0), mTrigger(nullptr), mMotor(nullptr) {}
    virtual ~Link() {}
    virtual LinkType Type() = 0;
    virtual int OnRecv(int64 now, int cnt = 0) = 0;
    virtual void UpdateActiveTime(int64 now) {}
    //   
    int         mFd;
    int64       mLastActiveTime;
    std::string mIp;//远端ip
    int         mPort;//远端port
    uint64      mKey;//
    Trigger*    mTrigger;
    TcpMotor*   mMotor;
};
class SocketLink : public Link
{
public:
    SocketLink() : mHead(0), mEnd(0)
    {
        memset(mBuff, 0, MAX_RECBUFF_SIZE);
        mLastActiveTime = TimeUtil::NowTimeS() + LINK_ACTIVE_TIMEOUT; 
    }
    virtual ~SocketLink() {}
    virtual LinkType Type() { return LINK_TYPE_SOCKET; };
    virtual int OnRecv(int64 now, int cnt = 0);
    virtual void UpdateActiveTime(int64 now) { mLastActiveTime = now + LINK_ACTIVE_TIMEOUT; }
    int         mHead;
    int         mEnd;
    char        mBuff[MAX_RECBUFF_SIZE];
};
class AcceptLink : public Link
{
public:
    AcceptLink() { mLastActiveTime = INT_64_MAX; }
    virtual ~AcceptLink() {}
    virtual int OnRecv(int64 now, int cnt = 0);
    virtual LinkType Type() { return LINK_TYPE_ACCEPT; };
};
class EventsLink : public Link
{
public:
    EventsLink() { mLastActiveTime = INT_64_MAX; }
    virtual ~EventsLink() {}
    virtual int OnRecv(int64 now, int cnt = 0);
    virtual LinkType Type() { return LINK_TYPE_EVENTS; };
};

class Trigger
{
public:
    Trigger(TcpMotor *motor) : mMotor(motor), mIsRunning(false)
    {
        mEpollFd        = epoll_create(256);
        mEvents         = new struct epoll_event[MAX_EVENT_NUM];
#ifdef USE_MATRIX_QUEUE
        mSendQueue      = new MatrixQueue<SendPacket*>(MAX_MATRIX_THREAD, MAX_MATRIX_THREAD, MAX_MATRIX_QUEUE_SIZE);
        mLinkQueue      = new MatrixQueue<Link*>(MAX_MATRIX_THREAD, MAX_MATRIX_THREAD, MAX_MATRIX_QUEUE_SIZE);
#endif
    }
    ~Trigger()
    {
        delete[] mEvents;
        close(mEpollFd);
    }
    void Run()
    {
        mIsRunning  = true;
        std::cout << "Trigger running!" << std::endl;
        mDriveFd    = eventfd(0, EFD_NONBLOCK);
        Link *link  = new EventsLink();
        link->mFd   = mDriveFd;
        link->mKey  = 0;//注意，个例
        AddLink(link);
        mThread     = std::thread(&Trigger::Loop, this);
        pthread_setname_np(mThread.native_handle(), "TcpMotor-Trigger-Loop");
    }
    void Stop()
    {
        mIsRunning = false;
        if (mThread.joinable())
            mThread.join();
        for (auto it : mIpPortLink)
            DelLink(it.second);
    }
    int AddLink(Link *link)
    {
        link->mTrigger  = this;
        link->mMotor    = mMotor;
        struct epoll_event event;
        event.events    = EPOLLIN | EPOLLET;
        event.data.fd   = link->mFd;
        event.data.ptr  = (void *)link;
        int result = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, link->mFd, &event);//TODO 失败的情况
        if (link->Type() == LINK_TYPE_SOCKET)
            mLinkTimer.push(std::make_pair(link->mLastActiveTime, link->mKey));
        mIpPortLink[link->mKey] = link;
        std::cout << "AddLink mFd=" << std::to_string(link->mFd) << ", type=" << link->Type() << ", result=" << std::to_string(result) << std::endl;
        return result;
    }
    int PushLink(Link *link)
    {
#ifdef USE_MATRIX_QUEUE
        bool r = mLinkQueue->Push(link);
#else
        bool r = mLinkQueue.enqueue(link);
#endif
        if (!r)
        {
            std::cout << "PutLink failed!" << std::endl;
            delete link;
            return INVALID;
        }
        link->mTrigger  = this;
        link->mMotor    = mMotor;
        //std::atomic_thread_fence(std::memory_order_acquire);
        EventNotify();

        return 0;
    }
    int PushPacket(SendPacket *packet)
    {
#ifdef USE_MATRIX_QUEUE
        bool r = mSendQueue->Push(packet);
#else
        bool r = mSendQueue.enqueue(packet);
#endif
        if (!r)
        {
            std::cout << "PutPacket failed!" << std::endl;
            delete packet;
            return INVALID;
        }
        //std::atomic_thread_fence(std::memory_order_acquire);
        EventNotify();

        return 0;
    }
    bool EventNotify()
    {
        uint64 data = 1;//被坑了一晚上，没有赋值
        int result = write(mDriveFd, &data, sizeof(uint64));
        if (result < 0 && errno != EAGAIN)
            return false;
        return (result == sizeof(uint64));
    }
    bool EventReset()
    {
        uint64 data;
        int result = read(mDriveFd, &data, sizeof(uint64));
        if (result < 0 && errno != EAGAIN)
            return false;
        return (result == sizeof(uint64));
    }
    void AcceptLink()
    {
        Link *link = nullptr;
        for (int i = 0; i < MAX_EVENT_NUM; ++i)
        {
#ifdef USE_MATRIX_QUEUE
            if (mLinkQueue->Pop(link))
#else
            if (mLinkQueue.try_dequeue(link))
#endif
            {
                if (link)
                    AddLink(link);
                else
                    delete link;
            }
            else
                break;
        }
    }
    int SendHandler(int num, int64 now)
    {
        int send_num = 0;
        for (int i = 0; i < num; ++i)
        {
            SendPacket *packet = nullptr;
#ifdef USE_MATRIX_QUEUE
            bool result = mSendQueue->Pop(packet);
#else
            bool result = mSendQueue.try_dequeue(packet);
#endif
            if (!result || !packet)
                return send_num;
            auto key    = SocketUtil::MakeKeyByIpPort(packet->mIp, packet->mPort);
            Link *link  = nullptr;
            auto it = mIpPortLink.find(key);
            if (it == mIpPortLink.end())
            {
                int sfd = SocketUtil::Connect(packet->mIp, packet->mPort);
                if (sfd < 0 || SocketUtil::Setsockopt(sfd) != 0)
                    continue;
                link                = new SocketLink();
                link->mFd           = sfd;
                link->mIp           = packet->mIp;
                link->mPort         = packet->mPort;
                link->mKey          = key;
                AddLink(link);
            }
            else
                link = it->second;
            if (!link)
                continue;
            link->UpdateActiveTime(now);
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
                    if (want_len > 0)
                        printf("Sent Half Packet: send to ip[%s], port[%5d], fd[%5d], gone_len[%2d], want_len[%2d], n[%2d]\n", 
                                link->mIp.c_str(), link->mPort, link->mFd, gone_len, want_len, n);
                }
                else
                {
                    printf("Sent failed Packet: send to ip[%s], port[%5d], fd[%5d], gone_len[%2d], want_len[%2d], n[%2d]\n", 
                            link->mIp.c_str(), link->mPort, link->mFd, gone_len, want_len, n);
                    if (fail_num++ < 2)
                    {
                        printf("Sent failed Packet: send to ip[%s], port[%5d], fd[%5d], gone_len[%2d], want_len[%2d], n[%2d], fail_num[%d]\n", 
                                link->mIp.c_str(), link->mPort, link->mFd, gone_len, want_len, n, fail_num);
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
private:
    void Loop()
    {
        int wait_time = 0;
        while (mIsRunning)
        {
            int cnt = Wait(wait_time);
            int64 now = TimeUtil::NowTimeS();
            for (int i = 0; i < cnt; ++i)
            {
                if (mEvents[i].events & EPOLLIN)
                {
                    Link *link = (Link *)mEvents[i].data.ptr;
                    if (!link)
                        continue;
                    link->UpdateActiveTime(now);
                    if (link->OnRecv(now, cnt) != 0)
                        DelLink(link);
                }
            }
#ifdef USE_MATRIX_QUEUE
            if (mSendQueue->size() > 0 || mLinkQueue->size() > 0)
#else
            if (mSendQueue.size_approx() > 0 || mLinkQueue.size_approx() > 0)
#endif
            {
                wait_time = 0;
            }
            else
            {
                wait_time = CheckLinkActive(now);
                wait_time = wait_time > EPOLL_WAIT_TIMEOUT ? EPOLL_WAIT_TIMEOUT : wait_time;
            }
        }
    }
    int DelLink(Link *link)
    {
        int result = epoll_ctl(mEpollFd, EPOLL_CTL_DEL, link->mFd, NULL);//TODO 删除失败情况
        auto it = mIpPortLink.find(link->mKey);
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
    int CheckLinkActive(int64 now)
    {
        int resent = EPOLL_WAIT_TIMEOUT;
        while(!mLinkTimer.empty())
        {
            if (mLinkTimer.top().first > now)
            {
                resent = (mLinkTimer.top().first - now) * 1000;
                break;
            }
            auto it = mIpPortLink.find(mLinkTimer.top().second);
            if (it != mIpPortLink.end() && it->second)
            {
                if (it->second->mLastActiveTime <= mLinkTimer.top().first)
                    DelLink(it->second);
                else
                    mLinkTimer.push(std::make_pair(now + LINK_ACTIVE_TIMEOUT, it->second->mKey));
            }
            std::cout << "CheckLinkActive=" << mLinkTimer.top().second << std::endl;
            mLinkTimer.pop();
        }
        return resent;
    }
private:
    int                     mPort;
    bool                    mIsRunning;
    int                     mEpollFd;
    struct epoll_event*     mEvents;
    std::thread             mThread;
    std::unordered_map<uint64, Link*>           mIpPortLink;
#ifdef USE_MATRIX_QUEUE
    MatrixQueue<SendPacket*>                    *mSendQueue;
    MatrixQueue<Link*>                          *mLinkQueue;
#else
    moodycamel::ConcurrentQueue<SendPacket*>    mSendQueue;
    moodycamel::ConcurrentQueue<Link*>          mLinkQueue;
#endif
    TcpMotor*               mMotor;
    int                     mDriveFd;//eventfd 用于接收连接和发送数据，避免线程空跑
    //
    using Pair = std::pair<int64, uint64>;
    struct cmp { bool operator() (const Pair &a, const Pair &b) { return a.first > b.first; } };
    std::priority_queue<Pair, std::vector<Pair>, cmp> mLinkTimer;
};

class TcpMotor
{
public:
    TcpMotor(int port, int trigger_num) : mPort(port), mRecvHandler(nullptr), mSendHandler(nullptr), mAutoInc(0)
    {
        mTriggerNum = trigger_num > MAX_TRIGGER_NUM ? MAX_TRIGGER_NUM : trigger_num;
        for (int i = 0; i < mTriggerNum; ++i)
            mTriggers[i] = new Trigger(this);
    }
    ~TcpMotor()
    {
        for (int i = 0; i < mTriggerNum; ++i)
        {
            delete mTriggers[i];
        }
        if (mRecvHandler)
            delete mRecvHandler;
        if (mSendHandler)
            delete mSendHandler;
    }
    void Run()
    {
        std::cout << "TcpMotor running!" << std::endl;
        Link *link      = new AcceptLink();
        std::string hostname;
        std::string localip;
        SocketUtil::GetHostInfo(hostname, localip);
        link->mFd       = SocketUtil::CreateBindListen(localip, mPort, false);
        link->mKey      = SocketUtil::MakeKeyByIpPort(localip, mPort);
        for (int i = 0; i < mTriggerNum; ++i)
            mTriggers[i]->Run();
        mTriggers[0]->PushLink(link);
    }
    void Stop()
    {
        for (int i = 0; i < mTriggerNum; ++i)
            mTriggers[i]->Stop();
    }
    void SetRecvHandler(TcpRecvHandler* recv) { mRecvHandler = recv; }
    void SetSendHandler(TcpSendHandler* send) { mSendHandler = send; }
    void Send(std::string ip, int port, const char* data, int len, void* ctx)
    {
        SendPacket *packet = new SendPacket(ip, port, data, len, ctx);
        //auto trigger = mTriggers[Mod(SocketUtil::MakeKeyByIpPort(ip, port), mTriggerNum)];
        auto trigger = mTriggers[Mod(mAutoInc++, mTriggerNum)];
        trigger->PushPacket(packet);
    }
    int AddLink(Link *link)
    {
        std::cout << "TcpMotor AddLink!" << std::endl;
        auto trigger = mTriggers[Mod(link->mKey, mTriggerNum)];
        return trigger->PushLink(link);
    }
    void RecvHandler(std::string &ip, int port, void* content, int contentLen)
    {
        mRecvHandler->OnRecv(ip, port, (char *)content, contentLen); 
    }
private:
    int                     mPort;
    TcpRecvHandler*         mRecvHandler;
    TcpSendHandler*         mSendHandler;
    Trigger*                mTriggers[MAX_TRIGGER_NUM];
    int                     mTriggerNum;
    //
    std::atomic<int>        mAutoInc;
};

int SocketLink::OnRecv(int64 now, int cnt)
{
    int result = 0;
    while (1)
    {
        int done  = 0;
        int count = recv(mFd, mBuff + mEnd, MAX_RECBUFF_SIZE - mEnd, 0);
        if (count == -1)
        {
            if (errno == 0 || errno == EAGAIN)
                done = 1;
            else
            {
                //printf("err recv count:%d errno:%d\n", count, errno);
                //printf("SocketLink::OnRecv fail 22222222 mHead:%d mEnd:%d\n", mHead, mEnd);
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
                    if (packet && packet->len <= MAX_PACKET_SIZE)
                    {
                        if (mHead + packet->len > mEnd) //断包 
                        {
                            if (mHead + packet->len > MAX_RECBUFF_SIZE)
                            {
                                mEnd -= mHead;
                                memcpy(mBuff, mBuff + mHead, mEnd);
                                mHead = 0;
                            }
                            break;
                        }
                        else //消费
                        {
                            mMotor->RecvHandler(mIp, mPort, (void *)packet->data, packet->len - sizeof(Packet));
                            mHead += packet->len;
                        }
                    }
                    else //错误包 
                    {
                        //printf("SocketLink::OnRecv fail packet->len:%d mHead:%d mEnd:%d\n", packet->len, mHead, mEnd);
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
            {
                //printf("SocketLink::OnRecv fail 22222222 mHead:%d mEnd:%d\n", mHead, mEnd);
                done = 2;
            }
        }
        //
        if (done == 1)
            break;
        else if (done == 2)
        {
            //printf("SocketLink::OnRecv fail 11111111 mHead:%d mEnd:%d\n", mHead, mEnd);
            result = 1;
            break;
        }
    }
    return result;
}
int AcceptLink::OnRecv(int64 now, int cnt)
{
    while (1)
    {
        struct sockaddr_in addr;
        socklen_t in_len    = sizeof(struct sockaddr_in);
        int socket_fd       = accept(mFd, (struct sockaddr *)&addr, &in_len);
        if (socket_fd == INVALID_SOCKET)
        {
            //printf("all have accepted\n");
            break;
        }
        SocketUtil::Setsockopt(socket_fd);
        char ip[NI_MAXHOST], port[NI_MAXSERV];
        if (getnameinfo((struct sockaddr *)&addr, in_len, ip, sizeof ip, port, sizeof port, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
        {
            //std::cout << "Accepted connection (ip=" << ip << ", port=" << port << ", socket_fd=" << std::to_string(socket_fd) << ")" << std::endl;
            continue;
        }
        if (SocketUtil::Nonblock(socket_fd) < 0)
            continue;
        std::cout << "Accepted connection (ip=" << ip << ", port=" << port << ", socket_fd=" << std::to_string(socket_fd) << ")" << std::endl;    
        Link *new_link      = new SocketLink();
        new_link->mFd       = socket_fd;
        new_link->mIp       = ip;
        new_link->mPort     = atoi(port);
        new_link->mKey      = SocketUtil::MakeKeyByIpPort(new_link->mIp, new_link->mPort);
        mMotor->AddLink(new_link);
    }
    return 0;
}
int EventsLink::OnRecv(int64 now, int cnt)
{
    mTrigger->EventReset();
    mTrigger->AcceptLink();
    cnt  = cnt > 0 ? cnt : 0;
    mTrigger->SendHandler(cnt + 1, now);
    return 0;
}
} //namespace dcore