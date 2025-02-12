# add_subdirectory(mysock)
# add_subdirectory(myhttp)

add_executable(myhttpd)
target_include_directories(myhttpd PUBLIC ${MY_INCS})
target_link_directories(myhttpd PUBLIC ${MY_LIBS})
target_sources(myhttpd PRIVATE main.cpp)
# target_link_libraries(myhttpd PRIVATE mysock PRIVATE myhttp)
