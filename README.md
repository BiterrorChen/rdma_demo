# rdma_demo
Learn rdma programming

---
example1:

server :

$ ./server

client : ( syntax:  client [servername] [val1] [val2] )

$./client  192.168.1.2  123 567

123 + 567 = 690
---

example2/01:
server : default port is 10086

$ ./server

client :

$./client  192.168.1.2  10086

---
example2/02:
server : default port is 10086

$ ./server read/write

client

$./client read/write 192.168.1.2  10086

---
example2/03:  
server :   
$ ./server

client

$./client server-address file-name

---

src:
### for write_imm
./server_write port_num   
./client_write ip port_num write_times

### for send/recv
./server port_num   
./client ip port_num write_times



