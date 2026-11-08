cmake_minimum_required(VERSION 3.12)
project(SmartAssignmentScheduler VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(scheduler_core
    src/scheduler.cpp
)

add_executable(smart_scheduler
    src/main.cpp
)
target_link_libraries(smart_scheduler PRIVATE scheduler_core)

enable_testing()
add_executable(test_scheduler
    tests/test_scheduler.cpp
)
target_link_libraries(test_scheduler PRIVATE scheduler_core)

add_test(NAME UnitTests COMMAND test_scheduler)
