# tcpmotor
a tcp transmit lib, single file

<br>
TODO:
<br>
1.多个epoll,增加triger
<br>
2.收队列，不阻塞

<br>
测试数据, 环境 macos, i7,6核, 16gb, docker centos7
```
[稳定]
TcpRecvHandler::OnRecv from ip[172.17.0.3], port[53760], content[], contentLen[116],
 qps      [3631]
 us0_499  [197100]
 us500_999[488]
 ms1_9    [1925]
 ms10_19  [131]
 ms20_29  [118]
 ms30_39  [114]
 ms40_49  [116]
 ms50_59  [8]
 ms60_69  [0]
 ms70_79  [0]
 ms80_89  [0]
 ms90_99  [0]
 ms100X   [0]
 average  [158]
 sumCount [200000]

[不稳定]
 TcpRecvHandler::OnRecv from ip[172.17.0.3], port[53784], content[], contentLen[216],
 qps      [3627]
 us0_499  [137631]
 us500_999[114]
 ms1_9    [254]
 ms10_19  [1]
 ms20_29  [0]
 ms30_39  [0]
 ms40_49  [0]
 ms50_59  [0]
 ms60_69  [0]
 ms70_79  [0]
 ms80_89  [0]
 ms90_99  [0]
 ms100X   [0]
 average  [63]
 sumCount [138000]

 [不稳定]
 TcpRecvHandler::OnRecv from ip[172.17.0.3], port[53814], content[], contentLen[316],
 qps      [2862]
 us0_499  [24776]
 us500_999[22]
 ms1_9    [52]
 ms10_19  [2]
 ms20_29  [0]
 ms30_39  [0]
 ms40_49  [0]
 ms50_59  [0]
 ms60_69  [0]
 ms70_79  [0]
 ms80_89  [0]
 ms90_99  [0]
 ms100X   [148]
 average  [12870]
 sumCount [25000]

 TcpRecvHandler::OnRecv from ip[172.17.0.3], port[53840], content[], contentLen[1016],
 qps      [463]
 us0_499  [25772]
 us500_999[25]
 ms1_9    [43]
 ms10_19  [32]
 ms20_29  [44]
 ms30_39  [44]
 ms40_49  [32]
 ms50_59  [8]
 ms60_69  [0]
 ms70_79  [0]
 ms80_89  [0]
 ms90_99  [0]
 ms100X   [0]
 average  [296]
 sumCount [26000]
 
 TcpRecvHandler::OnRecv from ip[172.17.0.3], port[53896], content[], contentLen[1016],
 qps      [99]
 us0_499  [1576]
 us500_999[17]
 ms1_9    [1109]
 ms10_19  [1508]
 ms20_29  [482]
 ms30_39  [295]
 ms40_49  [13]
 ms50_59  [0]
 ms60_69  [0]
 ms70_79  [0]
 ms80_89  [0]
 ms90_99  [0]
 ms100X   [0]
 average  [10773]
 sumCount [5000]
 ```