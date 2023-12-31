#ifndef RAINFALL_SIMULATION_H_
#define RAINFALL_SIMULATION_H_
#include <cstddef>

// return the number of steps taken for all the water to be absorbed
int simulateRainFall(float *waterAboveGround, float *waterAbsorbed, const int *elevation_map,
const size_t rain_time, const float absorption_rate, const size_t dim_landscape, const size_t num_threads);

#endif