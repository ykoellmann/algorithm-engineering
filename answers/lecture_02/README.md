# Lecture 02 – False Sharing, Race Conditions and Schedules

## Task 1: False Sharing

Figure: [`task1/false_sharing.jpg`](task1/false_sharing.jpg)

## Task 2: There's Plenty of Room at the Top

The figure shows where performance gains will come from after Moores law ends. The Top is everything above the transistors, namely software, algorithms and hardware architecture, while the Bottom is the semiconductor technology (miniaturization of transistors). 

The Top is not a replacement for Moore's law, because it works differently. Moore's law was a free lunch: smaller transistors made every program faster without changing any code. At the Top, every gain needs someone to actually change something, and one change often forces changes in the other layers.

Most software was written to be quick to develop, not quick to run, so a lot of unnecessary work is hidden in it (bloat). The more interesting part is tailoring: a program that ignores multiple cores, vector units and caches only uses a small part of what the chip can do. This is exactly what we do in this course with OpenMP, and false sharing from task 1 shows that even correct parallel code can be slow when it ignores how the cache works.

A better algorithm saves work on any hardware, but it is hard to predict when someone finds one. Many classic algorithms assume a machine that does one operation at a time and has equally fast memory. Real machines have many cores and caches, so a new machine model means designing algorithms for what hardware really looks like. New problem domains like machine learning need new algorithms in the first place.

The idea of hardware architecture is the opposite of the last decades. Instead of spending more transistors to make one core faster, we can use simpler cores and put more of them on the chip, or build hardware like GPUs for just one kind of task. This only works if the program can make use of that parallelism, which connects the hardware back to the software and the algorithms.
Because of this, the gains are uneven: only one program or problem gets faster at a time, and once code is well optimized, there is less left to gain.

## Task 4: Numerical Integration with `#pragma omp for`

Code: [`task4/pi_omp_for.cpp`](task4/pi_omp_for.cpp) (original: [`task4/pi_numerical_integration.cpp`](task4/pi_numerical_integration.cpp))

- `#pragma omp parallel` starts the team of threads, `#pragma omp for` splits the loop iterations between them. No manual splitting with `thread_id` and `num_threads` is needed anymore.
- `reduction(+ : sum)` gives every thread a local `sum` and adds them up at the end, so there is no race condition on `sum`.

Results for n = 100,000,000:

| Threads | Runtime | Speedup | π |
|---|---|---|---|
| sequential | 0.083 s | – | 3.14159265359043 |
| 1 | 0.082 s | 1.0× | 3.14159265359043 |
| 2 | 0.041 s | 2.0× | 3.14159265358991 |
| 4 | 0.021 s | 4.0× | 3.14159265358968 |
| 8 | 0.011 s | 7.8× | 3.14159265358981 |
| 12 | 0.010 s | 8.1× | 3.14159265358983 |
| 24 | 0.011 s | 7.6× | 3.14159265358978 |

The speedup is linear up to 8 threads. After that the program is so short that probably the cost of starting the threads and the reduction takes a large share of the runtime (not measured).
π differs slightly in the last digits between thread counts, because floating-point addition is not associative and the partial sums are added in a different order.
