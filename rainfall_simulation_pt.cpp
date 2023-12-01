#include "rainfall_simulation.h"
#include <iostream>
#include <algorithm>

// Store the indexes of the cells that can flow water to the current cell
struct FromNeighbors {
    size_t indexes[4];
    size_t num;
    size_t numOut;
};

inline const FromNeighbors *initFromNeighbors(const int *elevation_map, const size_t dim_landscape) {
    FromNeighbors *fromNeighbors = new FromNeighbors[dim_landscape*dim_landscape]();
    #define IND(i,j) ((i)*dim_landscape + (j))
    for (size_t i=0 ; i<dim_landscape; ++i) {
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
            // if lowest elevation is the current cell, then it has no neighbor
            if (elevation_map[IND(i,j)] <= lowestElevation) {
                continue;
            }
            // left
            if (j > 0 && elevation_map[IND(i,j-1)] == lowestElevation) {
                fromNeighbors[IND(i, j-1)].indexes[fromNeighbors[IND(i, j-1)].num] = IND(i,j); 
                ++fromNeighbors[IND(i, j-1)].num;
                ++fromNeighbors[IND(i,j)].numOut;
            }
            // right
            if (j < dim_landscape-1 && elevation_map[IND(i,j+1)] == lowestElevation) {
                fromNeighbors[IND(i, j+1)].indexes[fromNeighbors[IND(i, j+1)].num] = IND(i,j);
                ++fromNeighbors[IND(i, j+1)].num;
                ++fromNeighbors[IND(i,j)].numOut;
            }
            // up
            if (i > 0 && elevation_map[IND(i-1,j)] == lowestElevation) {
                fromNeighbors[IND(i-1, j)].indexes[fromNeighbors[IND(i-1, j)].num] = IND(i,j);
                ++fromNeighbors[IND(i-1, j)].num;
                ++fromNeighbors[IND(i,j)].numOut;
            }
            // down
            if (i < dim_landscape-1 && elevation_map[IND(i+1,j)] == lowestElevation) {
                fromNeighbors[IND(i+1, j)].indexes[fromNeighbors[IND(i+1, j)].num] = IND(i,j);
                ++fromNeighbors[IND(i+1, j)].num;
                ++fromNeighbors[IND(i,j)].numOut;
            }
        } 
    } 
    return fromNeighbors;
}

void printFromNeighbors(const FromNeighbors *fromNeighbors, const size_t dim_landscape) {
    for (size_t ind = 0; ind < dim_landscape*dim_landscape; ++ind) {
        std::cout << "Cell " << ind << " has " << fromNeighbors[ind].num << " neighbors: ";
        for (size_t i = 0; i < fromNeighbors[ind].num; ++i) {
            std::cout << fromNeighbors[ind].indexes[i] << " ";
        }
        std::cout << std::endl;
    }
}

int simulateRainFall(float *waterAboveGround, float *waterAbsorbed, const int *elevation_map,
                     const size_t rain_time, const float absorption_rate, const size_t dim_landscape) {
    size_t num_steps = 0; // number of steps taken for all the water to be absorbed
    bool allAbsorbed = false;
    float *waterAboveGroundNext = new float[dim_landscape*dim_landscape];
    float * const waterAboveGroundNextCopy = waterAboveGroundNext;
    // calculate the loest neighbor for each cell
    const FromNeighbors *fromNeighbors = initFromNeighbors(elevation_map, dim_landscape);
    printFromNeighbors(fromNeighbors, dim_landscape);
    while (!allAbsorbed) {
        allAbsorbed = true;
        // traverse the landscape
        for (size_t ind=0; ind<dim_landscape*dim_landscape; ++ind) {
            float rainAmount = (num_steps < rain_time)? 1.0F : 0.0F; 
            float rainAbsorbed = std::min(waterAboveGround[ind], absorption_rate);
            float waterLoss;
            if (fromNeighbors[ind].numOut == 0) {
                waterLoss = 0.0F;
            } else {
                waterLoss =  (waterAboveGround[ind] > 1.0F)? 1.0F : waterAboveGround[ind];
            }
            float waterFromNeighbors = 0.0F;
            for (size_t i = 0; i < fromNeighbors[ind].num; ++i) {
                size_t neighborIndex = fromNeighbors[ind].indexes[i];
                waterFromNeighbors += waterAboveGround[neighborIndex]/fromNeighbors[neighborIndex].numOut;
            }
            waterAboveGroundNext[ind] = waterAboveGround[ind] + rainAmount - rainAbsorbed - waterLoss + waterFromNeighbors;
            if (waterAboveGroundNext[ind] > 0.0F) {
                allAbsorbed = false;
            }
        }
        std::swap(waterAboveGround, waterAboveGroundNext);
        ++num_steps;
    }
    delete[] fromNeighbors;
    delete[] waterAboveGroundNextCopy;
    return num_steps;
}