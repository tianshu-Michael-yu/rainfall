#include "rainfall_simulation.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <barrier>

#define NUM_THREADS 1

struct LowestNeighbors {
    size_t indexes[4];
    size_t num;
};

constexpr size_t ceilDiv(const size_t &a, const size_t &b) {
    return (a + b - 1) / b;
}

void simulate(bool &allAbsorbed, const size_t dim_landscape, size_t &num_steps, 
const size_t rain_time, float *waterAboveGround, float *waterAbsorbed, 
const float &absorption_rate, float *waterAboveGroundCopy, const LowestNeighbors *lowestNeighbors, 
const size_t itPerBlock, const size_t id, std::barrier<std::__empty_completion> *iterationBarrier);
 
inline const LowestNeighbors *initLowestNeighbors(const int *elevation_map, const size_t dim_landscape) {
    LowestNeighbors *lowestNeighbors = new LowestNeighbors[dim_landscape*dim_landscape];
    #define IND(i,j) ((i)*dim_landscape + (j))
    // calculate the lowest neighbor for each cell
    for (size_t i=0; i<dim_landscape; ++i) {
        for (size_t j=0; j<dim_landscape; ++j) {
            lowestNeighbors[IND(i,j)].num = 0;
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
    std::thread threads[NUM_THREADS]; 
    size_t itPerBlock = ceilDiv(dim_landscape*dim_landscape, NUM_THREADS); 
    std::barrier<std::__empty_completion> iterationBarrier(NUM_THREADS);
    for (size_t id = 0; id < NUM_THREADS; ++id) {
        threads[id] = std::thread(simulate, std::ref(allAbsorbed), dim_landscape, 
        std::ref(num_steps), rain_time, waterAboveGround, waterAbsorbed, 
        absorption_rate, waterAboveGroundCopy, lowestNeighbors, itPerBlock, id, &iterationBarrier);
    }
    for (auto &th : threads) {
        th.join();
    }
    delete[] lowestNeighbors;
    delete[] waterAboveGroundCopy;
    return num_steps;
}

void simulate(bool &allAbsorbed, const size_t dim_landscape, size_t &num_steps, 
const size_t rain_time, float *waterAboveGround, float *waterAbsorbed, 
const float &absorption_rate, float *waterAboveGroundCopy, const LowestNeighbors *lowestNeighbors, 
const size_t itPerBlock, const size_t id, std::barrier<std::__empty_completion> *iterationBarrier)
{
    size_t start = id*itPerBlock;
    size_t end = std::min(id*itPerBlock +itPerBlock, dim_landscape*dim_landscape);
    while (!allAbsorbed)
    {
        allAbsorbed = true;
        // traverse the landscape
        for (size_t ind = start; ind < end; ++ind)
        {
            if (num_steps < rain_time)
            {
                waterAboveGround[ind] += 1.0F;
            }
            // absorb water
            waterAbsorbed[ind] += std::min(waterAboveGround[ind], absorption_rate);
            // update water above ground
            waterAboveGround[ind] = std::max(0.0F, waterAboveGround[ind] - absorption_rate);
            waterAboveGroundCopy[ind] = waterAboveGround[ind];
            if (waterAboveGround[ind] > 0.0F)
            {
                allAbsorbed = false;
            }
        }
        iterationBarrier->arrive_and_wait();
        for (size_t ind = 0; ind < dim_landscape * dim_landscape; ++ind)
        {
            // calculate the amount of water that can flow to the each of the lowest neighbor
            if (lowestNeighbors[ind].num == 0)
            {
                continue;
            }
            float waterLoss = (waterAboveGroundCopy[ind] > 1.0F) ? 1.0F : waterAboveGroundCopy[ind];
            waterAboveGround[ind] -= waterLoss;
            float waterToEachNeighbor = waterLoss / lowestNeighbors[ind].num;
            // update the water above ground for each neighbor
            for (size_t i = 0; i < lowestNeighbors[ind].num; ++i)
            {
                size_t neighborIndex = lowestNeighbors[ind].indexes[i];
                waterAboveGround[neighborIndex] += waterToEachNeighbor;
            }
        }
        iterationBarrier->arrive_and_wait();
        ++num_steps;
    }
}