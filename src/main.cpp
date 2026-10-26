#include "simulation.hpp"

#include <print>

int main(){

    // Edit the inputs here between runs. Anything left out keeps its default
    // (see SimulationParams in simulation.hpp). Fields must stay in declaration order
    SimModel::SimulationParams params{
        .name            = "scenario_test",
        .intent          = HMM_Params::States::NOT_CROSS,
        .dt              = 1,   // don't change
        .total_time      = 30,
        .signal_duration = 5,
        .init_vel        = 0.1,
        .curb_distance   = 1,
        .seed            = 0,
    };

    SimModel::Scenario scenario = SimModel::simulate(params);
    SimModel::write_scenario(scenario, "data/scenarios.json");


    for (std::size_t i = 0; i < scenario.true_states.size(); ++i){
        const auto& obs = scenario.observations[i];
        std::println("t={:2}  {:9}  signal={}  dist={:5.2f}  vel={:.2f}",
                    i * params.dt,
                    SimModel::state_to_string(scenario.true_states[i]),
                    obs.walk_signal, obs.distance_to_curb, obs.velocity);
    }

    return 0;
}
