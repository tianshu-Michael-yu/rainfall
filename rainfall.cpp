#include <iostream>
#include <fstream>
#include <cstdint>
#include <chrono>
#include "rainfall_simulation.h"
#include "io_processing.h"


template <typename T>
inline void printMatrix(const T *matrix, const size_t dim_landscape) {
    for (size_t i = 0; i < dim_landscape; ++i) {
        for (size_t j = 0; j < dim_landscape; ++j) {
            std::cout << matrix[i*dim_landscape + j] << " ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char * argv[]) {
    size_t num_threads = 1;
    // get all the arguments
    #ifdef SEQ
    if (argc != 5) {
        std::cout << "Usage: " << argv[0] << " <rain_time> <absorption_rate> <dim_landscape> <landscape_file>" << std::endl;
        return EXIT_FAILURE;
    }
    const size_t rain_time = atoi(argv[1]);
    const float absorption_rate = atof(argv[2]);
    const size_t dim_landscape = atoi(argv[3]);
    std::ifstream landscape_file(argv[4], std::ifstream::in);
    #else
    if (argc != 6) {
        std::cout << "Usage: " << argv[0] << " <num_threads> <rain_time> <absorption_rate> <dim_landscape> <landscape_file>" << std::endl;
        return EXIT_FAILURE;
    }
    num_threads = atoi(argv[1]);
    const size_t rain_time = atoi(argv[2]);
    const float absorption_rate = atof(argv[3]);
    const size_t dim_landscape = atoi(argv[4]);
    std::ifstream landscape_file(argv[5], std::ifstream::in);
    #endif
    // read in the landscape and establish the elevation map
    if (landscape_file.fail()) {
        std::cout << "Error: cannot open file " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }
    int *elevationMap = createElevationMap(landscape_file, dim_landscape);
    // set up water above the ground (currently)
    float *waterAboveGround  = new float[dim_landscape*dim_landscape]();
    float *waterAbsorbed     = new float[dim_landscape*dim_landscape]();
    // loop through time until the water is all absorbed
    // record wall clock time
    auto start_time = std::chrono::system_clock::now();
    const size_t num_steps = simulateRainFall(waterAboveGround, waterAbsorbed, elevationMap, rain_time, absorption_rate, dim_landscape, num_threads);
    auto end_time = std::chrono::system_clock::now();
    std::chrono::duration<double> run_time = end_time - start_time;
    float run_time_float = run_time.count();
    // print the abosrbed water 
    printOutput(waterAbsorbed, dim_landscape, num_steps, run_time_float);
    // clean up
    delete[] waterAboveGround;
    delete[] waterAbsorbed;
    delete[] elevationMap;
    return EXIT_SUCCESS; 
}