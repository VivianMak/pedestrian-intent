#include "simulation.hpp"

int main(){
    SimModel::SimulationParams params = SimModel::init_sim_params();
    SimModel::Scenario scenario = SimModel::simulate(params);
    SimModel::write_scenario(scenario, "data/scenarios.json");

    return 0;
}
