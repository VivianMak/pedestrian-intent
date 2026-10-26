#pragma once

#include "params.hpp"
#include "simulation.hpp"

namespace Viterbi{

    using namespace HMM_Params;

    struct Result{
        double prob;
        std::vector<States> path;
    };

    double emission_log_prob(States state, const Observation& obs);

    Result decode(const std::vector<Observation>& obs);


} //namespace Viterbi