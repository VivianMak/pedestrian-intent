#pragma once

#include <cstdint>
#include <random>
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

    const char* state_to_string(HMM_Params::States state);

    bool walk_signal(int t, int period);
    double sim_vel(HMM_Params::States intent, double vel, std::mt19937& rng);
    HMM_Params::States sample_next_state(HMM_Params::States current, std::mt19937& rng);
    Scenario simulate(const SimulationParams& params);

}
