#include "tcpmotor.h"
#include <signal.h>


static long long failed_num = 0;
static long long all_num = 0;
static void stop(int signo)
{
    printf("\nall_num[%ld], failed_num[%ld], succ_rate[%ld]\n", all_num, failed_num, 1000000 - failed_num * 1000000 / all_num);
}
int main(int argc, char *argv[])
{

    // if (signal(SIGINT, stop) == SIG_ERR) {
    //     return 0;
    // }
    if (argc != 6)
    {
        printf("ip port fd_num interval packet_size run_time(s)\n");
    }

    std::string ip = argv[1];
    int port = atoi(argv[2]);
    const int max = atoi(argv[3]);
    int sleep_time = atoi(argv[4]);
    int packet_size = atoi(argv[5]);
    packet_size = packet_size > 16 ? packet_size - 16 : packet_size;
    std::string tail = dcore::RandomString(packet_size);
    int run_time = atoi(argv[6]);

    int *fds = new int[max];
    memset(fds, 0 , sizeof(fds));
    int endtime = dcore::TimeUtil::NowTimeS() + run_time;
    while (endtime > dcore::TimeUtil::NowTimeS()) 
    {
        for (int j = 0; j < max; ++j)
        {
            if (fds[j] == 0)
            {
                int sfd = dcore::SocketUtil::Connect(ip, port);
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
                //    ip.c_str(), 11111, fds[j], gone_len, want_len, n);
                if (n >= 0)
                {
                    gone_len += n;
                    want_len -= n;
                    // if (want_len > 0)
                    //     printf("Sent Half Packet: send to ip[%s], port[%5d], fd[%5d], gone_len[%2d], want_len[%2d], n[%2d]\n", 
                    //        ip.c_str(), 11111, fds[j], gone_len, want_len, n);
                }
                else
                {
                    printf("Sent Fail Packet: send to ip[%s], port[%5d], fd[%5d], gone_len[%2d], want_len[%2d], n[%2d]\n", 
                       ip.c_str(), 11111, fds[j], gone_len, want_len, n);
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
            //usleep(10);
        }
        usleep(sleep_time);
    }

    delete[] fds;

    return 0;
}