#include <iostream>
#include <pthread.h>
#include <mutex>
#include <ctime>
#include <time.h>
using namespace std;

#define NUM_THREADS 4
#define MAX 4

typedef struct padded_int{
  int val;
  char padding[60];

}padded_int;

typedef struct thread_data{
  int id;
  int *data_ptr;
  int data_len;
  int result;
  padded_int pad_ptr;

}thread_data;

mutex m;
int count_try1;
int count_try2;
int count_try3;
int count_try4;

void getRandomArray(int *data_ptr, int data_len)
{
  for(int i = 0; i < data_len; i++)
  {
    data_ptr[i] = rand() % MAX;
  }
}

//Count3s_seq implements count 3s Sequentially
int count3s_seq(int *a, int len)
{
  int count_seq = 0;
  for(int i = 0; i < len; i++)
  {
    if(a[i] == 3)
    {
      count_seq++;
    }
  }
  return count_seq;
}

//Count3s_try1 implements count 3s without mutal exclusion and a global count
void * count3s_try1_thread(void * arg)
{
  thread_data *data = (thread_data*)arg;
  long id = data->id;
  int stride = data->data_len/NUM_THREADS;
  int start = id*stride;

  for(int i = start; i <start + stride; i++)
  {
    if(data->data_ptr[i] == 3)
    {
      count_try1++;
    }
  }
  return NULL;
}

int count3s_try1(int *a, int len)
{
  int i;
  pthread_t tid[NUM_THREADS];
  thread_data thread_info[NUM_THREADS];
  for(i = 0; i < NUM_THREADS; i++)
  {
    thread_info[i].id = i;
    thread_info[i].data_ptr = a;
    thread_info[i].data_len = len;
    pthread_create(&tid[i], NULL, count3s_try1_thread, (void *)&thread_info[i]);
  }

  for(i = 0; i< NUM_THREADS; i++)
  {
    pthread_join(tid[i], NULL);

  }
  return count_try1;
}

// //Count3s_try2 implements count 3s with a global count and mutexes
void * count3s_try2_thread(void * arg)
{
  thread_data *data = (thread_data*)arg;
  long id = data->id;
  int stride = data->data_len/NUM_THREADS;
  int start = id*stride;
  for(int i = start; i <start + stride; i++)
  {
    if(data->data_ptr[i] == 3)
    {
      m.lock();
      count_try2++;
      m.unlock();
    }
  }
  return NULL;
}

int count3s_try2(int *a, int len)
{
  int i;
  pthread_t tid[NUM_THREADS];
  thread_data thread_info[NUM_THREADS];
  for(i = 0; i < NUM_THREADS; i++)
  {
    thread_info[i].id = i;
    thread_info[i].data_ptr = a;
    thread_info[i].data_len = len;
    thread_info[i].result = 0;
    pthread_create(&tid[i], NULL, count3s_try2_thread, (void *)&thread_info[i]);
  }
  for(i = 0; i< NUM_THREADS; i++)
  {
    pthread_join(tid[i], NULL);
  }
  return count_try2;
}

// //Count3s_try3 implements count 3s with a local count and mutexes
void * count3s_try3_thread(void * arg)
{
  thread_data *data = (thread_data*)arg;
  long id = data->id;
  int stride = data->data_len/NUM_THREADS;
  int start = id*stride;
  for(int i = start; i <start + stride; i++)
  {
    if(data->data_ptr[i] == 3)
    {
      data->result = data->result + 1;

    }
  }
  m.lock();
  count_try3 = count_try3 + data->result;
  m.unlock();
  return NULL;
}

int count3s_try3(int* a, int len)
{
  int i;
  pthread_t tid[NUM_THREADS];
  thread_data thread_info[NUM_THREADS];
  for(i = 0; i < NUM_THREADS; i++)
  {
    thread_info[i].id = i;
    thread_info[i].data_ptr = a;
    thread_info[i].data_len = len;
    thread_info[i].result = 0;
    pthread_create(&tid[i], NULL, count3s_try3_thread, (void *)&thread_info[i]);
  }
  for(i = 0; i< NUM_THREADS; i++)
  {
    pthread_join(tid[i], NULL);
  }
  return count_try3;
}
//Count3s_try4 implements count 3s with padding
void * count3s_try4_thread(void * arg)
{
  thread_data *data = (thread_data*)arg;
  long id = data->id;
  int stride = data->data_len/NUM_THREADS;
  int start = id*stride;
  data->pad_ptr.val = 0;
  for(int i = start; i <start + stride; i++)
  {
    if(data->data_ptr[i] == 3)
    {
      data->pad_ptr.val++;
    }
  }
  m.lock();
  count_try4 += data->pad_ptr.val;
  m.unlock();
  return NULL;
}

