#include "simulation.hpp"

int main(){

    // Edit the inputs here between runs. Anything left out keeps its default
    // (see SimulationParams in simulation.hpp). Fields must stay in declaration order
    SimModel::SimulationParams params{
        .name            = "scenario_1",                // key in the json
        .intent          = HMM_Params::States::CROSS,
        .dt              = 1.0,                         // s
        .total_time      = 30.0,                        // s
        .signal_duration = 5.0,                         // s, GO <-> STOP
        .init_vel        = 0.2,                         // m/s
        .curb_distance   = 10.5,                        // m
        .seed            = 0,
    };

    SimModel::Scenario scenario = SimModel::simulate(params);
    SimModel::write_scenario(scenario, "data/scenarios.json");

    return 0;
}
