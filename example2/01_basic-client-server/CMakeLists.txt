add_executable(example2_server1 server.c)
add_executable(example2_client1 client.c)

target_link_libraries(example2_server1 rdmacm ibverbs pthread)
target_link_libraries(example2_client1 rdmacm ibverbs pthread)
