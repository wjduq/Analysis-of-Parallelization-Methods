# Analysis of Parallelization Methods

**Date:** 9/30/2022

 By: Lucy Anderson and Will Duquette
 
The following code and examples are for a paper written by William Duquette and Lucy Anderson called "Analysis of Parallelization Methods." Outside of the paper, there are two files worth noting. First, there is a C++ file where we implement a multi-threaded count 3s algorithm on the CPU (there is code to time the functions). Second, there is a CUDA file in which we use CUDA (developed by NVIDIA for use on their GPUs) to run a count 3s algorithm on the GPU. Within our CUDA file we have two approaches to the problem. First, we use the default NULL stream. The second approach utilizes different streams to improve our code when we see larger data.

## Reproduction of Results
In general to reproduce our results, compile the cpp file using the -pthread tag in the command line and the cu file using nvcc. There are more specific instructions below.

### CUDA (GPU)
- To compile the CUDA version: nvcc c3gpu.cu -o c3s.out
- The length of the array has to be a power of 2
- To change the length of the array, change the value of len on line 38

### CPP (CPU)
- To compile the CPU version: g++ -pthread  c3.cpp -o c3.out
- To change the length of the array, edit lines 219 and 220 so that the numbers match the desired array length. 
