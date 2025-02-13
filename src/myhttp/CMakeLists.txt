add_library(myhttp "")
target_include_directories(myhttp PUBLIC ${MY_INCS})
target_sources(myhttp PRIVATE types.cpp PRIVATE intake.cpp PRIVATE outtake.cpp)
