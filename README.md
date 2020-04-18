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
	
	占用一个CPU 75～85%， 8万QPS， 平均延时3.5ms
	
	all_num[36000000], failed_num[0], succ_rate[1000000]
	msg_len     [1024]
	qps         [84468]
	us0_499     [7737727]
	us500_999   [24535934]
	ms1_9       [3391688]
	ms10_19     [21809]
	ms20_29     [12694]
	ms30_39     [11945]
	ms40_49     [10867]
	ms50_59     [10455]
	ms60_69     [9416]
	ms70_79     [8981]
	ms80_89     [8333]
	ms90_99     [7125]
	ms100_199   [58846]
	ms200_299   [43349]
	ms300_399   [35556]
	ms400_499   [24363]
	ms500_599   [16350]
	ms600_699   [14669]
	ms700_799   [12488]
	ms800_899   [10335]
	ms900_999   [7865]
	ms1000X     [9205]
	average     [3523]
	sumCount    [36000000]
	
## 内存检测

	==00:00:00:17.467 666== HEAP SUMMARY:
	==00:00:00:17.467 666==     in use at exit: 0 bytes in 0 blocks
	==00:00:00:17.467 666==   total heap usage: 506,685 allocs, 506,685 frees, 345,307,952 bytes allocated
	==00:00:00:17.467 666== 
	==00:00:00:17.468 666== All heap blocks were freed -- no leaks are possible