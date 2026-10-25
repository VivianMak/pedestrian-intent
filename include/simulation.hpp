#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "params.hpp"

namespace SimModel{

    // Input: defines one scenario. Set the fields by name in main.cpp (.init_vel = 0.2, ...),
    // anything left out keeps the default below. Named fields must be listed in this order
    struct SimulationParams{

        std::string name;

        HMM_Params::States intent = HMM_Params::States::CROSS;

        int dt = 1;                     // s
        int total_time = 30;            // s

        int signal_duration = 30;       // s, GO <-> STOP

        double init_vel = 0.2;          // m/s
        double curb_distance = 10.5;    // m

        std::uint32_t seed = 0;         // same seed -> same trajectory

    };

    // Output: params + generated data, one entry per timestep
    struct Scenario{

        SimulationParams params;

        std::vector<HMM_Params::Observation> observations;
        std::vector<HMM_Params::States> true_states;    // ground truth for scoring Viterbi

    };

    bool walk_signal(int t, int period);
    Scenario simulate(const SimulationParams& params);
    void write_scenario(const Scenario& scenario, const std::string& path);

}
