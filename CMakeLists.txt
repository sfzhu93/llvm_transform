cmake_minimum_required(VERSION 3.19)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

project(LlvmTransformPasses)

find_package(LLVM 16 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(${LLVM_DEFINITIONS})

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(include)

# file(GLOB SOURCES "src/*.cpp")

add_library(MyPeepholeAdd SHARED "src/ph_add.cpp")
add_library(MbaAdd SHARED "src/mba_add.cpp")
add_library(MyConstProp SHARED "src/cp.cpp")

# include(AddLLVM)
# add_llvm_pass_plugin(static_wp_lava)
target_compile_definitions(MyPeepholeAdd PRIVATE ${LLVM_DEFINITIONS})
target_compile_definitions(MbaAdd PRIVATE ${LLVM_DEFINITIONS})
target_compile_definitions(MyConstProp PRIVATE ${LLVM_DEFINITIONS})

# llvm_map_components_to_libnames(llvm_libs support core)
# target_link_libraries(static_wp_lava PRIVATE ${llvm_libs} z3)
target_link_libraries(MyPeepholeAdd PRIVATE LLVM)
target_link_libraries(MbaAdd PRIVATE LLVM)
target_link_libraries(MyConstProp PRIVATE LLVM)

# Compiler options
if(NOT LLVM_ENABLE_RTTI)
    set_target_properties(MyPeepholeAdd PROPERTIES
    COMPILE_FLAGS "-fno-rtti")
    set_target_properties(MbaAdd PROPERTIES
    COMPILE_FLAGS "-fno-rtti")
    set_target_properties(MyConstProp PROPERTIES
    COMPILE_FLAGS "-fno-rtti")
endif()
