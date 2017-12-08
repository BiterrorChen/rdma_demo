#项目名称  
PROJECT(rdma_demo)  
#声明要求的最小版本  
cmake_minimum_required(VERSION 2.6)  

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
  
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O0")  
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O0")  

add_subdirectory(RDMA_example_application)
add_subdirectory(./example2/01_basic-client-server)
add_subdirectory(./example2/02_read-write)
