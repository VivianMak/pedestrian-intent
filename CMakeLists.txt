cmake_minimum_required(VERSION 3.10)
project(PedestrianIntent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# nlohmann/json: installed on the system 
find_package(nlohmann_json 3.2.0 REQUIRED)


##### Build Libraries #####

# Simulation
add_library(sim STATIC src/simulation.cpp)
target_include_directories(sim PUBLIC include)

# Scenario IO: writes and reads scenarios as json
add_library(scenario_io STATIC src/scenario_io.cpp)
target_link_libraries(scenario_io
    PUBLIC  sim
    PRIVATE nlohmann_json::nlohmann_json)

# Viterbi
add_library(viterbi STATIC src/viterbi.cpp)
target_link_libraries(viterbi PUBLIC sim)


# enable_testing()
# add_subdirectory(tests)

# Main program executable
add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE sim scenario_io viterbi)
