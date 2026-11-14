# Lecture 03 – Reductions, Barriers and More

Chapter 3 of *Introduction to Parallel Computing* (Trobec et al., 2018): Programming Multi-core and Shared Memory Multiprocessors Using OpenMP.

## Task 1: Listing 3.1 vs. Listing 3.2

Listing 3.1 only prints the thread number. All threads start at almost the same time and each one needs just a few microseconds for the work. Which thread reaches `printf` first depends on how fast the operating system starts the threads, on the scheduler and on who gets stdout first. The program does not control any of this, so the order is different in every run.

In Listing 3.2 thread `t` first computes `fib(45 + t)` with the naive recursive algorithm and prints afterwards. The runtime of this algorithm grows by a factor of about 1.62 with every step of `n`, so thread `t` needs about 1.62^t times as long as thread 0. The differences are several seconds, which is far more than the random jitter in thread start and scheduling. This is why the threads print in the order of their numbers.

This only works with one thread per logical core. Then all threads run at the same time from the start and none has to wait for a free core.

## Task 2: Listing 3.10 (Game of Life)

The code uses the cores well when `size` is large and badly when `size` is small.

For a large `size`, the cells of the next generation do not depend on each other. `plane` is only read and every cell of `aux_plane` is written by exactly one iteration, so there is no race condition. Every cell costs the same (eight neighbors), so the static split of the iterations gives every thread the same amount of work. Also, `size * size` iterations are enough work for any number of threads, and the work per generation is much larger than the cost of starting the parallel region.

For a small `size`, e.g. the 10 × 10 plane from Fig. 3.8, one generation is only 100 cells. This takes less time than creating or waking up the threads and the barrier at the end of the loop, so the parallel version can be slower than the serial one. If only the outer loop is parallelized, a `size` smaller than the number of threads also leaves cores idle. `collapse(2)` helps with that, because it makes `size * size` iterations available, but it cannot remove the overhead.

Also, the parallel region is started and ended once per generation (`while (gens-- > 0)`). One region around the `while` loop with `omp for` inside would save this overhead, but then one thread would have to swap the pointers (`omp single`) between two barriers. And the program does very little arithmetic per cell and reads three rows of memory, so for a very large `size` it is probably limited by memory bandwidth, and the speedup flattens before all cores are used (not measured).

## Task 3: Pi with Monte Carlo, random number generation from Example 3.5

Code: [`task3/pi_monte_carlo_lcg.cpp`](task3/pi_monte_carlo_lcg.cpp) (base: [`../lecture_01/task4/pi_monte_carlo_parallel.cpp`](../lecture_01/task4/pi_monte_carlo_parallel.cpp))

Changes (only the random number generation, the parallelization stays the same):
- `default_random_engine` + `uniform_real_distribution` are replaced by the function `rnd` from Listing 3.17: a linear congruential generator `seed = (1140671485 * seed + 12820163) % 2^24` that returns `seed / 2^24`.
- The state is one `unsigned int` per thread (`seed_local = thread_id`), declared inside the parallel region. It is private to the thread, so there is no race condition. It is not an element of a shared `seeds[]` array as in Listing 3.17, because neighboring array elements would lie on the same cache line and the threads would slow each other down (false sharing). Listing 3.18/3.19 avoid this the same way.
- `seed_seq` is no longer needed.

Expected impact: the generator is only a multiplication, an addition and a bit mask per number, without an engine object and a distribution, so it should be faster. The price should be the quality: the state has only 24 bits, so the period is at most 16.7 million numbers, and for n = 100,000,000 the numbers repeat many times.

Results for n = 100,000,000, compiled with `g++ -O2 -fopenmp` on an AMD Ryzen 9 7900 (12 cores, 24 threads) with GCC 16.2.1:

| Threads | Runtime (`default_random_engine`) | Runtime (LCG) | π (LCG) |
|---|---|---|---|
| 1 | 1.65 s | 0.29 s | 3.141605 |
| 2 | 0.83 s | 0.145 s | 3.141605 |
| 4 | 0.42 s | 0.073 s | 3.141599 |
| 8 | 0.21 s | 0.037 s | 3.141600 |

The LCG is about 5.6× faster for every thread count (1.65 s → 0.29 s with one thread, 0.21 s → 0.037 s with 8 threads). The gain from more threads is the same as before, so only the generator got faster and the parallelization is not affected.

I expected π to be less accurate, but it is not. The LCG results are within about 1e-5 of π, while `default_random_engine` is off by up to 4e-4 (3.14201 with 4 threads). To find out why, I summed over exactly one period of the generator (2^23 pairs, one thread). This gives an error of about 5e-6, although 8 million random points would normally be off by around 6e-4. So the points of the LCG are not random, they lie very evenly on a lattice, and the hit count is almost exact. Different start seeds give the same value (seeds 0 and 2 both 3.1415868, seeds 1 and 7 both 3.1415977), so the threads do not draw many different points. This is probably not a good property for Monte Carlo in general, here it only works because the integral is simple i think.
