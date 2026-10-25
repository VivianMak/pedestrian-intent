# Predict Pedestrian Intent at Crosswalks

> Vivian Mak | Data Structure and Algorithm 

Using the Viterbi algorithm for hidden Markov models applied to a pedestrian-intent crosswalk scenario.



## Defining a Statistical Model

File: [pedestiran-intent/include/params.hpp](include/params.hpp)

- Using a Log Prob https://en.wikipedia.org/wiki/Log_probability
    - using log probabilities for computational efficiency and avoid float underflow where it rounds a low number to 0
    - adding vs multiplying prob

### Transition Probabilities

| Prev States | CROSS | NOT_CROSS |
| :---: | :---: | :---: |
| CROSS| 0.85 | 0.15 |
| NOT_CROSS | 0.25 | 0.75 |

Wtih a previously derived `CROSS` state:
- The likelihood of transitioning to `CROSS` is high
- The likelihood of transitioning to `NOT_CROSS` is low

Wtih a previously derived `NOT_CROSS` state:
- The likelihood of transitioning to `CROSS` is low
- The likelihood of transitioning to `NOT_CROSS` is high

### Emission Probabilities

| States | SIGNAL | DISTANCE | VELOCITY |
| :---: | :---: | :---: | :---: |
| CROSS| 0.5 | 0.2 | 0.3 |
| NOT_CROSS | 0.1 | 0.4 | 0.5 |


For a `CROSS` state,

- The signal has the highest contribution, because pedestrians will often only cross when `SIGNAL = True`
- The distance partially contributes, because there is a non-zero chance a pedestrian will cross even if they are approaching
- The velocity is the second highest contributor, because a pedestrian increasing in velocity is assumed to try to cross

For a `NOT_CROSS` state,
- The signal has a low contribution to account for pedestrians that cross anyway
- The distance to the curb and velocity will be the most important to determine whether a pedestrian intents to `NOT_CROSS` by slowing down or vice versa


## Simulating Observations

File: [include/simulation.hpp](include/simulation.hpp)

Set a simulation goal of recovering `CROSS` or `NOT_CROSS`

### Simulating Pedestrian Signal

Defined by a bool of the following states:
- `GO` - (`True`)
- `STOP` - (`False`) 

Transitions every 20 seconds.


### Simulating Velocity

Approach 1: Gaussian distribution, independent samples
- drawing random samples from the distribution
- but this lacks correlation to previous time step
- clamps speed to min/max

Approach 2: Gaussian noise, w/ random vel spikes
- update the previous speed with some gaussian noise
- add a random occasional increase/decrease in speed to simulate running/slowing down
- set intial speed to some random vel (m/s) capping max and min

Approach 3: Subcategories within velocity
- creaate velocity states: slow-walking, walking, running
- define tri-model distribution
- randomness to jump btwn means

## Next Steps
- Refine the observation models (ie how long until the signal countdowns to 0, incorporate other objects like approaching vehicles)

## Resources
- https://medium.com/@satyamsettiveerapraveen/viterbi-algorithm-62b0434f831d
- https://en.wikipedia.org/wiki/Viterbi_algorithm