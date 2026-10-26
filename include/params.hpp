#pragma once

#include <array>

namespace HMM_Params{

    // Hidden state labels
    enum struct States{
        CROSS,          // 0
        NOT_CROSS,      // 1
        Count           // 2
    };

    // Observation type labels
    enum struct Observations{
        SIGNAL,
        DISTANCE,
        VELOCITY,
        // GAZE_ANGLE
        Count
    };

    // Raw values observed at one timestep, produced by the simulation
    struct Observation{
        bool walk_signal;           // true/false
        float distance_to_curb;     // m
        float velocity;             // m/s
        // float GAZE_ANGLE;        // radians
    };


    constexpr std::size_t NUM_STATES =
        static_cast<std::size_t>(States::Count);

    constexpr std::size_t NUM_OBSERVATIONS =
        static_cast<std::size_t>(Observations::Count);


    // Initial Probabilities, 50/50
    constexpr std::array<double, NUM_STATES>
    InitialProbs = {0.5, 0.5};

    // Transition Matrix
    constexpr std::array<std::array<double, NUM_STATES>, NUM_STATES>
    TransitionProbs = {{
        {0.85, 0.15},       // CROSS -> {CROSS NOT_CROSS}
        {0.25, 0.75}        // NOT_CROSS -> {CROSS, NOT_CROSS}
    }};

    // Emission Matrix (B)
    constexpr std::array<std::array<double, NUM_OBSERVATIONS>, NUM_STATES>
    EmissionProbs = {{
        {0.5, 0.2, 0.3},    // CROSS -> {SIGNAL, DISTANCE, VELOCITY}
        {0.1, 0.4, 0.5}     // NOT_CROSS -> {SIGNAL, DISTANCE, VELOCITY}
    }};

    // Define this for reverse emission matrix
    constexpr double FAR_DIST = 3.0;
    constexpr double SLOW_VEL = 0.5;

} // namespace HMM_Params