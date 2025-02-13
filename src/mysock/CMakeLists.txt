add_library(mysock "")
target_include_directories(mysock PUBLIC ${MY_INCS})
target_sources(mysock PRIVATE configure.cpp PRIVATE sockets.cpp)
