#include <iostream>
#include <omp.h>
#include <random>

using namespace std;

int main() {
  unsigned int seed = 0;

  int n = 100000000; // number of points to generate
  int counter = 0; // counter for points lying in the first quadrant of a unit circle
  auto start_time = omp_get_wtime(); // omp_get_wtime() is an OpenMP library routine

#pragma omp parallel // parallel region starts here
  { // idea for parallelisation: split up loop iterations between threads
    int num_threads = omp_get_num_threads(); // used as increment in for loop
    int thread_id = omp_get_thread_num();

    // one engine per thread with a different seed, otherwise all threads would draw the same numbers;
    // seed_seq scrambles (seed, thread id), because plain seed + id gives identical (0, 1) or correlated streams
    seed_seq seq{seed, unsigned(thread_id)};
    default_random_engine re{seq};
    uniform_real_distribution<double> zero_to_one{0.0, 1.0};

    int counter_local = 0; // for counting hits locally
    for (int i = thread_id; i < n; i += num_threads) {
      auto x = zero_to_one(re); // generate random number between 0.0 and 1.0
      auto y = zero_to_one(re); // generate random number between 0.0 and 1.0
      if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
        ++counter_local;
      }
    }
#pragma omp atomic // only one thread performs the update of the counter
    counter += counter_local;
  } // parallel region ends here

  auto run_time = omp_get_wtime() - start_time;
  auto pi = 4 * (double(counter) / n);

  cout << "pi: " << pi << endl;
  cout << "run_time: " << run_time << " s" << endl;
  cout << "n: " << n << endl;
  cout << "threads: " << omp_get_max_threads() << endl;
}
