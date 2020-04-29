# TcpMotor
a tcp transmit lib, two files, lock free, multi threads

	TODO:
	
	1.限流
	2.DelLink 删除失败情况
	3.增强健壮性
	4.压测

	
## 环境

	docker centos7 5cpu
	
## 测试数据

	跨docker容器
	msg_len     [2048]
	qps         [41271]
	us0_499     [99969]
	us500_999   [31]
	ms1_9       [0]
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
	average     [195]
	sumCount    [100000]

	重启 usleep(100); cpu:53%
	all_num[32200000], failed_num[0], succ_rate[1000000]
	msg_len     [1024]
	qps         [166295]
	us0_499     [30606012]
	us500_999   [1377597]
	ms1_9       [213595]
	ms10_19     [2662]
	ms20_29     [0]
	ms30_39     [0]
	ms40_49     [48]
	ms50_59     [42]
	ms60_69     [44]
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
	average     [351]
	sumCount    [32200000]

	重启 usleep(10); cpu:58%
	all_num[10300000], failed_num[0], succ_rate[1000000]
	msg_len     [1024]
	qps         [166737]
	us0_499     [10292438]
	us500_999   [2546]
	ms1_9       [4913]
	ms10_19     [0]
	ms20_29     [0]
	ms30_39     [0]
	ms40_49     [79]
	ms50_59     [24]
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
	average     [182]
	sumCount    [10300000]

	重启 usleep(1); cpu:60%
	all_num[11100000], failed_num[0], succ_rate[1000000]
	msg_len     [1024]
	qps         [168062]
	us0_499     [11078056]
	us500_999   [5904]
	ms1_9       [15397]
	ms10_19     [586]
	ms20_29     [0]
	ms30_39     [0]
	ms40_49     [0]
	ms50_59     [26]
	ms60_69     [12]
	ms70_79     [0]
	ms80_89     [19]
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
	average     [165]
	sumCount    [11100000]

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