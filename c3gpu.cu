#include <cuda.h>
#include <iostream>
#include <omp.h>
#include <ctime>
#include <time.h>

using namespace std;

const int N = 1 << 20;
#define MAX 4

__global__
void count3s_array(int *a, int* count)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(a[i] == 3)
  {
    count[i] = 1;
  }else{
    count[i] = 0;
  }
}


void getRandomArray(int *data_ptr, int data_len)
{
  for(int i = 0; i < data_len; i++)
  {
    data_ptr[i] = rand() % MAX;
  }
}


int main()
{

  srand(time(0));
  int len = 32768; //change this in order to change the size of the array
  int *count_array = (int *)malloc(len*sizeof(int));
  int *count_array_null = (int *)malloc(len*sizeof(int));
  int *test_array = (int *)malloc(len*sizeof(int));

  getRandomArray(test_array, len);

 //for checking correctness. Prints out array
 /*
  for(int i = 0; i<len; i++)
  {
    cout << test_array[i];
  }cout << endl;*/


  //create streams
  const int num_streams = 8;

  cudaStream_t streams[num_streams];
  float *data[num_streams];

  //allocate gpu memory
  int *d_test_array;
  int *d_count_array;
  int *d_count_array_null;
  cudaMalloc(&d_test_array, len*sizeof(int));
  cudaMalloc(&d_count_array, len*sizeof(int));
  cudaMalloc(&d_count_array_null, len*sizeof(int));

  //copy host to device
  cudaMemcpy(d_test_array, test_array, len*sizeof(int), cudaMemcpyHostToDevice);
  cudaMemcpy(d_count_array, count_array, len*sizeof(int), cudaMemcpyHostToDevice);
  cudaMemcpy(d_count_array_null, count_array_null, len*sizeof(int), cudaMemcpyHostToDevice);

  struct timespec begin_stream, end_stream;
  clock_gettime(CLOCK_REALTIME, &begin_stream);
  int threadsPerBlock;

  // must be power of two that is smaller than the length of the array. Ideally, you want
  // the threads per block to be as large as possible so long as they are under 1024
  if (len < 1024){
    threadsPerBlock = len;
  }
  else{
    threadsPerBlock = 1024;
  }

  //int threadsPerBlock = 8; // must be power of two that is smaller than the length of the array
  int numBlocks = len/threadsPerBlock;
  for (int i = 0; i < num_streams; ++i){

    cudaStreamCreate(&streams[i]); //Create an asynchronous stream

    cudaMalloc(&data[i], N * sizeof(float));

    // Launch one worker kernal per streams
    count3s_array<<<numBlocks, threadsPerBlock, 0, streams[i]>>>(d_test_array, d_count_array);
  }

  cudaDeviceSynchronize(); //Blocks until the device has completed all preceding requested tasks

  for (int i = 0; i < num_streams; ++i){
    cudaStreamDestroy(streams[i]); //Destroys and cleans up an asynchronous stream
  }

  //TIME USING NULL STREAM------------------------------------------------------
  struct timespec begin_null, end_null;
  clock_gettime(CLOCK_REALTIME, &begin_null);
  // No Streams
  count3s_array<<<numBlocks, threadsPerBlock>>>(d_test_array, d_count_array_null);
  cudaDeviceSynchronize();

  //copy device to host
  cudaMemcpy(count_array, d_count_array, len*sizeof(int), cudaMemcpyDeviceToHost);
  cudaMemcpy(count_array_null, d_count_array_null, len*sizeof(int), cudaMemcpyDeviceToHost);

  int totalnull = 0;

  // Sum the count array from null stream
  #pragma omp parallel for reduction(+ : total)
  for (int i = 0; i < len; ++i){
    totalnull += count_array_null[i];
  }

  clock_gettime(CLOCK_REALTIME, &end_null); // end null timing

  int totalstream = 0;

  #pragma omp parallel for reduction(+ : total)
  for (int i = 0; i < len; ++i){
    totalstream += count_array[i];
  }
  clock_gettime(CLOCK_REALTIME, &end_stream);

  // Streams calc
  long seconds_stream = end_stream.tv_sec - begin_stream.tv_sec;
  long nanos_stream = end_stream.tv_nsec - begin_stream.tv_nsec;
  double elapsed_stream = seconds_stream*1000 + nanos_stream*1e-6; //convert to ms

  // Null calc
  long seconds_null = end_null.tv_sec - begin_null.tv_sec;
  long nanos_null = end_null.tv_nsec - begin_null.tv_nsec;
  double elapsed_null = seconds_null*1000 + nanos_null*1e-6; //convert to ms

  cout << "Timing with Null: "<< elapsed_null << " ms"<<endl;
  cout << "GPU NULL Count is: "<< totalnull << endl;

  //We subtract elapsed_null in order to take account for the time that executing
  //the kernel using only the null stream takes and so that doesn't impact the overall
  //time of our kernel with streams
  cout << "Timing with Stream: "<< elapsed_stream-elapsed_null << " ms"<<endl;
  cout<<"GPU Stream Count is: "<< totalstream << endl;

  //free gpu memory
  cudaFree(d_count_array);
  cudaFree(d_test_array);
  free(test_array);
  free(count_array);


  return 0;
}
