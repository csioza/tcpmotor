# Overview:
TcpMotor is a minimal tcp transmit lib, whitch has lock-free, multi-thread, only-header, using c++11 and high performance features.

# Design:


# Usage:

## Notice:


# Test
## 生产环境
	centos7, 24cpu

	跨机房 cpu:3%
	using matrixqueue.h

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
	average     [87]
	sumCount    [680000]

## 本地机器
	using matrixqueue2.h

	msg_len     [1024]
	qps         [15585]
	us0_499     [994711]
	us500_999   [504]
	ms1_9       [75]
	ms10_19     [0]
	ms20_29     [0]
	ms30_39     [0]
	ms40_49     [0]
	ms50_59     [0]
	ms60_69     [0]
	ms70_79     [0]
	ms80_89     [0]
	ms90_99     [0]
	average     [183]
	sumCount    [1000000]

	using matrixqueue.h

	msg_len     [1024]
	qps         [17193]
	us0_499     [993150]
	us500_999   [679]
	ms1_9       [130]
	ms10_19     [0]
	ms20_29     [0]
	ms30_39     [0]
	ms40_49     [0]
	ms50_59     [0]
	ms60_69     [0]
	ms70_79     [0]
	ms80_89     [0]
	ms90_99     [0]
	average     [162]
	sumCount    [1000000]

	using concurrentqueue.h

	msg_len     [1024]
	qps         [17856]
	us0_499     [979580]
	us500_999   [671]
	ms1_9       [1218]
	ms10_19     [309]
	ms20_29     [252]
	ms30_39     [39]
	ms40_49     [0]
	ms50_59     [0]
	ms60_69     [0]
	ms70_79     [0]
	ms80_89     [0]
	ms90_99     [0]
	average     [178]
	sumCount    [1000000]
## 内存检测

	==00:00:00:17.467 666== HEAP SUMMARY:
	==00:00:00:17.467 666==     in use at exit: 0 bytes in 0 blocks
	==00:00:00:17.467 666==   total heap usage: 506,685 allocs, 506,685 frees, 345,307,952 bytes allocated
	==00:00:00:17.467 666== 
	==00:00:00:17.468 666== All heap blocks were freed -- no leaks are possible

# TODO List:
	1.限流
	2.DelLink 删除失败情况
	3.增强健壮性
	4.压测
	5.queue验证正确性