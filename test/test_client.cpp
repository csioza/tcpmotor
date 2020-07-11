#include "tcpmotor.h"
#include <signal.h>


static long long failed_num = 0;
static long long all_num = 0;
static void stop(int signo)
{
    printf("\nall_num[%ld], failed_num[%ld], succ_rate[%ld]\n", all_num, failed_num, 1000000 - failed_num * 1000000 / all_num);
}

class TestTcpRecvHandler : public dcore::TcpRecvHandler
{
public:
    TestTcpRecvHandler() : sumCount(0), us0_499(0), us500_999(0),
            ms10_19(0), ms20_29(0), ms30_39(0), ms40_49(0), ms50_59(0), ms60_69(0), ms70_79(0), ms80_89(0), ms90_99(0), 
            ms100_199(0), ms200_299(0), ms300_399(0), ms400_499(0), ms500_599(0), ms600_699(0), ms700_799(0), ms800_899(0), ms900_999(0), 
            ms1000X(0), sumSub(0), average(0) {}
    virtual ~TestTcpRecvHandler() {}
    virtual void OnRecv(std::string callback_ip, int callback_port, char* content, int contentLen) 
    {
        static int64_t last = dcore::TimeUtil::NowTimeUs();
        std::string data(content, content + contentLen);
        std::string time(content, content + 16);
        int64_t now = dcore::TimeUtil::NowTimeUs();
        sumCount++;
        int64_t sub = now - stoll(time);
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
        else if (sub < 200000)
            ms100_199++;
        else if (sub < 300000)
            ms200_299++;
        else if (sub < 400000)
            ms300_399++;
        else if (sub < 500000)
            ms400_499++;
        else if (sub < 600000)
            ms500_599++;
        else if (sub < 700000)
            ms600_699++;
        else if (sub < 800000)
            ms700_799++;
        else if (sub < 900000)
            ms800_899++;
        else if (sub < 1000000)
            ms900_999++;
        else
            ms1000X++;
        sumSub += sub;
        if (sumCount != 0 && sumCount % 100000 == 0)
        {
            average = sumSub / sumCount;
            int64_t qps = sumCount * 1000000 / (now - last);
            printf("\nmsg_len     [%ld]\nqps         [%ld]\nus0_499     [%ld]\nus500_999   [%ld]\nms1_9       [%ld]\nms10_19     [%ld]\nms20_29     [%ld]\nms30_39     [%ld]\nms40_49     [%ld]\nms50_59     [%ld]\nms60_69     [%ld]\nms70_79     [%ld]\nms80_89     [%ld]\nms90_99     [%ld]\nms100_199   [%ld]\nms200_299   [%ld]\nms300_399   [%ld]\nms400_499   [%ld]\nms500_599   [%ld]\nms600_699   [%ld]\nms700_799   [%ld]\nms800_899   [%ld]\nms900_999   [%ld]\nms1000X     [%ld]\naverage     [%ld]\nsumCount    [%ld]\n" ,
                /*data.c_str(), */contentLen, qps,
                us0_499, us500_999, ms1_9, ms10_19, ms20_29, ms30_39, ms40_49,ms50_59, ms60_69, ms70_79, ms80_89, ms90_99, 
                ms100_199, ms200_299, ms300_399, ms400_499, ms500_599, ms600_699, ms700_799, ms800_899, ms900_999, ms1000X,
                average, sumCount.load());
            sumCount = 0;
            us0_499 = 0;
            us500_999 = 0;
            ms1_9 = 0;
            ms10_19 = 0;
            ms20_29 = 0;
            ms30_39 = 0;
            ms40_49 = 0;
            ms50_59 = 0;
            ms60_69 = 0;
            ms70_79 = 0;
            ms80_89 = 0;
            ms90_99 = 0;
            ms100_199 = 0;
            ms200_299 = 0;
            ms300_399 = 0;
            ms400_499 = 0;
            ms500_599 = 0;
            ms600_699 = 0;
            ms700_799 = 0;
            ms800_899 = 0;
            ms900_999 = 0;
            ms1000X = 0;
            sumSub = 0;
            last = now;
        }
        // printf("TcpRecvHandler::OnRecv from ip[%s], port[%d], content[%s], contentLen[%d], sub[%d]\n",
        //         callback_ip.c_str(), callback_port, data.c_str(), contentLen, now - stoll(data));
    }
    std::atomic<int64_t> sumCount;
    int64_t us0_499;
    int64_t us500_999;
    int64_t ms1_9;
    int64_t ms10_19;
    int64_t ms20_29;
    int64_t ms30_39;
    int64_t ms40_49;
    int64_t ms50_59;
    int64_t ms60_69;
    int64_t ms70_79;
    int64_t ms80_89;
    int64_t ms90_99;
    int64_t ms100_199;
    int64_t ms200_299;
    int64_t ms300_399;
    int64_t ms400_499;
    int64_t ms500_599;
    int64_t ms600_699;
    int64_t ms700_799;
    int64_t ms800_899;
    int64_t ms900_999;
    int64_t ms1000X;
    int64_t sumSub;
    int64_t average;
};

int main(int argc, char *argv[])
{

    // if (signal(SIGINT, stop) == SIG_ERR) {
    //     return 0;
    // }
    if (argc != 6)
    {
        printf("ip port fd_num interval packet_size run_time(s)\n");
    }

    //////////////////////////////////////////////////////
    std::string ip = argv[1];
    int port = atoi(argv[2]);
    const int max = atoi(argv[3]);
    int sleep_time = atoi(argv[4]);
    int packet_size = atoi(argv[5]);
    packet_size = packet_size > 16 ? packet_size - 16 : packet_size;

    dcore::TcpMotor *server = new dcore::TcpMotor(11122, 5);
    dcore::TcpRecvHandler * handler = new TestTcpRecvHandler();
    server->SetRecvHandler(handler);
    server->Run();

    std::string tail = dcore::RandomString(packet_size);
    int run_time = atoi(argv[6]);
    int endtime = dcore::TimeUtil::NowTimeS() + run_time;
    int count = 0;
    while (endtime > dcore::TimeUtil::NowTimeS()) 
    {
        std::string data = std::to_string(dcore::TimeUtil::NowTimeUs()) + tail;
        std::string hash = dcore::HashCodeString(data);
        data += hash;
        //printf("data.size %d hash %s data %s\n",data.size(), hash.c_str(), data.c_str());
        //memcpy(content, data.c_str(), data.size());
        for (int i = 0; i < max; ++i)
        {
            server->Send(ip, port, data.c_str(), data.size(), NULL);
            count++;
        }
        if (count % 10000 == 0)
        {
            printf("send packet num %d\n",count);
        }
        usleep(sleep_time);
    }

    while (1)
    {
        usleep(1000000000);
    }
    
    return 0;
}