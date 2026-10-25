#pragma once

#include <array>
#include <cmath>

namespace HMM_Params{

    using std::log;

    // Hidden states
    enum struct States{
        CROSS,
        NOT_CROSS,
        Count
    };

    struct State{
        double cross;
        double not_cross;
    };

    // Observations
    enum struct Observations{
        SIGNAL,
        DISTANCE,
        VELOCITY,
        // GAZE_ANGLE
        Count
    };

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


    // Transition Matrix (A) -> Row: Current State, Column: Next State
    constexpr std::array<std::array<double, NUM_OBSERVATIONS>, NUM_OBSERVATIONS> 
    TransitionProbs = {{
        {log(0.85), log(0.15)},     // CROSS -> {CROSS (0.85), NOT_CROSS(0.15)}
        {log(0.25), log(0.75)}      // NOT_CROSS -> {CROSS (0.9), NOT_CROSS(0.1)}
    }};

    // Emission Matrix (B) -> Row: State, Column: Observation
    constexpr std::array<std::array<double, NUM_STATES>, NUM_OBSERVATIONS> 
    EmissionProbs = {{
        {log(0.6), log(0.2), log(0.2)}, // CROSS -> {SIGNAL (0.6), DISTANCE (0.2), VELOCITY (0.2)}
        {log(0.1), log(0.4), log(0.5)}  // NOT_CROSS -> {SIGNAL (0.1), DISTANCE (0.4), VELOCITY (0.5)}
    }};

} // namespace HMM_Params