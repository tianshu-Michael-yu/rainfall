#ifndef IO_PROCESSING_H_
#define IO_PROCESSING_H_
#include <fstream>
#include <cstddef>

int *createElevationMap(std::ifstream &landscape_file, const size_t dim_landscape);

void printOutput(const float *waterAbsorbed, const size_t dim_landscape, const size_t num_steps, const float run_time);

#endif