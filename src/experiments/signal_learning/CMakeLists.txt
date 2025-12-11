cmake_minimum_required(VERSION 3.10)
project(signal_learning C)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 设置编译选项 - 包含调试信息
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -Wall -Wextra")

# 添加编译定义，便于调试
add_definitions(-DDEBUG)

# 第一个实验：信号基础观察
add_executable(01_signal_basics 01_signal_basics/src/01_signal_basics.c)
target_include_directories(01_signal_basics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 第二个实验：信号处理器
add_executable(02_signal_handler 02_signal_handlers/src/02_signal_handler.c)
target_include_directories(02_signal_handler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 信号处理器调试版本
add_executable(02_signal_handler_debug 02_signal_handlers/src/02_signal_handler_debug.c)
target_include_directories(02_signal_handler_debug PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 信号处理器可观测版本
add_executable(02_signal_handler_observable 02_signal_handlers/src/02_signal_handler_observable.c)
target_include_directories(02_signal_handler_observable PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# sigaction简单测试
add_executable(test_sigaction_simple 02_signal_handlers/src/test_sigaction_simple.c)
target_include_directories(test_sigaction_simple PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


# 第三个实验：信号信息提取器
add_executable(03_signal_info_extractor 03_signal_communication/src/03_signal_info_extractor.c)
target_include_directories(03_signal_info_extractor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 设置输出目录
set_target_properties(01_signal_basics 02_signal_handler 02_signal_handler_debug 
                   02_signal_handler_observable test_sigaction_simple 03_signal_info_extractor
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# TODO: 后续实验
# add_executable(04_siginfo_demo 04_siginfo_demo.c)

# 打印调试信息
message(STATUS "Building signal learning experiments with debug flags")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "Debug Flags: ${CMAKE_C_FLAGS_DEBUG}")
