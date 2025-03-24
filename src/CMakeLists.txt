set(SRC_LIST testAllCache.cpp)

#添加可执行程序
ADD_EXECUTABLE(cacheProject ${SRC_LIST})
#添加动态库
ADD_LIBRARY(cache SHARED ${SRC_LIST})
#设置编译时动态库链接
TARGET_LINK_LIBRARIES(cache)