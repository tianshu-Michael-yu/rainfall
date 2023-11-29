#include "io_processing.h"
#include <iostream>

int *createElevationMap(std::ifstream &landscape_file, const size_t dim_landscape) {
    int *elevationMap = new int[dim_landscape*dim_landscape];
    for (size_t i = 0; i < dim_landscape*dim_landscape; ++i) {
        landscape_file >> elevationMap[i];
    }
    return elevationMap;
}

void printOutput(const float *waterAbsorbed, const size_t dim_landscape, const size_t num_steps, const float run_time) {
    std::cout << "Rainfall simulation completed in " << num_steps << " time steps" << std::endl;
    std::cout << "Runtimes: " << run_time << " seconds" << std::endl;

    std::cout << "The following grid shows the number of raindrops absorbed at each point: " << std::endl;
    for (size_t i= 0; i< dim_landscape; ++i) {
        for (size_t j = 0; j < dim_landscape; ++j) {
            std::cout << waterAbsorbed[i*dim_landscape + j] << " ";
        }
        std::cout << std::endl;
    }
}