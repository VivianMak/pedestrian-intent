#include "observations.hpp"
#include <nlohmann/json.hpp>

/**
 * DISTANCE, VELOCITY
 * - To get distance, we just need to draw random samples of velocity
 * - Assume at time=0, we start at position=0
 * - Create a distribution for velocity to draw from, should be gaussian, to assume constant velocity?
 * - Calculate distance = velocity * time
 *
 * PEDESTRIAN SIGNAL
 * - Every 30 seconds, the signal transitions from GO -> STOP
 *
 * GAZE_ANGLE
 * - todo
 */

namespace SimModel{

    SimulationParams init_sim_params(){
        // Define scenario params
        SimulationParams scenario;

        scenario.name = "scenario_1";
        scenario.intent = HMM_Params::States::CROSS;
        scenario.dt = 1.0;
        scenario.total_time = 30.0;
        scenario.signal_duration = 5.0;
        scenario.init_vel = 0.2;
        scenario.curb_distance = 10.5;

        return scenario;
    }

    bool walk_signal(double t, double period){
        return ((int)(t / period) % 2 == 0);
    }

    Scenario simulate(const SimulationParams& params){
        Scenario output;
        output.params = params;

        double time = 0.0;
        bool walkSignal = walk_signal(time, params.signal_duration);

        return output;
    }

    void write_scenario(const Scenario& scenario, const std::string& path){
        // write to json
    }
}
