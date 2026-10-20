#pragma once

namespace SimModel{

    int time;

    bool walkSignal = ((int)(time / 30.0) % 2 == 0);
}

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


