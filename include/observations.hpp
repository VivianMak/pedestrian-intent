#pragma once

#include "params.hpp"

namespace SimModel{

    struct SimulationParams{

        HMM_Params::States intent; //HMM_Params::States::CROSS

        double dt = 1.0;
        double total_time;

        double signal_duration;

        double init_vel;          // m/s
        double curb_distance;     // m

    };

    SimulationParams init_sim_params();

    void sim_observations();

    void write_simulation();
}
