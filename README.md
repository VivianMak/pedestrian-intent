# Predict Pedestrian Intent at Crosswalks

> Vivian Mak | Data Structure and Algorithm 

Using the Viterbi algorithm for hidden Markov models applied to a pedestrian-intent crosswalk scenario.



## Defining a Statistical Model

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


## Next Steps
- Refine the observation models (ie how long until the signal countdowns to 0, incorporate other objects like approaching vehicles)