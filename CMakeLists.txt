cmake_minimum_required(VERSION 3.10)
project(PedestrianIntent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# nlohmann/json: installed on the system 
find_package(nlohmann_json 3.2.0 REQUIRED)


##### Build Libraries #####

# Simulation: generates scenarios and writes them to json
add_library(sim STATIC src/simulation.cpp)
target_include_directories(sim PUBLIC include)                  # so "simulation.hpp" and "params.hpp" are found
target_link_libraries(sim PUBLIC nlohmann_json::nlohmann_json)  # PUBLIC: anything linking sim gets it too


# enable_testing()
# add_subdirectory(tests)

# Main program executable
add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE sim)
