#pragma once

#include "params.hpp"
#include "simulation.hpp"

namespace Viterbi{

    using namespace HMM_Params;

    struct Result{
        double log_prob;
        std::vector<States> path;
    };

    double emission_log_prob(States state, const Observation& obs);

    Result decode(const std::vector<Observation>& obs);

    double accuracy(const std::vector<States>& predicted, const std::vector<States>& truth);


} //namespace Viterbi