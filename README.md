# TcpMotor
a tcp transmit lib, lock free, multi threads

	TODO:
	1.包的有序性
	2.多生产多消费队列能否动态长度
	
	1.限流
	2.DelLink 删除失败情况
	3.增强健壮性
	4.压测
	5.queue验证正确性

## 生产环境
	centos7, 32cpu

	跨机房 cpu:50%
	msg_len     [1024]
	qps         [96175]
	us0_499     [667254]
	us500_999   [8824]
	ms1_9       [1438]
	ms10_19     [0]
	ms20_29     [0]
	ms30_39     [0]
	ms40_49     [0]
	ms50_59     [0]
	ms60_69     [0]
	ms70_79     [0]
	ms80_89     [0]
	ms90_99     [0]
	ms100_199   [0]
	ms200_299   [0]
	ms300_399   [0]
	ms400_499   [0]
	ms500_599   [0]
	ms600_699   [0]
	ms700_799   [0]
	ms800_899   [0]
	ms900_999   [0]
	ms1000X     [0]
	average     [87]
	sumCount    [680000]
	
## 内存检测

	==00:00:00:17.467 666== HEAP SUMMARY:
	==00:00:00:17.467 666==     in use at exit: 0 bytes in 0 blocks
	==00:00:00:17.467 666==   total heap usage: 506,685 allocs, 506,685 frees, 345,307,952 bytes allocated
	==00:00:00:17.467 666== 
	==00:00:00:17.468 666== All heap blocks were freed -- no leaks are possible