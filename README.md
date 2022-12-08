# Analysis-of-Parallelization-Methods


Lucy Anderson and Will Duquette

In order to reproduce our results, compile the cpp file using the -pthread tag in the command line and the cu file using nvcc.

CUDA File --------------------------------------------------------
To compile the CUDA version: nvcc c3gpu.cu -o c3s.out
The length of the array has to be a power of 2
To change the length of the array, change the value of len on line 38

----CPU FILE----
To compile the CPU version: g++ -pthread  c3.cpp -o c3.out
To change the length of the array, edit lines 219 and 220 so that the numbers match the desired array length. 
