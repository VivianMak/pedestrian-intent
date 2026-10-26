#include "viterbi.hpp"

#include <algorithm>
#include <iterator>

namespace Viterbi{

    using namespace HMM_Params;

    double emission_log_prob(States state, const Observation& obs);


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
                auto it = std::max_element(scores.begin(), scores.end());
                std::size_t best_prev = std::distance(scores.begin(), it);

                // Add emission probability to the current best path
                V[t][i] = *it + emission_log_prob(static_cast<States>(i), obs[t]);
                backptr[t][i] = best_prev;
            }
        }

        // Back tracking
    }


} // namespace Viterbi


