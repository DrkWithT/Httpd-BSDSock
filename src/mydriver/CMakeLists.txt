add_library(mydriver "")
target_include_directories(mydriver PUBLIC ${MY_INCS})
target_sources(mydriver PRIVATE task_queue.cpp PRIVATE entry_job.cpp PRIVATE worker_job.cpp PRIVATE driver.cpp)
