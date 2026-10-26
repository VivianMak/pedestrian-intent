#include "simulation.hpp"
#include "scenario_io.hpp"
#include "viterbi.hpp"

#include <print>

int main(){

    // Adjust simulation params
    SimModel::SimulationParams params{
        .name            = "scenario_test_not_cross", // scenario_test_cross
        .intent          = HMM_Params::States::NOT_CROSS,
        .dt              = 1,   // don't change
        .total_time      = 40,
        .signal_duration = 10,
        .init_vel        = 0.1,
        .curb_distance   = 30,
        .seed            = 0,
    };

    // Create scenario
    SimModel::Scenario scenario = SimModel::simulate(params);
    ScenarioIO::write_scenario(scenario, "data/scenarios.json");

    // Print simulated scenario
    // for (std::size_t i = 0; i < scenario.true_states.size(); ++i){
    //     const auto& obs = scenario.observations[i];
    //     std::println("t={:2}  {:9}  signal={}  dist={:5.2f}  vel={:.2f}",
    //                 i * params.dt,
    //                 SimModel::state_to_string(scenario.true_states[i]),
    //                 obs.walk_signal, obs.distance_to_curb, obs.velocity);
    // }


    Viterbi::Result hidden = Viterbi::decode(scenario.observations);

    // Print the true state next to the Viterbi prediction, x marks a wrong prediction
    std::println("\n{:4}  {:9}  {:9}  {:1}  {:6}  {:>5}  {}",
                "t", "true", "predicted", "", "signal", "dist", "vel");
    for (std::size_t i = 0; i < scenario.true_states.size(); ++i){
        const auto& obs = scenario.observations[i];
        bool correct = hidden.path[i] == scenario.true_states[i];
        std::println("t={:2}  {:9}  {:9}  {:1}  {:6}  {:5.2f}  {:.2f}",
                    i * params.dt,
                    SimModel::state_to_string(scenario.true_states[i]),
                    SimModel::state_to_string(hidden.path[i]),
                    correct ? " " : "x",
                    obs.walk_signal, obs.distance_to_curb, obs.velocity);
    }


    // Calculate accuracy
    double acc = Viterbi::accuracy(hidden.path, scenario.true_states);
    std::println("\naccuracy: {:.1f}%", acc * 100);
    std::println("log probability of the Viterbi path: {:.4f}", hidden.log_prob);

    // Save the prediction, its accuracy and log probability next to the scenario in the json
    ScenarioIO::write_viterbi_intent(params.name, hidden.path, acc, hidden.log_prob, "data/scenarios.json");

    return 0;
}
