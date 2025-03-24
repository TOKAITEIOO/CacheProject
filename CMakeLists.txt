#设置cmake的最低版本和项目名称
cmake_minimum_required(VERSION 3.0)
project(CacheProject)

#生成debug版本，可以进行gdb调试
SET(CMAKE_BUILD_TYPE "Dubug")
#设置项目可执行文件的输出路径
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置项目库文件的输出的路径
SET(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/lib)

#设置项目库文件的搜索
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)

#设置头文件的搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/example)

#设置项目的源文件搜索路径
ADD_SUBDIRECTORY(src)

