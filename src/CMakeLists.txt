set (RDMA_SRC 
  Configuration.cpp
  debug.cpp
  RdmaSocket.cpp
  )

add_library(rdmalib ${RDMA_SRC})

target_link_libraries(rdmalib rdmacm ibverbs pthread)
