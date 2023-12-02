#include "rainfall_simulation.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <barrier>
#include <functional>

struct Neighbors {
    size_t indexes[4]; // neighbors from which water can flow to the current cell
    size_t num; // number of neighbors
    size_t numOut; // number of neighbors that the current cell can flow water to
    float waterToEachNeighbor; // amount of water that can flow to each neighbor
};

constexpr size_t ceilDiv(const size_t &a, const size_t &b) {
    return (a + b - 1) / b;
}

void simulate(const size_t dim_landscape, 
const size_t rain_time, float *waterAboveGround, float *waterAbsorbed, 
const float &absorption_rate,  Neighbors *neighbors, 
const size_t itPerBlock, const size_t id);
 
inline Neighbors *initNeighbors(const int *elevation_map, const size_t dim_landscape) {
    Neighbors *neighbors = new Neighbors[dim_landscape*dim_landscape]();
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
            // if lowest elevation is the current cell, then it has no neighbor
            if (elevation_map[IND(i,j)] <= lowestElevation) {
                continue;
            }
            // left
            if (j > 0 && elevation_map[IND(i,j-1)] == lowestElevation) {
                neighbors[IND(i,j-1)].indexes[neighbors[IND(i,j-1)].num] = IND(i,j);
                ++neighbors[IND(i,j-1)].num;
                ++neighbors[IND(i,j)].numOut;   
            }
            // right
            if (j < dim_landscape-1 && elevation_map[IND(i,j+1)] == lowestElevation) {
                neighbors[IND(i,j+1)].indexes[neighbors[IND(i,j+1)].num] = IND(i,j);
                ++neighbors[IND(i,j+1)].num;
                ++neighbors[IND(i,j)].numOut;
            }
            // up
            if (i > 0 && elevation_map[IND(i-1,j)] == lowestElevation) {
                neighbors[IND(i-1,j)].indexes[neighbors[IND(i-1,j)].num] = IND(i,j);
                ++neighbors[IND(i-1,j)].num;
                ++neighbors[IND(i,j)].numOut;
            }
            // down
            if (i < dim_landscape-1 && elevation_map[IND(i+1,j)] == lowestElevation) {
                neighbors[IND(i+1,j)].indexes[neighbors[IND(i+1,j)].num] = IND(i,j);
                ++neighbors[IND(i+1,j)].num;
                ++neighbors[IND(i,j)].numOut;
            }
        }
    }
    return neighbors;
}


bool allAbsorbed = false;
size_t num_steps = 0; // number of steps taken for all the water to be absorbed

void setAllAbsorbed() {
    allAbsorbed = true;
}

void incrNumSteps() {
    ++num_steps;
}

std::barrier<void (*)()> *iterationBarrier;
std::barrier<void (*)()> *waterFlowBarrier;


int simulateRainFall(float *waterAboveGround, float *waterAbsorbed, const int *elevation_map,
                     const size_t rain_time, const float absorption_rate, const size_t dim_landscape,
                     const size_t NUM_THREADS) {
    // calculate the loest neighbor for each cell
    Neighbors *neighbors = initNeighbors(elevation_map, dim_landscape);
    std::thread threads[NUM_THREADS]; 
    iterationBarrier = new std::barrier(NUM_THREADS, 
    setAllAbsorbed); 
    waterFlowBarrier = new std::barrier(NUM_THREADS,
    incrNumSteps);
    size_t itPerBlock = ceilDiv(dim_landscape*dim_landscape, NUM_THREADS); 
    for (size_t id = 0; id < NUM_THREADS; ++id) {
        threads[id] = std::thread(simulate, dim_landscape, 
         rain_time, waterAboveGround, waterAbsorbed, 
        absorption_rate,  neighbors, itPerBlock, id);
    }
    for (auto &th : threads) {
        th.join();
    }
    delete iterationBarrier;
    delete waterFlowBarrier;
    delete[] neighbors;
    return num_steps;
}

void simulate(const size_t dim_landscape, 
const size_t rain_time, float *waterAboveGround, float *waterAbsorbed, 
const float &absorption_rate,  Neighbors *neighbors, 
const size_t itPerBlock, const size_t id)
{
    size_t start = id*itPerBlock;
    size_t end = std::min(id*itPerBlock +itPerBlock, dim_landscape*dim_landscape);
    while (!allAbsorbed)
    {
        iterationBarrier->arrive_and_wait();
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
            if (waterAboveGround[ind] > 0.0F)
            {
                allAbsorbed = false;
            }
            if (neighbors[ind].numOut == 0)
            {
                continue;
            }
            // calculate the amount of water that can flow to the each of the lowest neighbor
            float waterLoss = (waterAboveGround[ind] > 1.0F) ? 1.0F : waterAboveGround[ind];
            waterAboveGround[ind] -= waterLoss;
            float waterToEachNeighbor = waterLoss / neighbors[ind].numOut;           
            neighbors[ind].waterToEachNeighbor = waterToEachNeighbor;
        }
        waterFlowBarrier->arrive_and_wait();
        for (size_t ind = start; ind < end; ++ind)
        {
            // update the water above ground by adding the water that flows from the neighbors
            for (size_t i = 0; i < neighbors[ind].num; ++i)
            {
                size_t neighborIndex = neighbors[ind].indexes[i];
                waterAboveGround[ind] += neighbors[neighborIndex].waterToEachNeighbor;
            }
        }
    }
}