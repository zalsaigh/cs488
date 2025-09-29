cmake_minimum_required(VERSION 3.31)
project(cs488 CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

# Add shared subproject which builds framework
add_subdirectory(shared)

# Add the A0 subproject
add_subdirectory(A0)

message(STATUS "Top-level CMake setup complete.")
