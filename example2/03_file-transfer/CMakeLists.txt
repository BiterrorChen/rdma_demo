add_executable(example2_server3 server.c common.c)
add_executable(example2_client3 client.c common.c)

target_link_libraries(example2_server3 rdmacm ibverbs pthread)
target_link_libraries(example2_client3 rdmacm ibverbs pthread)
