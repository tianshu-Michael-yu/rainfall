#!/bin/bash

./rainfall_seq 50 0.5 4096 ../hw5/measurement_4096x4096.in > ../seq.out
./rainfall_pt 4 50 0.5 4096 ../hw5/measurement_4096x4096.in > ../pt.out
