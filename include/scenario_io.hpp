#pragma once

#include <string>

#include "simulation.hpp"

// Saving and loading scenarios as json. The only part of the project that uses nlohmann/json
namespace ScenarioIO{

    // Add one scenario to the json file at path (replaces an existing entry with the same name)
    void write_scenario(const SimModel::Scenario& scenario, const std::string& path);

    // Load the scenario called name from the json file at path
    SimModel::Scenario read_scenario(const std::string& name, const std::string& path);

    // Fill in "predicted_intent" (Viterbi's path, one state per timestep) and "accuracy"
    // (fraction correct, 0 to 1) for the scenario called name. The scenario must already be in the file
    void write_viterbi_intent(const std::string& name,
                              const std::vector<HMM_Params::States>& predicted,
                              double accuracy,
                              const std::string& path);

}
