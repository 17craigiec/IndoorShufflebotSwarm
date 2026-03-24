cmake_minimum_required(VERSION 3.10)
project(ShuffleServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add debug symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../msgs
)

# Source files
set(SOURCES
    ShuffleServer.cc
    shuttleservermain.cc
)

# Create executable
add_executable(shuffleserver ${SOURCES})

# Link libraries if needed (e.g., pthread for networking)
target_link_libraries(shuffleserver PRIVATE pthread)
