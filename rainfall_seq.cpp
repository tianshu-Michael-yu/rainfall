#include <iostream>
#include <fstream>
#include <cstdint>
#include <ctime>
#include "rainfall_simulation.h"
#include "io_processing.h"


int main(int argc, char * argv[]) {
    // get all the arguments
    const size_t rain_time = atoi(argv[1]);
    const float absorption_rate = atof(argv[2]);
    const size_t dim_landscape = atoi(argv[3]);
    std::ifstream landscape_file(argv[4], std::ifstream::in);
    // read in the landscape and establish the elevation map
    int *elevationMap = createElevationMap(landscape_file, dim_landscape);
    // set up water above the ground (currently)
    float *waterAboveGround  = new float[dim_landscape*dim_landscape]();
    float *waterAbsorbed     = new float[dim_landscape*dim_landscape]();
    // loop through time until the water is all absorbed
    clock_t start_time = clock();
    const size_t num_steps = simulateRainFall(waterAboveGround, waterAbsorbed, elevationMap, rain_time, absorption_rate, dim_landscape);
    clock_t end_time = clock();
    float run_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;
    // print the abosrbed water 
    printOutput(waterAbsorbed, dim_landscape, num_steps, run_time);
    // clean up
    delete[] waterAboveGround;
    delete[] waterAbsorbed;
    delete[] elevationMap;
    return EXIT_SUCCESS; 
}