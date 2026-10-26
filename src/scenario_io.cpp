#include "scenario_io.hpp"
#include <nlohmann/json.hpp>
#include <cmath>
#include <fstream>
#include <stdexcept>

/**
 * AI Implementation of writing to and reading from json file
 *
 *  "scenarios": {
 *      "<name>": { description, intent, params, observations, true_states,
 *                  predicted_intent, accuracy, viterbi_log_prob },
 *      ...
 *  }
 */

namespace ScenarioIO{

    // Helpers only used for json, so they stay private to this file
    namespace {

        // ordered_json keeps keys in the order they are inserted
        // (plain nlohmann::json sorts them alphabetically)
        using json = nlohmann::ordered_json;

        // json stores floats as doubles, so 10.3f would be printed as 10.300000190734863
        // rounding to 4 decimals keeps the file readable
        double round4(float value){
            return std::round(static_cast<double>(value) * 1e4) / 1e4;
        }

        // Inverse of SimModel::state_to_string, turns "CROSS" back into States::CROSS
        HMM_Params::States string_to_state(const std::string& text){
            if (text == "CROSS")     return HMM_Params::States::CROSS;
            if (text == "NOT_CROSS") return HMM_Params::States::NOT_CROSS;
            throw std::runtime_error("string_to_state: unknown state " + text);
        }
    }

    void write_scenario(const SimModel::Scenario& scenario, const std::string& path){
        const SimModel::SimulationParams& p = scenario.params;

        // ---- Build this scenario's entry ----
        json entry;
        entry["name"] = p.name;
        entry["description"] = "";      // left empty on purpose, fill in by hand afterwards
        entry["intent"] = SimModel::state_to_string(p.intent);

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
            entry["true_states"].push_back(SimModel::state_to_string(state));
        }

        // Viterbi's predicted state at each timestep, how many it got right, and the
        // log probability of that path. Left empty here, filled in later by write_viterbi_intent
        entry["predicted_intent"] = json::array();
        entry["accuracy"] = nullptr;            // written as null until Viterbi runs
        entry["viterbi_log_prob"] = nullptr;

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

    SimModel::Scenario read_scenario(const std::string& name, const std::string& path){

        // ---- Load the whole file ----
        std::ifstream in(path);
        if (!in.is_open()){
            throw std::runtime_error("read_scenario: could not open " + path + " for reading");
        }
        json file = json::parse(in);    // a malformed file throws json::parse_error

        // ---- Find this scenario's entry ----
        if (!file.contains("scenarios") || !file.at("scenarios").contains(name)){
            throw std::runtime_error("read_scenario: no scenario \"" + name + "\" in " + path);
        }
        const json& entry = file.at("scenarios").at(name);

        // .at() throws a clear "key not found" error if the file is missing a field
        SimModel::Scenario scenario;
        SimModel::SimulationParams& p = scenario.params;

        p.name = name;
        p.intent = string_to_state(entry.at("intent").get<std::string>());
        // description is not loaded, Scenario has no field for it

        // Input params
        const json& params = entry.at("params");
        p.dt              = params.at("dt").get<int>();
        p.total_time      = params.at("total_time").get<int>();
        p.signal_duration = params.at("signal_duration").get<int>();
        p.init_vel        = params.at("init_vel").get<double>();
        p.curb_distance   = params.at("curb_distance").get<double>();
        p.seed            = params.at("seed").get<std::uint32_t>();

        // One Observation per timestep (values come back rounded to 4 decimals, as written)
        for (const json& obs : entry.at("observations")){
            scenario.observations.push_back({
                obs.at("walk_signal").get<bool>(),
                obs.at("distance_to_curb").get<float>(),
                obs.at("velocity").get<float>()
            });
        }

        // Ground truth hidden state at each timestep, same length as observations
        for (const json& state : entry.at("true_states")){
            scenario.true_states.push_back(string_to_state(state.get<std::string>()));
        }

        return scenario;
    }

    void write_viterbi_intent(const std::string& name,
                              const std::vector<HMM_Params::States>& predicted,
                              double accuracy,
                              double log_prob,
                              const std::string& path){

        // ---- Load the whole file ----
        // The scenario must already be there (written by write_scenario)
        json file;
        {
            std::ifstream in(path);
            if (!in.is_open()){
                throw std::runtime_error("write_viterbi_intent: could not open " + path + " for reading");
            }
            file = json::parse(in);     // a malformed file throws json::parse_error before anything is overwritten
        }

        // ---- Find this scenario's entry ----
        if (!file.contains("scenarios") || !file.at("scenarios").contains(name)){
            throw std::runtime_error("write_viterbi_intent: no scenario \"" + name + "\" in " + path
                                     + ", write it with write_scenario first");
        }
        json& entry = file["scenarios"][name];

        // One prediction per timestep, so it has to line up with true_states
        const std::size_t num_steps = entry.at("true_states").size();
        if (predicted.size() != num_steps){
            throw std::runtime_error("write_viterbi_intent: got " + std::to_string(predicted.size())
                                     + " predictions but scenario \"" + name + "\" has "
                                     + std::to_string(num_steps) + " timesteps");
        }

        // ---- Fill in the prediction, its accuracy and log probability (replaces any earlier ones) ----
        entry["predicted_intent"] = json::array();
        for (const HMM_Params::States state : predicted){
            entry["predicted_intent"].push_back(SimModel::state_to_string(state));
        }
        entry["accuracy"] = round4(accuracy);   // fraction correct, 0 to 1
        // log probability can be large (e.g. -123.4567), so round as a double instead of round4's float
        entry["viterbi_log_prob"] = std::round(log_prob * 1e4) / 1e4;

        // ---- Write the whole file back, indented by 4 spaces ----
        std::ofstream out(path);
        if (!out.is_open()){
            throw std::runtime_error("write_viterbi_intent: could not open " + path + " for writing");
        }
        out << file.dump(4) << '\n';
    }
}
