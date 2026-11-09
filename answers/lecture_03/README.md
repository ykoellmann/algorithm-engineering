# Lecture 03 – Reductions, Barriers and More

Chapter 3 of *Introduction to Parallel Computing* (Trobec et al., 2018): Programming Multi-core and Shared Memory Multiprocessors Using OpenMP.

## Task 1: Listing 3.1 vs. Listing 3.2

**Listing 3.1** only prints the thread number (`printf(" %d", omp_get_thread_num())`). All threads start at almost the same time and each needs only a few microseconds of work. In which order they reach `printf` depends on how fast the operating system starts each thread, on the scheduler, and on who gets the lock on stdout first. This is not controlled by the program, so the order changes from run to run.

**Listing 3.2** lets thread `t` compute `fib(45 + t)` with the naive recursive algorithm before it prints. The runtime of this algorithm grows by a factor of about φ ≈ 1.62 with every increment of `n`, so thread `t` needs about 1.62^t times as long as thread 0. The differences in runtime are many seconds, which is far more than the random jitter in thread start and scheduling. Therefore the threads finish, and print, in the order of their thread numbers.

This only holds with one thread per logical core. Then all threads really run at the same time from the start, and none has to wait for a free core.

## Task 2: Listing 3.10 (Game of Life)

The code uses the cores well when `size` is large and badly when `size` is small.

**Large `size`:**
- The cells of the next generation are independent: `plane` is only read and every cell of `aux_plane` is written by exactly one iteration, so there is no race condition.
- Every cell costs the same (eight neighbors), so the static split of the iterations gives each thread the same amount of work.
- `size * size` iterations are enough for any number of threads.
- The work per generation is large compared to the cost of starting and ending the parallel region.

**Small `size`:**
- With e.g. a 10 × 10 plane (Fig. 3.8), one generation is only 100 cells. That takes less time than creating or waking up the team of threads and the barrier at the end of the loop. The parallel version can then be slower than the serial one.
- Even if the work were worth splitting, `size` limits the parallelism. With only the outer loop parallelized, a `size` smaller than the number of threads would leave cores idle. `collapse(2)` fixes this, since it makes `size * size` iterations available, but it cannot fix the overhead problem.

**Other points:**
- The parallel region is started and ended once per generation (`while (gens-- > 0)`). One `parallel` region around the `while` loop with `omp for` inside would save this overhead, but the pointer swap would then have to be done by one thread (`omp single`) between two barriers.
- The program does little arithmetic per cell and reads three rows of memory. For a very large `size` it is probably limited by memory bandwidth instead of by the number of cores, so the speedup flattens before all cores are used (this is my reasoning, not measured).

## Task 3: Pi with Monte Carlo, random number generation from Example 3.5

Code: [`task3/pi_monte_carlo_lcg.cpp`](task3/pi_monte_carlo_lcg.cpp) (base: [`../lecture_01/task4/pi_monte_carlo_parallel.cpp`](../lecture_01/task4/pi_monte_carlo_parallel.cpp))

Changes (only the random number generation, the parallelization stays the same):
- `default_random_engine` + `uniform_real_distribution` are replaced by the function `rnd` from Listing 3.17: a linear congruential generator `seed = (1140671485 * seed + 12820163) % 2^24` that returns `seed / 2^24`.
- The state is one `unsigned int` per thread (`seed_local = thread_id`), declared inside the parallel region. It is private to the thread, so there is no race condition. It is not an element of a shared `seeds[]` array as in Listing 3.17, because neighboring array elements would lie on the same cache line and the threads would slow each other down (false sharing). Listing 3.18/3.19 avoid this the same way.
- `seed_seq` is no longer needed.

Expected impact: the generator is only a multiplication, an addition and a bit mask per number, with no engine object and no distribution. It should therefore be faster. The price is quality: the state has only 24 bits, so the period is at most 16.7 million numbers, and the seeds 0, 1, 2, … start at nearby points of the same cycle. For n = 100,000,000 this can make π less accurate, so the π column below has to be checked as well.

Results for n = 100,000,000, compiled with `g++ -O2 -fopenmp` on: _(fill in CPU)_

| Threads | Runtime (`default_random_engine`) | Runtime (LCG) | π (LCG) |
|---|---|---|---|
| 1 | | | |
| 2 | | | |
| 4 | | | |
| 8 | | | |
