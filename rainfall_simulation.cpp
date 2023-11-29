#include "rainfall_simulation.h"
#include <iostream>
#include <algorithm>

struct LowestNeighbors {
    size_t indexes[4];
    size_t num;
};

inline void flow_to_neighbor(float *waterAboveGround, const int *elevation_map, float *waterAboveGroundCopy, 
const LowestNeighbors *lowestNeighbors,const size_t dim_landscape ) {
    // find the lowest neighbor for each cell, if equal elevation, evenly divide the water
    for (size_t ind = 0; ind < dim_landscape*dim_landscape; ++ind) {
        // calculate the amount of water that can flow to the each of the lowest neighbor 
        if (lowestNeighbors[ind].num == 0) {
            continue;
        }
        float waterLoss =  (waterAboveGroundCopy[ind] > 1.0F)? 1.0F : waterAboveGroundCopy[ind];
        waterAboveGround[ind] -= waterLoss;
        float waterToEachNeighbor = waterLoss / lowestNeighbors[ind].num;
        // update the water above ground for each neighbor
        for (size_t i = 0; i < lowestNeighbors[ind].num; ++i) {
            waterAboveGround[lowestNeighbors[ind].indexes[i]] += waterToEachNeighbor;
        }
    }
}

inline const LowestNeighbors *initLowestNeighbors(const int *elevation_map, const size_t dim_landscape) {
    LowestNeighbors *lowestNeighbors = new LowestNeighbors[dim_landscape*dim_landscape];
    #define IND(i,j) ((i)*dim_landscape + (j))
    // calculate the lowest neighbor for each cell
    for (size_t i=0; i<dim_landscape; ++i) {
        for (size_t j=0; j<dim_landscape; ++j) {
            // find the lowest elevation among the neighbors
            int lowestElevation = elevation_map[IND(i,j)];
            // left
            if (j > 0) {
                lowestElevation = std::min(lowestElevation, elevation_map[IND(i,j-1)]);
            }
            // right
            if (j < dim_landscape-1) {
                lowestElevation = std::min(lowestElevation, elevation_map[IND(i,j+1)]);
            }
            // up
            if (i > 0) {
                lowestElevation = std::min(lowestElevation, elevation_map[IND(i-1,j)]);
            }
            // down
            if (i < dim_landscape-1) {
                lowestElevation = std::min(lowestElevation, elevation_map[IND(i+1,j)]);
            }
            // store neighbors with lowest elevation
            // left
            if (j > 0 && elevation_map[IND(i,j-1)] == lowestElevation) {
                lowestNeighbors[IND(i,j)].indexes[lowestNeighbors[IND(i,j)].num] = IND(i,j-1);
                ++lowestNeighbors[IND(i,j)].num;
            }
            // right
            if (j < dim_landscape-1 && elevation_map[IND(i,j+1)] == lowestElevation) {
                lowestNeighbors[IND(i,j)].indexes[lowestNeighbors[IND(i,j)].num] = IND(i,j+1);
                ++lowestNeighbors[IND(i,j)].num;
            }
            // up
            if (i > 0 && elevation_map[IND(i-1,j)] == lowestElevation) {
                lowestNeighbors[IND(i,j)].indexes[lowestNeighbors[IND(i,j)].num] = IND(i-1,j);
                ++lowestNeighbors[IND(i,j)].num;
            }
            // down
            if (i < dim_landscape-1 && elevation_map[IND(i+1,j)] == lowestElevation) {
                lowestNeighbors[IND(i,j)].indexes[lowestNeighbors[IND(i,j)].num] = IND(i+1,j);
                ++lowestNeighbors[IND(i,j)].num;
            }
        }
    }
    return lowestNeighbors;
}

int simulateRainFall(float *waterAboveGround, float *waterAbsorbed, const int *elevation_map,
                     const size_t rain_time, const float absorption_rate, const size_t dim_landscape) {
    size_t num_steps = 0; // number of steps taken for all the water to be absorbed
    bool allAbsorbed = false;
    float *waterAboveGroundCopy = new float[dim_landscape*dim_landscape];
    // calculate the loest neighbor for each cell
    const LowestNeighbors *lowestNeighbors = initLowestNeighbors(elevation_map, dim_landscape);
    while (!allAbsorbed) {
        allAbsorbed = true;
        // traverse the landscape
        for (size_t ind=0; ind<dim_landscape*dim_landscape; ++ind) {
                if (num_steps < rain_time) {
                    waterAboveGround[ind] += 1.0F;
                }
                // absorb water
                waterAbsorbed[ind] = std::min(waterAboveGround[ind], absorption_rate);
                // update water above ground
                waterAboveGround[ind] = std::max(0.0F, waterAboveGround[ind] - absorption_rate);
                waterAboveGroundCopy[ind] = waterAboveGround[ind];
                if (waterAboveGround[ind] > 0.0F) {
                    allAbsorbed = false;
                }
        }
        if (allAbsorbed) {
            break;
        }

        flow_to_neighbor(waterAboveGround, elevation_map, waterAboveGroundCopy, lowestNeighbors, dim_landscape);
        ++num_steps;
    }
    delete[] lowestNeighbors;
    delete[] waterAboveGroundCopy;
    return num_steps;
}