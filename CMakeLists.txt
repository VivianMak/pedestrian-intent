cmake_minimum_required(VERSION 3.10)
project(PedestrianIntent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


##### Build Libraries #####

# MyModule
add_library(sim STATIC src/simulation.cpp)
target_link_libraries(sim PUBLIC nlohmann/json)


# enable_testing()
# add_subdirectory(tests)

# Main program executable
add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE sim)   # matrix_lib comes along transitively