int count3s_try4(int *a, int len)
{
  int i;
  pthread_t tid[NUM_THREADS];
  thread_data thread_info[NUM_THREADS];
  padded_int private_count_padded[NUM_THREADS];
  for(i = 0; i < NUM_THREADS; i++)
  {
    thread_info[i].id = i;
    thread_info[i].data_ptr = a;
    thread_info[i].data_len = len;
    thread_info[i].result = 0;
    thread_info[i].pad_ptr = private_count_padded[i];
    pthread_create(&tid[i], NULL, count3s_try4_thread, (void *)&thread_info[i]);
  }
  for(i = 0; i< NUM_THREADS; i++)
  {
    pthread_join(tid[i], NULL);
  }
  return count_try4;
}

int main()
{

  //Change these two lines to change the size of the array
  int test_array[32768];
  int test_array_len = 32768;

  srand(time(0));
  getRandomArray(test_array, test_array_len);

  struct timespec begin_seq, end_seq;
  struct timespec begin_try1, end_try1;
  struct timespec begin_try2, end_try2;
  struct timespec begin_try3, end_try3;
  struct timespec begin_try4, end_try4;


  //SEQUENTIAL TIME CHECK-------------------------------------------------------
  clock_gettime(CLOCK_REALTIME, &begin_seq);
  int num_threes_seq = count3s_seq(test_array, test_array_len);
  clock_gettime(CLOCK_REALTIME, &end_seq);

  long seconds_seq = end_seq.tv_sec - begin_seq.tv_sec;
  long nanos_seq = end_seq.tv_nsec - begin_seq.tv_nsec;
  double elapsed_seq = seconds_seq*1000 + nanos_seq*1e-6; //convert to ms

  cout << "Sequential: " << num_threes_seq;
  cout << " Time: "<< elapsed_seq << " ms"<<endl;

  //TRY 1 TIMING----------------------------------------------------------------
  clock_gettime(CLOCK_REALTIME, &begin_try1);
  int num_threes_try1 = count3s_try1(test_array, test_array_len);
  clock_gettime(CLOCK_REALTIME, &end_try1);

  long seconds_try1 = end_try1.tv_sec - begin_try1.tv_sec;
  long nanos_try1 = end_try1.tv_nsec - begin_try1.tv_nsec;
  double elapsed_try1 = seconds_try1*1000 + nanos_try1*1e-6; //convert to ms

  cout << "Try 1: " << num_threes_try1;
  cout << " Time: "<< elapsed_try1 << " ms"<<endl;


  //TRY 2 TIMING----------------------------------------------------------------
  clock_gettime(CLOCK_REALTIME, &begin_try2);
  int num_threes_try2 = count3s_try2(test_array, test_array_len);
  clock_gettime(CLOCK_REALTIME, &end_try2);

  long seconds_try2 = end_try2.tv_sec - begin_try2.tv_sec;
  long nanos_try2 = end_try2.tv_nsec - begin_try2.tv_nsec;
  double elapsed_try2 = seconds_try2*1000 + nanos_try2*1e-6; //convert to ms

  cout << "Try 2: " << num_threes_try2;
  cout << " Time: "<< elapsed_try2 << " ms"<<endl;

  //TRY 3 TIMING----------------------------------------------------------------
  clock_gettime(CLOCK_REALTIME, &begin_try3);
  int num_threes_try3 = count3s_try3(test_array, test_array_len);
  clock_gettime(CLOCK_REALTIME, &end_try3);

  long seconds_try3 = end_try3.tv_sec - begin_try3.tv_sec;
  long nanos_try3 = end_try3.tv_nsec - begin_try3.tv_nsec;
  double elapsed_try3 = seconds_try3*1000 + nanos_try3*1e-6; //convert to ms

  cout << "Try 3: " << num_threes_try3;
  cout << " Time: "<< elapsed_try3 << " ms"<<endl;

  //TRY 4 TIMING----------------------------------------------------------------
  clock_gettime(CLOCK_REALTIME, &begin_try4);
  int num_threes_try4 = count3s_try4(test_array, test_array_len);
  clock_gettime(CLOCK_REALTIME, &end_try4);

  long seconds_try4 = end_try4.tv_sec - begin_try4.tv_sec;
  long nanos_try4 = end_try4.tv_nsec - begin_try4.tv_nsec;
  double elapsed_try4 = seconds_try4*1000 + nanos_try4*1e-6; //convert to ms

  cout << "Try 4: " << num_threes_try4;
  cout << " Time: "<< elapsed_try4 << " ms"<<endl;


  return 0;
}
