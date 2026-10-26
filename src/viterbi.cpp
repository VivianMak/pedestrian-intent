#include "viterbi.hpp"

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

                // set best to infinity
                double best = -std::numeric_limits<double>::infinity();
                std::size_t best_prev = 0;

                for (int j = 0; j < NUM_STATES; j++){
                    double score = V[t-1][i] + std::log(TransitionProbs[i][j]);

                    if (score > best){
                        best = score;
                        best_prev = j;
                    }
                }

                V[t][i] = best + emission_log_prob(static_cast<States>(i), obs[t]);
            }
        }

        // Back tracking
    }


} // namespace Viterbi


