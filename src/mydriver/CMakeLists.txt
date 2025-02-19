add_library(mydriver "")
target_include_directories(mydriver PUBLIC ${MY_INCS})
target_sources(mydriver PUBLIC driver.cpp)
