set (RDMA_SRC 
  Configuration.cpp
  debug.cpp
  RdmaSocket.cpp
  )

add_library(rdmalib ${RDMA_SRC})

target_link_libraries(rdmalib rdmacm ibverbs pthread)

add_executable(client client.cpp )
add_executable(server server.cpp )
target_link_libraries(client rdmacm ibverbs pthread rdmalib)
target_link_libraries(server rdmacm ibverbs pthread rdmalib)
