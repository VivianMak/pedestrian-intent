#include "simulation.hpp"
#include <nlohmann/json.hpp>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <random> 

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
    double sim_vel(HMM_Params::States intent, double vel){

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
            vel = sim_vel(state, vel);
            dist = std::max(dist - vel * params.dt, 0.0); 

        }

        return output;
    }


    
    /**
     * AI Implementation of writing to json file
     * 
     *  "scenarios": {
     *      "<name>": { description, intent, params, observations, true_states },
     *      ...
     *  }
     */

    // Helpers only used for writing json, so they stay private to this file
    namespace {

        // ordered_json keeps keys in the order they are inserted
        // (plain nlohmann::json sorts them alphabetically)
        using json = nlohmann::ordered_json;

        // json has no built-in enum -> string, so convert the hidden state by hand
        const char* state_to_string(HMM_Params::States state){
            switch(state){
                case HMM_Params::States::CROSS:     return "CROSS";
                case HMM_Params::States::NOT_CROSS: return "NOT_CROSS";
                default:                            return "UNKNOWN";
            }
        }

        // json stores floats as doubles, so 10.3f would be printed as 10.300000190734863
        // rounding to 4 decimals keeps the file readable
        double round4(float value){
            return std::round(static_cast<double>(value) * 1e4) / 1e4;
        }
    }

    void write_scenario(const Scenario& scenario, const std::string& path){
        const SimulationParams& p = scenario.params;

        // ---- Build this scenario's entry ----
        json entry;
        entry["name"] = p.name;
        entry["description"] = "";      // left empty on purpose, fill in by hand afterwards
        entry["intent"] = state_to_string(p.intent);

        // Input params (name and intent are already written above, so they are not repeated)
        entry["params"] = {
            {"dt",              p.dt},
            {"total_time",      p.total_time},
            {"signal_duration", p.signal_duration},
            {"init_vel",        p.init_vel},
            {"curb_distance",   p.curb_distance},
            {"seed",            p.seed}
        };

        // One object per timestep. Start from an empty array so an empty list
        // is written as [] instead of null
        entry["observations"] = json::array();
        for (const HMM_Params::Observation& obs : scenario.observations){
            entry["observations"].push_back({
                {"walk_signal",      obs.walk_signal},
                {"distance_to_curb", round4(obs.distance_to_curb)},
                {"velocity",         round4(obs.velocity)}
            });
        }

        // Ground truth hidden state at each timestep, same length as observations
        entry["true_states"] = json::array();
        for (const HMM_Params::States state : scenario.true_states){
            entry["true_states"].push_back(state_to_string(state));
        }

        // ---- Merge into the existing file ----
        // Each run adds one scenario, so load what is already there first.
        // A scenario with the same name replaces the old entry instead of duplicating it
        json file;
        {
            std::ifstream in(path);
            // Missing or empty file -> start fresh. A malformed file throws
            // json::parse_error before anything is overwritten
            if (in.is_open() && in.peek() != std::ifstream::traits_type::eof()){
                in >> file;
            }
        }
        file["scenarios"][p.name] = entry;

        // ---- Write the whole file back, indented by 4 spaces ----
        std::ofstream out(path);
        if (!out.is_open()){
            throw std::runtime_error("write_scenario: could not open " + path + " for writing");
        }
        out << file.dump(4) << '\n';
    }
}
