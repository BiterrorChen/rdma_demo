add_executable(server server.c)
add_executable(client client.c)

target_link_libraries(server rdmacm ibverbs pthread)
target_link_libraries(client rdmacm ibverbs pthread)
