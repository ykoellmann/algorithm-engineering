#include <iomanip>
#include <iostream>
#include <omp.h>

using namespace std;

// LCG from Listing 3.17 (Trobec et al.): the whole state is one unsigned int that lives in a register
double rnd(unsigned int *seed) {
  *seed = (1140671485 * (*seed) + 12820163) % (1 << 24);
  return double(*seed) / (1 << 24);
}

int main() {
  int n = 100000000; // number of points to generate
  int counter = 0; // counter for points lying in the first quadrant of a unit circle
  auto start_time = omp_get_wtime(); // omp_get_wtime() is an OpenMP library routine

#pragma omp parallel // parallel region starts here
  { // idea for parallelisation: split up loop iterations between threads
    int num_threads = omp_get_num_threads(); // used as increment in for loop
    int thread_id = omp_get_thread_num();

    unsigned int seed_local = thread_id; // one seed per thread, otherwise all threads would draw the same numbers

    int counter_local = 0; // for counting hits locally
    for (int i = thread_id; i < n; i += num_threads) {
      auto x = rnd(&seed_local); // generate random number between 0.0 and 1.0
      auto y = rnd(&seed_local); // generate random number between 0.0 and 1.0
      if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
        ++counter_local;
      }
    }
#pragma omp atomic // only one thread performs the update of the counter
    counter += counter_local;
  } // parallel region ends here

  auto run_time = omp_get_wtime() - start_time;
  auto pi = 4 * (double(counter) / n);

  cout << setprecision(10) << "pi: " << pi << endl;
  cout << "run_time: " << run_time << " s" << endl;
  cout << "n: " << n << endl;
  cout << "threads: " << omp_get_max_threads() << endl;
}
