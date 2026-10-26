#include "viterbi.hpp"

#include <ranges>
#include <algorithm>
#include <iterator>
#include <cmath>

namespace Viterbi{

    using namespace HMM_Params;

    // Turn an enum (States, Observations) into an array index
    template <typename E>
    constexpr std::size_t idx(E e){
        return static_cast<std::size_t>(e);
    }

    /**
     * Based on the current state and list of observations
     * Calculate
     */
    double emission_log_prob(States state, const Observation& obs){

        double log_prob = 0.0;

        // Get the probabilities from emission table
        double p_signal = EmissionProbs[idx(state)][idx(Observations::SIGNAL)];
        double p_dist = EmissionProbs[idx(state)][idx(Observations::DISTANCE)];
        double p_vel = EmissionProbs[idx(state)][idx(Observations::VELOCITY)];
        
        // Add up all the probabilities
        // condition ? a : b
        log_prob += std::log(obs.walk_signal ? p_signal : 1.0 - p_signal);  // signal on?
        log_prob += std::log(obs.distance_to_curb >= FAR_DIST ? p_dist : 1.0 - p_dist); // distance far?
        log_prob += std::log(obs.velocity <= SLOW_VEL ? p_vel : 1.0 - p_vel);


        return log_prob;
    }


    /**
     * Given a list of observations, one per timestep
     * Fill in the table of hidden intents
     * Backtrace to find the optimal path
     */
    Result decode(const std::vector<HMM_Params::Observation>& obs){

        int T = obs.size();

        // Create empty table for hidden states and back pointers
        std::vector<std::array<double, NUM_STATES>> V(T);
        std::vector<std::array<size_t, NUM_STATES>> backptr(T);

        // Use inital probabilities for timestep 0
        for (int y = 0; y < NUM_STATES; y++){
            V[0][y] = std::log(InitialProbs[y]) + emission_log_prob(static_cast<States>(y), obs[0]);
        }

        // Main loop - through observations
        for (int t = 1; t < T; t++){

            // Loop through states
            for (int i = 0; i < NUM_STATES; i++){

                // Score each previous state to current state transition
                std::array<double, NUM_STATES> scores;
                for (std::size_t j = 0; j < NUM_STATES; j++){
                    // score = prev best + transition probability
                    scores[j] = V[t-1][j] + std::log(HMM_Params::TransitionProbs[j][i]);
                }

                // Find index of the highest score to map back to state
                auto it = std::ranges::max_element(scores);
                std::size_t best_prev = std::distance(scores.begin(), it);

                // Add emission probability to the current best path
                V[t][i] = *it + emission_log_prob(static_cast<States>(i), obs[t]);
                backptr[t][i] = best_prev;
            }
        }

        // Back tracking

        // find the final best score, in the last timestep
        auto it = std::ranges::max_element(V[T-1]);
        std::size_t best_last = std::distance(V[T-1].begin(), it);

        // copy down the states backwards
        std::vector<std::size_t> state_idx(T);
        state_idx[T-1] = best_last;

        for (int t = T - 1; t > 0; t--){
            // get the state that led to best last state ...
            state_idx[t - 1] = backptr[t][state_idx[t]];
        }


        Result result;
        result.path.reserve(T);
        for (int s : state_idx){
            result.path.push_back(static_cast<States>(s));
        }
        result.log_prob = V[T-1][best_last];

        return result;
    }


    /**
     * Calculate the accuracy of alignment of predicted and truth
     */
    double accuracy(const std::vector<States>& predicted,
                    const std::vector<States>& truth){
        if (predicted.empty() || predicted.size() != truth.size()) return 0.0;

        std::size_t correct = 0;

        // count all alignment
        for (std::size_t i = 0; i < predicted.size(); ++i){
            if (predicted[i] == truth[i]) ++correct;
        }

        // return percentage
        return static_cast<double>(correct) / predicted.size();
    }

} // namespace Viterbi



    