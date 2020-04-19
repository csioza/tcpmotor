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

    while(1)
    {
        usleep(1000000);
    }

    server->Stop();
    delete server;

    return 0;
}