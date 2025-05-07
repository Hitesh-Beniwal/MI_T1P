// Activity 1: Vector Addition using OpenMP (Basic Parallelism)
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <omp.h> // Include OpenMP header

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

  randomVector(v1, size);
  randomVector(v2, size);

  auto start = high_resolution_clock::now();

  // Parallelizing using OpenMP
  #pragma omp parallel for
  for (int i = 0; i < size; i++) {
    v3[i] = v1[i] + v2[i];
  }

  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(stop - start);
  cout << "Time taken by OpenMP Vector Addition: " << duration.count() << " microseconds" << endl;

  free(v1);
  free(v2);
  free(v3);
  return 0;
}
