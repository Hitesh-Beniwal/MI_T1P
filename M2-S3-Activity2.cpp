// Activity 2: Advanced OpenMP Usage and Performance Exploration
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <omp.h>

using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size) {
  for (int i = 0; i < size; i++) {
    vector[i] = rand() % 100;
  }
}

int main() {
  unsigned long size = 100000000;
  srand(time(0));
  int *v1 = (int *)malloc(size * sizeof(int));
  int *v2 = (int *)malloc(size * sizeof(int));
  int *v3 = (int *)malloc(size * sizeof(int));
  long long total = 0;

  randomVector(v1, size);
  randomVector(v2, size);

  // OpenMP with default(none) and explicit sharing
  auto start = high_resolution_clock::now();

  #pragma omp parallel for default(none) shared(v1, v2, v3, size)
  for (int i = 0; i < size; i++) {
    v3[i] = v1[i] + v2[i];
  }

  auto stop = high_resolution_clock::now();
  cout << "OMP with default(none) Time: " << duration_cast<microseconds>(stop - start).count() << " μs" << endl;

  // Atomic update version
  total = 0;
  start = high_resolution_clock::now();

  #pragma omp parallel for default(none) shared(v3, size, total)
  for (int i = 0; i < size; i++) {
    #pragma omp atomic
    total += v3[i];
  }

  stop = high_resolution_clock::now();
  cout << "Total (atomic): " << total << " | Time: " << duration_cast<microseconds>(stop - start).count() << " μs" << endl;

  // Reduction version
  total = 0;
  start = high_resolution_clock::now();

  #pragma omp parallel for default(none) shared(v3, size) reduction(+:total)
  for (int i = 0; i < size; i++) {
    total += v3[i];
  }

  stop = high_resolution_clock::now();
  cout << "Total (reduction): " << total << " | Time: " << duration_cast<microseconds>(stop - start).count() << " μs" << endl;

  // Critical section version
  total = 0;
  start = high_resolution_clock::now();

  #pragma omp parallel default(none) shared(v3, size, total)
  {
    long long local_sum = 0;

    #pragma omp for
    for (int i = 0; i < size; i++) {
      local_sum += v3[i];
    }

    #pragma omp critical
    {
      total += local_sum;
    }
  }

  stop = high_resolution_clock::now();
  cout << "Total (critical): " << total << " | Time: " << duration_cast<microseconds>(stop - start).count() << " μs" << endl;

  // Scheduling techniques
  start = high_resolution_clock::now();
  total = 0;

  #pragma omp parallel for default(none) shared(v3, size) reduction(+:total) schedule(static, 1000)
  for (int i = 0; i < size; i++) {
    total += v3[i];
  }

  stop = high_resolution_clock::now();
  cout << "Total (static schedule, chunk=1000): " << total << " | Time: " << duration_cast<microseconds>(stop - start).count() << " μs" << endl;

  // Try dynamic schedule as well
  start = high_resolution_clock::now();
  total = 0;

  #pragma omp parallel for default(none) shared(v3, size) reduction(+:total) schedule(dynamic, 1000)
  for (int i = 0; i < size; i++) {
    total += v3[i];
  }

  stop = high_resolution_clock::now();
  cout << "Total (dynamic schedule, chunk=1000): " << total << " | Time: " << duration_cast<microseconds>(stop - start).count() << " μs" << endl;

  free(v1);
  free(v2);
  free(v3);
  return 0;
}
