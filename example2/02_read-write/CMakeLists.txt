add_executable(example2_server2 rdma-server.c rdma-common.c)
add_executable(example2_client2 rdma-client.c rdma-common.c)

target_link_libraries(example2_server2 rdmacm ibverbs pthread)
target_link_libraries(example2_client2 rdmacm ibverbs pthread)
