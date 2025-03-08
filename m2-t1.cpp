//Parallel Matrix Multiplication Task Solution  by Hitesh Beniwal
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <omp.h>
#include <fstream>
#include <chrono>
#include <random>

using namespace std;
using namespace chrono;

#define N 500  // Matrix size
#define MAX_THREADS 8

// Global Matrices
vector<vector<int>> A(N, vector<int>(N));
vector<vector<int>> B(N, vector<int>(N));
vector<vector<int>> C(N, vector<int>(N, 0));

// Function to initialize matrices with random values
void initialize_matrices() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 10);  // Random values between 1 and 10

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = dist(gen);
            B[i][j] = dist(gen);
        }
    }
}

// Function to reset matrix C before each execution
void reset_C() {
    for (auto &row : C) {
        fill(row.begin(), row.end(), 0);
    }
}

// Sequential Matrix Multiplication
void sequential_multiplication() {
    reset_C();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Pthread Parallel Matrix Multiplication
struct ThreadData {
    int start_row, end_row;
};

void* pthread_multiply(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

void parallel_pthread() {
    reset_C();
    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];
    int rows_per_thread = N / MAX_THREADS;

    for (int i = 0; i < MAX_THREADS; i++) {
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == MAX_THREADS - 1) ? N : (i + 1) * rows_per_thread;
        pthread_create(&threads[i], NULL, pthread_multiply, &thread_data[i]);
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

// OpenMP Parallel Matrix Multiplication
void parallel_openmp() {
    reset_C();
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                #pragma omp atomic
                C[i][j] += A[i][k] * B[k][j];  // Avoid race condition
            }
        }
    }
}

// Function to measure execution time using chrono
void measure_time(void (*func)(), const string& method) {
    auto start = chrono::high_resolution_clock::now();
    func();
    auto end = chrono::high_resolution_clock::now();
    
    // Correct way to calculate duration in seconds
    chrono::duration<double> elapsed = end - start;

    cout << method << " Execution Time: " << elapsed.count() << " sec" << endl;
}


int main() {
    initialize_matrices();

    measure_time(sequential_multiplication, "Sequential");
    measure_time(parallel_pthread, "Pthread Parallel");
    measure_time(parallel_openmp, "OpenMP Parallel");

    return 0;
}
