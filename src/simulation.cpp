#include "simulation.hpp"
#include <algorithm>
#include <random>
#include <print>

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
   
    /**
     * Switch the walk signal every period
     */
    bool walk_signal(int t, int period){
        return ((t / period) % 2 == 0);
    }

    /**
     * Based on intent, give the next timestep's velocity
     */
    constexpr double MIN_VEL   = 0.1;
    constexpr double MAX_VEL   = 1.5;
    constexpr double VEL_NOISE = 0.05;   // random noise
    constexpr double VEL_DRIFT = 0.1;   // slow down or speed up vel based on intent

    double sim_vel(HMM_Params::States intent, double vel, std::mt19937& rng){

        // Consider intent
        double drift;
        if (intent == HMM_Params::States::CROSS){
            drift = VEL_DRIFT; 
        }else{
            drift = -VEL_DRIFT;
        }

        // Gaussian noise centred on drift
        std::normal_distribution<double> noise(drift, VEL_NOISE);
        vel += noise(rng);

        return std::clamp(vel, MIN_VEL, MAX_VEL);
    }

    /**
     * Based on transition probability table, simulate the true intent
     */
    HMM_Params::States sample_next_state(HMM_Params::States current, std::mt19937& rng){
        // Current gives the row number, the column over is the next state
        const auto& row = HMM_Params::TransitionProbs[static_cast<std::size_t>(current)];

        // Pick an index with those odds, then map it back to a state
        std::discrete_distribution<int> pick(row.begin(), row.end());
        return static_cast<HMM_Params::States>(pick(rng));
    }

    Scenario simulate(const SimulationParams& params){
        Scenario output;
        output.params = params;

        std::mt19937 rng(params.seed);

        // Define observations at t=0
        double dist = params.curb_distance;
        double vel = params.init_vel;
        HMM_Params::States state = params.intent; 

        for(int t = 0; t <= params.total_time; t += params.dt){

            // std::println("At timestep: {}; Current intent is: {}", t, state_to_string(state));
            
            HMM_Params::Observation obs;
            obs.walk_signal = walk_signal(t, params.signal_duration);
            obs.distance_to_curb = dist;
            obs.velocity = vel;

            // Add current observation to list
            output.observations.push_back(obs);
            output.true_states.push_back(state);

            // End scenario if pedestrian is at curb
            if (dist <= 0.0) break;

            // Update state, use transition probability table
            state = sample_next_state(state, rng);
            
            // Update distance and velocity for next time step
            vel = sim_vel(state, vel, rng);
            dist = std::max(dist - vel * params.dt, 0.0); 

        }

        return output;
    }


    // Enums have no built-in text form (for json or println), so convert the hidden state by hand.
    // Declared in simulation.hpp so main.cpp and scenario_io.cpp can use it too
    const char* state_to_string(HMM_Params::States state){
        switch(state){
            case HMM_Params::States::CROSS:     return "CROSS";
            case HMM_Params::States::NOT_CROSS: return "NOT_CROSS";
            default:                            return "UNKNOWN";
        }
    }
}
