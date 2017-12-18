set(RDMA_SRC 
  RDMACMSocket.cpp 
  RDMACMServerSocket.cpp 
  HostAndPort.cpp 
  Buffer.cpp
  MessageHeader.cpp
  RDMAWriteImmServerSocket.cpp
  RDMAWriteImmSocket.cpp
  RDMAWriteServerSocket.cpp
  RDMAWriteSocket.cpp)

add_library(rdmalib ${RDMA_SRC})

target_link_libraries(rdmalib rdmacm ibverbs pthread)

add_executable(client client.cpp) 
add_executable(server server.cpp)
target_link_libraries(client rdmacm ibverbs pthread rdmalib)
target_link_libraries(server rdmacm ibverbs pthread rdmalib)

add_executable(client_write client_write.cpp) 
add_executable(server_write server_write.cpp)
target_link_libraries(client_write rdmacm ibverbs pthread rdmalib)
target_link_libraries(server_write rdmacm ibverbs pthread rdmalib)
