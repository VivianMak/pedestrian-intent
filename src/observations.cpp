#include "observations.hpp"

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

    double dt = 1.0;
    double time = 0.0;

    double signal_duration = 30.0;

    bool walkSignal = ((int)(time / signal_duration) % 2 == 0);


    SimulationParams init_sim_params(){
        // Define scenario params
        SimulationParams scenario;

        scenario.intent = HMM_Params::States::CROSS;
        scenario.dt = 1.0;
        scenario.total_time = 30.0;
        scenario.signal_duration = 5.0;
        scenario.init_vel = 0.2;
        scenario.curb_distance = 10.5;

    }

    void sim_observations(){

    }

    void write_simulation(){
        // write to json
    }
}
