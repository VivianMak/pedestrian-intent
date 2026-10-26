#pragma once

#include <string>

#include "simulation.hpp"

// Saving and loading scenarios as json. The only part of the project that uses nlohmann/json
namespace ScenarioIO{

    // Add one scenario to the json file at path (replaces an existing entry with the same name)
    void write_scenario(const SimModel::Scenario& scenario, const std::string& path);

    // Load the scenario called name from the json file at path
    SimModel::Scenario read_scenario(const std::string& name, const std::string& path);

}
