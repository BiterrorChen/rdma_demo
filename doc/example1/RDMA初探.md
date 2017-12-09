# RDMA初探

标签（空格分隔）： RDMA

---
### 基本数据结构
- rdma_event_channel：用于创建RDMA ID
- rdma_cm_id：类似与sockfs
- rdma_cm_event：RDMA网络相关的事件
- rdma_conn_param：一条连接相关参数，例如接收端内存地址
- ibv_cq：Completion Queue
- ibv_mr：注册内存
- ibv_qp_init_attr：Queen Pair属性
- ibv_sge：接收/发送数据的本地内存地址
- ibv_send_wr：send work request
- ibv_recv_wr：receive work request
- ibv_wc：wait complete


### 建立监听端口
RDMA建立监听端口非常简单，确定端口后，只需要调用4个API
1. rdma_create_event_channel
2. rdma_create_id
3. rdma_bind_addr
4. rdma_listen
```
struct sockaddr_in addr;
struct rdma_cm_event *event = NULL;
struct rdma_cm_id *listener = NULL;
struct rdma_event_channel *ec = NULL;

memset(&addr, 0, sizeof(addr));
//设置ip地址
addr.sin_family = AF_INET;
addr.sin_port = htons(10086);
addr.sin_addr.s_addr = INADDR_ANY;
//创建一个channel，用与处理所有event
TEST_Z(ec = rdma_create_event_channel());
//创建ec相关的id，类似于TCP的sockfd
TEST_NZ(rdma_create_id(ec, &listener, NULL, RDMA_PS_TCP));
//为id绑定ip地址
TEST_NZ(rdma_bind_addr(listener, (struct sockaddr *)&addr));
//监听
TEST_NZ(rdma_listen(listener, 10)); /* backlog=10 is arbitrary */
```

### 建立连接
RDMA建立连接需要经历两个状态，`RDMA_CM_EVENT_CONNECT_REQUEST`和`RDMA_CM_EVENT_ESTABLISHED`
1. 从event_channel获取一个`CONNECT_REQUEST`事件
2. 创建PD(Protection Domain)
3. 创建CQ(Completion Queue)
4. 注册内存MR
5. 创建CP(Queue Pair)
6. 调用rdma_accept

```
//RDMA_CM_EVENT_CONNECT_REQUEST状态
//创建PD
pd = ibv_alloc_pd(cm_id->verbs);
//创建CC(completion channel)，用于处理completion queue event
comp_chan = ibv_create_comp_channel(cm_id->verbs);
//创建CQ，2个CQE
cq = ibv_create_cq(cm_id->verbs, 2, NULL, comp_chan, 0);
ibv_req_notify_cq(cq, 0)

buf = (uint32_t *)calloc(2, sizeof(uint32_t));
if (!buf) return 1;
//注册内存
mr = ibv_reg_mr(pd, 
                buf, 
                2 * sizeof(uint32_t),
                IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE);

qp_attr.cap.max_send_wr = 1;
qp_attr.cap.max_send_sge = 1;
qp_attr.cap.max_recv_wr = 1;
qp_attr.cap.max_recv_sge = 1;

qp_attr.send_cq = cq;
qp_attr.recv_cq = cq;

qp_attr.qp_type = IBV_QPT_RC;
//创建QP
err = rdma_create_qp(cm_id, pd, &qp_attr);

//将mem和rkey和连接绑定，client可能需要
rep_pdata.buf_va = htonl((uintptr_t)buf);
rep_pdata.buf_rkey = htonl(mr->rkey);

conn_param.responder_resources = 1;
conn_param.private_data = &rep_pdata;
conn_param.private_data_len = sizeof rep_pdata;

/* Accept connection */
err = rdma_accept(cm_id, &conn_param);
//RDMA_CM_EVENT_ESTABLISHED状态
```
### 发起连接
发起连接需要经历`RDMA_CM_EVENT_ADDR_RESOLVED`->`RDMA_CM_EVENT_ROUTE_RESOLVED`->`RDMA_CM_EVENT_ESTABLISHED`
```
//在建立监听端口有类似的代码，类似于TCP建立sockfd
cm_channel = rdma_create_event_channel();
rdma_create_id(cm_channel, &cm_id, NULL, RDMA_PS_TCP);
//解析地址
rdma_resolve_addr(cm_id, NULL, t->ai_addr, RESOLVE_TIMEOUT_MS);
//RDMA_CM_EVENT_ADDR_RESOLVED状态

rdma_resolve_route(cm_id, RESOLVE_TIMEOUT_MS);
//RDMA_CM_EVENT_ROUTE_RESOLVED状态

//建立连接需要PD,CQ,QP
pd = ibv_alloc_pd(cm_id->verbs);
comp_chan = ibv_create_comp_channel(cm_id->verbs);
cq = ibv_create_cq(cm_id->verbs, 2, NULL, comp_chan, 0);
ibv_req_notify_cq(cq, 0);
//分配内存，注册
buf = calloc(2, sizeof(uint32_t));
mr = ibv_reg_mr(pd, buf, 2 * sizeof(uint32_t), IBV_ACCESS_LOCAL_WRITE);

qp_attr.cap.max_send_wr = 2;
qp_attr.cap.max_send_sge = 1;
qp_attr.cap.max_recv_wr = 1;
qp_attr.cap.max_recv_sge = 1;

qp_attr.send_cq = cq;
qp_attr.recv_cq = cq;
qp_attr.qp_type = IBV_QPT_RC;
//创建QP
err = rdma_create_qp(cm_id, pd, &qp_attr);
if (err) return err;

conn_param.initiator_depth = 1;
conn_param.retry_count = 7;

/* 連接至伺服器 */
err = rdma_connect(cm_id, &conn_param);
```
### 接收数据
接收数据就是调用`ibv_post_recv`，从内存中读取即可
对于receive方式，接收到数据会有事件通知
```
sge.addr = (uintptr_t)buf + sizeof(uint32_t);
sge.length = sizeof(uint32_t);
sge.lkey = mr->lkey;

recv_wr.sg_list = &sge;
recv_wr.num_sge = 1;
//sge.addr有数据到达会通知
if (ibv_post_recv(cm_id->qp, &recv_wr, &bad_recv_wr)) return 1;
```

### 发送数据
将数据填充到合适的buffer，调用`ibv_post_send`发送即可。

```
sge.addr = (uintptr_t)buf;
sge.length = sizeof(uint32_t);
sge.lkey = mr->lkey;

send_wr.opcode = IBV_WR_SEND;
send_wr.send_flags = IBV_SEND_SIGNALED;
send_wr.sg_list = &sge;
send_wr.num_sge = 1;

if (ibv_post_send(cm_id->qp, &send_wr, &bad_send_wr)) return 1;
```

