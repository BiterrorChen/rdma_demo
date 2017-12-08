add_executable(example1_server server.c)
add_executable(example1_client client.c)

target_link_libraries(example1_server rdmacm ibverbs pthread)
target_link_libraries(example1_client rdmacm ibverbs pthread)
