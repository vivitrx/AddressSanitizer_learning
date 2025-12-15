cmake_minimum_required(VERSION 3.10)
project(ToyASan VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -gdwarf-4 -fno-omit-frame-pointer -fPIC")

# Directories
set(TOY_ASAN_DIR src/toy_asan)
set(TESTS_DIR src/tests)
set(EXPERIMENTS_DIR src/experiments)

# Build toy ASan as a shared library
file(GLOB_RECURSE TOY_ASAN_SOURCES "${TOY_ASAN_DIR}/*.c")
add_library(toy_asan SHARED ${TOY_ASAN_SOURCES})

# Link libraries for toy_asan
target_link_libraries(toy_asan dl)

# Set output directory
set_target_properties(toy_asan PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Install target
install(TARGETS toy_asan
    LIBRARY DESTINATION lib
)

# Build test programs
file(GLOB TEST_SOURCES "${TESTS_DIR}/*.c")

foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    
    # Link with toy_asan library
    target_link_libraries(${test_name} toy_asan)

    # Set output directory for tests
    set_target_properties(${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    # Add custom target to run test with toy ASan
    add_custom_target(run_${test_name}
        COMMAND ${CMAKE_COMMAND} -E env 
                LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib
                LD_PRELOAD=${CMAKE_BINARY_DIR}/lib/libtoy_asan.so
                ${CMAKE_BINARY_DIR}/bin/${test_name}
        DEPENDS toy_asan ${test_name}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        COMMENT "Running ${test_name} with Toy ASan"
    )
endforeach()

# Add a target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_COMMAND} -E env 
            LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib
            LD_PRELOAD=${CMAKE_BINARY_DIR}/lib/libtoy_asan.so
            ${CMAKE_BINARY_DIR}/bin/*
    DEPENDS toy_asan
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running all tests with Toy ASan"
)

# Build experiments
if(EXISTS ${EXPERIMENTS_DIR})
    add_subdirectory(${EXPERIMENTS_DIR})
endif()

# Custom targets
add_custom_target(all_tests
    COMMENT "Building all test programs"
)

foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_dependencies(all_tests ${test_name})
endforeach()

# Print configuration
message(STATUS "Toy ASan Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "  Source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "  Binary dir: ${CMAKE_BINARY_DIR}")
