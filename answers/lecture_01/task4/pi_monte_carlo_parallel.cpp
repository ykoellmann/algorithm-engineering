#include <iostream>
#include <omp.h>
#include <random>

using namespace std;

int main() {
  unsigned int seed = 0;

  int n = 100000000; // number of points to generate
  int counter = 0; // counter for points lying in the first quadrant of a unit circle
  auto start_time = omp_get_wtime(); // omp_get_wtime() is an OpenMP library routine

  // each thread counts its own hits
#pragma omp parallel reduction(+ : counter)
  {
    // one engine per thread with a different seed, otherwise all threads would draw the same numbers
    seed_seq seq{seed, unsigned(omp_get_thread_num())};
    default_random_engine re{seq};
    uniform_real_distribution<double> zero_to_one{0.0, 1.0};

    // split the iterations evenly
#pragma omp for
    for (int i = 0; i < n; ++i) {
      auto x = zero_to_one(re); // generate random number between 0.0 and 1.0
      auto y = zero_to_one(re); // generate random number between 0.0 and 1.0
      if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
        ++counter;
      }
    }
  }

  auto run_time = omp_get_wtime() - start_time;
  auto pi = 4 * (double(counter) / n);

  cout << "pi: " << pi << endl;
  cout << "run_time: " << run_time << " s" << endl;
  cout << "n: " << n << endl;
  cout << "threads: " << omp_get_max_threads() << endl;
}
