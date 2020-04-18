# TcpMotor
a tcp transmit lib, two files, lock free, two threads

	TODO:
	
	版本1
	1.增加新线程用于剥离业务与TcpMotor线程，收队列，不阻塞TcpMotor(待定)
	2.添加计时器，定时移除不活跃的link
	
	版本2
	1.尝试多个epoll,开多个线程处理收发，增加triger
	
## 环境

	docker centos7 5cpu
	
## 测试数据
	
	占用一个CPU 75～85%， 8万QPS， 平均延时658us
	
	all_num[30500000], failed_num[0], succ_rate[1000000]
	msg_len     [1024]
	qps         [85567]
	us0_499     [8107907]
	us500_999   [20776500]
	ms1_9       [1602449]
	ms10_19     [13129]
	ms20_29     [0]
	ms30_39     [5]
	ms40_49     [0]
	ms50_59     [0]
	ms60_69     [0]
	ms70_79     [0]
	ms80_89     [0]
	ms90_99     [5]
	ms100_199   [5]
	ms200_299   [0]
	ms300_399   [0]
	ms400_499   [0]
	ms500_599   [0]
	ms600_699   [0]
	ms700_799   [0]
	ms800_899   [0]
	ms900_999   [0]
	ms1000X     [0]
	average     [658]
	sumCount    [30500000]
	
## 内存检测

	==00:00:00:17.467 666== HEAP SUMMARY:
	==00:00:00:17.467 666==     in use at exit: 0 bytes in 0 blocks
	==00:00:00:17.467 666==   total heap usage: 506,685 allocs, 506,685 frees, 345,307,952 bytes allocated
	==00:00:00:17.467 666== 
	==00:00:00:17.468 666== All heap blocks were freed -- no leaks are possible