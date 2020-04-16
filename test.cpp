#include "tcpmotor.h"
#include <signal.h>


static long long failed_num = 0;
static long long all_num = 0;
static void stop(int signo)
{
    printf("\nall_num[%ld], failed_num[%ld], succ_rate[%ld]\n", all_num, failed_num, 1000000 - failed_num * 1000000 / all_num);
}
int main()
{

    // if (signal(SIGINT, stop) == SIG_ERR) {
    //     return 0;
    // }

    dcore::TcpMotor *server = new dcore::TcpMotor(11111);
    dcore::TcpRecvHandler * handler = new dcore::TcpRecvHandler();
    server->SetRecvHandler(handler);
    server->Run();

    const int max = 100;
    int sleep_time = 50;

    std::string hostname;
    std::string localip;
    dcore::SocketUtil::GetHostInfo(hostname, localip);
    std::string tail = dcore::RandomString(1008);

    int fds[max] = {0};
    for (int i = 0; i < 1000; ++i)
    {
        for (int j = 0; j < max; ++j)
        {
            if (fds[j] == 0)
            {
                int sfd = dcore::SocketUtil::Connect(localip, 11111);
                if (sfd < 0 || dcore::SocketUtil::Setsockopt(sfd) < 0)
                    continue;
                fds[j] = sfd;
            }
            std::string data        = std::to_string(dcore::TimeUtil::NowTimeUs()) + tail;
            int len                 = data.size() + sizeof(dcore::Packet);
            dcore::Packet *packet   = (dcore::Packet *)malloc(len);
            packet->len             = len;
            memcpy(packet->data, data.c_str(), data.size());

            int want_len = len;
            int gone_len = 0;
            while (want_len > 0)
            {
                int n = send(fds[j], (char *)packet + gone_len, want_len, MSG_NOSIGNAL);
                // printf("Sent Packet: send to ip[%s], port[%5d], fd[%5d], gone_len[%2d], want_len[%2d], n[%2d]\n", 
                //    localip.c_str(), 11111, fds[j], gone_len, want_len, n);
                if (n >= 0)
                {
                    gone_len += n;
                    want_len -= n;
                    // if (want_len > 0)
                    //     printf("Sent Half Packet: send to ip[%s], port[%5d], fd[%5d], gone_len[%2d], want_len[%2d], n[%2d]\n", 
                    //        localip.c_str(), 11111, fds[j], gone_len, want_len, n);
                }
                else
                {
                    usleep(10);
                    ++failed_num;
                    close(fds[j]);
                    fds[j] = 0;
                    break;
                }
            }
            all_num++;
            if (all_num % 100000 == 0)
            {
                printf("\nall_num[%ld], failed_num[%ld], succ_rate[%ld]\n", all_num, failed_num, 1000000 - failed_num * 1000000 / all_num);
            }
            delete packet;
        }
        usleep(sleep_time);
    }
    server->Stop();
    delete server;

    return 0;
}