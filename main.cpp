#include "tcpmotor.h"

int main()
{
    dcore::TcpMotor *tcp = new dcore::TcpMotor(11111);

    moodycamel::ConcurrentQueue<int> q;
	for (int i = 0; i != 123; ++i)
		q.enqueue(i);

	int item;
	for (int i = 0; i != 123; ++i) {
		q.try_dequeue(item);
		assert(item == i);
	}
    while(true)
    {
        usleep(10000000);
    }
    return 0;
}
