add_library(utilities "")
target_include_directories(utilities PUBLIC ${MY_INCS})
target_sources(utilities PRIVATE mycaching.cpp PRIVATE url_lexing.cpp PRIVATE url_parsing.cpp)
