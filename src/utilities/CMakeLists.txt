add_library(utilities "")
target_include_directories(utilities PUBLIC ${MY_INCS})
target_sources(utilities PRIVATE mycaching.cpp)
