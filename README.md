# Predict Pedestrian Intent at Crosswalks

> Vivian Mak | Data Structure and Algorithm 

Using the Viterbi algorithm for hidden Markov models applied to a pedestrian-intent crosswalk scenario.

Log Prob

https://en.wikipedia.org/wiki/Log_probability
- using log probabilities for computational efficiency and avoid float underflow where it rounds a low number to 0
- adding v multiplying prob

Emission Probabilities

| States | SIGNAL | DISTANCE | VELOCITY |
| :---: | :---: | :---: | :---: |
| CROSS| 0.6 | 0.2 | 0.2 |
| NOT_CROSS | 0.1 | 0.4 | 0.5 |