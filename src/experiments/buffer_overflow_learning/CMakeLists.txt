cmake_minimum_required(VERSION 3.10)
project(buffer_overflow_learning)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -Wextra")

# 可选：添加栈保护选项来对比效果
option(ENABLE_STACK_PROTECTION "Enable stack protection for comparison" OFF)
if(ENABLE_STACK_PROTECTION)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
    message(STATUS "Stack protection enabled")
else()
    message(STATUS "Stack protection disabled (vulnerable)")
endif()

# 栈溢出实验
add_executable(01_basic_stack_overflow 
    01_stack_overflow/src/01_basic_stack_overflow.c
)
target_include_directories(01_basic_stack_overflow PRIVATE 01_stack_overflow/src)

add_executable(02_return_address_overflow 
    01_stack_overflow/src/02_return_address_overflow.c
)
target_include_directories(02_return_address_overflow PRIVATE 01_stack_overflow/src)

add_executable(03_direct_stack_overflow 
    01_stack_overflow/src/03_direct_stack_overflow.c
)
target_include_directories(03_direct_stack_overflow PRIVATE 01_stack_overflow/src)

add_executable(04_successful_attack 
    01_stack_overflow/src/04_successful_attack.c
)
target_include_directories(04_successful_attack PRIVATE 01_stack_overflow/src)

add_executable(05_stack_protection_demo 
    01_stack_overflow/src/05_stack_protection_demo.c
)
target_include_directories(05_stack_protection_demo PRIVATE 01_stack_overflow/src)

# 堆溢出实验
add_executable(01_basic_heap_overflow 
    02_heap_overflow/src/01_basic_heap_overflow.c
)
target_include_directories(01_basic_heap_overflow PRIVATE 02_heap_overflow/src)

add_executable(02_metadata_corruption 
    02_heap_overflow/src/02_metadata_corruption.c
)
target_include_directories(02_metadata_corruption PRIVATE 02_heap_overflow/src)

# 溢出后果实验
add_executable(01_function_pointer_hijack 
    03_consequences/src/01_function_pointer_hijack.c
)
target_include_directories(01_function_pointer_hijack PRIVATE 03_consequences/src)

# 创建一个便利的测试目标
add_custom_target(run_all_experiments
    COMMAND echo "=== 运行所有缓冲区溢出实验 ==="
    COMMAND echo ""
    COMMAND echo "1. 栈溢出基础实验 (正常情况):"
    COMMAND ./01_basic_stack_overflow "short" || true
    COMMAND echo ""
    COMMAND echo "2. 栈溢出基础实验 (溢出情况):"
    COMMAND ./01_basic_stack_overflow "this_is_way_too_long" || true
    COMMAND echo ""
    COMMAND echo "3. 堆溢出基础实验 (正常情况):"
    COMMAND ./01_basic_heap_overflow "normal" || true
    COMMAND echo ""
    COMMAND echo "4. 堆溢出基础实验 (溢出情况):"
    COMMAND ./01_basic_heap_overflow "this_will_overflow_into_next_block" || true
    COMMAND echo ""
    COMMAND echo "5. 函数指针劫持实验 (正常情况):"
    COMMAND ./01_function_pointer_hijack "short" || true
    COMMAND echo ""
    COMMAND echo "6. 函数指针劫持实验 (尝试劫持):"
    COMMAND ./01_function_pointer_hijack "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" || true
    COMMAND echo ""
    COMMAND echo "=== 实验完成 ==="
    DEPENDS 01_basic_stack_overflow 02_return_address_overflow 
            01_basic_heap_overflow 02_metadata_corruption
            01_function_pointer_hijack
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "运行所有缓冲区溢出实验"
)

# 创建帮助目标
add_custom_target(help_experiments
    COMMAND ${CMAKE_COMMAND} -E echo "=== 缓冲区溢出实验使用指南 ==="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "栈溢出实验:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ./01_basic_stack_overflow \"<输入字符串>\""
    COMMAND ${CMAKE_COMMAND} -E echo "  ./02_return_address_overflow \"<输入字符串>\""
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "堆溢出实验:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ./01_basic_heap_overflow \"<输入字符串>\""
    COMMAND ${CMAKE_COMMAND} -E echo "  ./02_metadata_corruption <溢出长度>"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "后果演示实验:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ./01_function_pointer_hijack \"<输入字符串>\""
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "安全警告: 这些程序故意包含缓冲区溢出漏洞，仅用于学习目的！"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "显示实验使用指南"
)

# 安装目标
install(TARGETS 
    01_basic_stack_overflow 
    02_return_address_overflow
    01_basic_heap_overflow 
    02_metadata_corruption
    01_function_pointer_hijack
    RUNTIME DESTINATION bin
)

# 打印构建信息
message(STATUS "")
message(STATUS "=== 缓冲区溢出学习实验 ===")
message(STATUS "")
message(STATUS "构建完成后，可以运行以下命令:")
message(STATUS "  make help_experiments  # 显示使用指南")
message(STATUS "  make run_all_experiments  # 运行所有实验")
message(STATUS "")
message(STATUS "安全警告: 这些程序故意包含缓冲区溢出漏洞，仅用于学习目的！")
message(STATUS "")
