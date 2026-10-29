# Predict Pedestrian Intent at Crosswalks

> Vivian Mak \
> Data Structure and Algorithm Course

Using the Viterbi algorithm for hidden Markov models applied to a pedestrian-intent crosswalk scenario. In this algorithm, the goal is to predict with quantified uncertainty of whether a pedestrian approaching a crosswalk is intending to cross or not cross. The simulated "observations" could include factors like the velocity of the pedestrian (float), distance to curb (float), pedestrian signal (True / False),. The emission probabilities will be defined by the likehoods of the observed states relating back to the cross / not cross intent at each timestep of the moving pedestrian. When looking for the next hidden state or intent, we take the previous time step's prediction into account to derive the "most likely" transition of intent.

Deliverables:

- Generate python visualizations of observations vs intent
- Scenario toy problems by hard coding a set of probabilities of high likelihood of crossing / not crossing
- Implement Viterbi algorithm to infer most likely sequence of intent states from simulated observation
- Github repo w/ readme, documentation, design decisions


My workflow:

1. Generate simulation and log scenario
2. Run on Viterbi
3. Compare intent paths
4. Plot visualization
5. Adjust parameters

## Defining a Statistical Model

File: [pedestiran-intent/include/params.hpp](include/params.hpp)

- Probabilities are stored as plain values (each row sums to 1)
    - easy to read and check, and the simulation samples directly from them
- Viterbi converts them to log probabilities https://en.wikipedia.org/wiki/Log_probability
    - Viterbi multiplies one probability per timestep along a path, so the product underflows (rounds to 0) on long sequences
    - log turns multiplying into adding: log(a * b) = log(a) + log(b), and a sum of logs doesn't shrink to 0

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
| CROSS| 0.6 | 0.5 | 0.1 |
| NOT_CROSS | 0.4 | 0.5 | 0.8 |


For a `CROSS` state,

- The signal has the highest contribution, because pedestrians will often only cross when `SIGNAL = True`
- The distance partially contributes, because there is a non-zero chance a pedestrian will cross even if they are approaching
- The velocity is the second highest contributor, because a pedestrian increasing in velocity is assumed to try to cross

For a `NOT_CROSS` state,
- The signal has a low contribution to account for pedestrians that cross anyway
- The distance to the curb and velocity will be the most important to determine whether a pedestrian intents to `NOT_CROSS` by slowing down or vice versa

The probabilities here don't sum up to 1 because each observation has multiple "substates". For example, the binary signal has GO or STOP states, the distance is defined by CLOSE or NOT CLOSE through `constexpr double FAR_DIST`.


## Simulating Observations

File: [include/simulation.hpp](include/simulation.hpp)

Set a simulation goal of recovering `CROSS` or `NOT_CROSS`

### Simulating Pedestrian Signal

Defined by a bool of the following states:
- `GO` - (`True`)
- `STOP` - (`False`) 

Transitions every x seconds.


### Simulating Velocity

I brainstormed several approaches to simulaiting the velocity, and decided to go with approach 2.

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

### Simulating Distance

We simply use the equation `dist = dist + (velocity * dt)`. The distance is controlled by the velocity.

### Logging Simulations

The simulations are logged in [data/scenarios.json](data/scenarios.json) using the `nlohmann_json` library. The data fields logged include:
- name of scenario
- description (optional)
- intent
- simulation params
- observation list
- true states
- predicted intent (filled in after from viterbi)
- accuracy

## Viterbi Algorithm

1. Use dynamic programming to build matrix of size 2 x N_observations
2. Initialize the first timestep's probability to 50/50 to capture unknown
3. For each timestep,
4. For each possible hidden state,
5. Find the probability of a transition from past timestep's state `prob_t = prob_t-1[state] + log(tranition_probability)`
6. Find the best path from t-1 timestep with highest probability
7. Compute a probability of hidden state based on observations
8. Calculate the probability of current state based on observations `prob_t-1[state] = prob_best_path + log(emission probability)`
9. Fill in a table with backpointers
10. Find highest probability at final timestep
11. Backtrack to find optimal path


## Testing and Results

## Next Steps
- Refine the observation models (ie how long until the signal countdowns to 0, incorporate other objects like approaching vehicles)
- A bigger emission table to account for the substates of observation categories (ie. Velocity = {slow, walking, running})

## Resources
- https://medium.com/@satyamsettiveerapraveen/viterbi-algorithm-62b0434f831d
- https://en.wikipedia.org/wiki/Viterbi_algorithm
- https://www.geeksforgeeks.org/artificial-intelligence/viterbi-algorithm-for-hidden-markov-models-hmms/

## AI Disclosure
The following functions are written with AI:
- [src/scenario_io.cpp](src/scenario_io.cpp) - Writing to and reading from json files
- [data/plot_scenarios.py](data/plot_scenarios.py) - Generate plots from json file