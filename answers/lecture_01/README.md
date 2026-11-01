# Lecture 01 – Course Introduction and Getting Started With OpenMP

## Task 1: The Free Lunch Is Over

In the year 2005, Herb Sutter published an article in Dr. Dobb's Journal in which he talked about the end of an era where software was getting faster for "free".
The so-called "free lunch" was referring to the fact that old code was able to run faster and faster by taking advantage of newer hardware which came out year after year, without changes to the code.
For years, clock speed, execution optimization and bigger caches were able to make sequential code run faster. Sutter claimed that this was over because clock speeds had stopped increasing around 2003. The reasons for Sutter's claim were different physical limitations of hardware. 

Higher clock speeds lead to more power consumption, which leads to more heat. Chips cannot be cooled beyond ~100 W with normal cooling methods.

Until 2005, smaller transistors also used less voltage, so more transistors could be packed onto the same chip and run at higher clock speeds with the same heat output. But after 2005, the voltage could not be reduced anymore because transistors do not work correctly when using less than about 1 V.

While CPUs were getting much faster, the access time of random access memory was not improving as fast, which led to a bottleneck when code was waiting for memory.

Sutter also hints at a more fundamental limit: light is not getting any faster. At 10 GHz, light only travels about 3 cm per clock cycle, and electrical signals on a chip are even slower.

Researchers at UC Berkeley (Asanović, Patterson et al., 2006) combined these limitations as Power Wall + Memory Wall + ILP Wall (Instruction-Level Parallelism) = "Brick Wall". The ILP Wall means that CPUs cannot find much more parallelism inside sequential code to execute at the same time. The only way to overcome this is using the additional transistors for more cores instead of higher clock speeds.

This is why Sutter called concurrency the next big revolution in software development. Since single cores no longer get much faster, programs have to be split into multiple threads to benefit from new hardware. Tools like OpenMP, which Sutter already mentioned in his article, make this easier, which is exactly what this course is about.

**Sources:**
- H. Sutter, *The Free Lunch Is Over: A Fundamental Turn Toward Concurrency in Software*, Dr. Dobb's Journal 30(3), March 2005. http://www.gotw.ca/publications/concurrency-ddj.htm
- K. Asanović et al., *The Landscape of Parallel Computing Research: A View from Berkeley*, EECS Technical Report UCB/EECS-2006-183, University of California, Berkeley, December 18, 2006.

## Task 2: Computer Systems: A Programmer's Perspective, Chapter 1
In the first chapter of Computer Systems: A Programmer’s Perspective, Bryant and O’Hallaron follow a simple hello.c program from source code to output on the screen and introduce the main parts of a computer system along the way. I chose two aspects: caches and parallelism.
### Caches and the memory hierarchy
When using programs on any device, typically most of the time is spent copying data, e.g. from disk to RAM to CPU to display. The first way to deal with this problem is buying more capable hardware. But in terms of memory, bigger storage is mostly always slower and fast storage much more expensive.
For example, registers are about 100 times faster than RAM, but when reading data from the disk, it can take up to 10,000,000 times longer than reading from RAM. The gap between CPU and memory is getting bigger and bigger, and this is where caches come into play.

Cache is a small and very fast memory that lies between CPU and RAM. The cache stores frequently used data, which aims to reduce the time spent on copying data that is used often.
L1 cache is almost as fast as registers are, whereas L2 cache is already 5 times slower than L1 cache. Nevertheless, L2 cache is still 5–10 times faster than reading from RAM.
Cache works because programs tend to access the same data multiple times or data that is close to each other. An example of this is a loop that sums up a list: the sum is used in every iteration, and the list items lie next to each other in memory, so a whole block of them is loaded into the cache at once.
The general memory hierarchy is: L0 registers, L1 cache, L2 cache, L3 cache, RAM, disk, remote storage, and each level acts as a cache for the next lower level.

When implementing algorithms, cache is important to consider. Cache-aware algorithms can be up to an order of magnitude faster than a naively implemented algorithm.

### Concurrency and parallelism
Concurrency describes the ability of a system to execute multiple tasks at the same time. Parallelism is described as using concurrency to make a system faster.
In the past, one CPU was switching between different tasks to simulate concurrency, while in reality executing them not at the same time but parts of each one after another.
Today, multi-core CPUs are used to execute multiple tasks in parallel. An example of this is an Intel Core i7 with 4 cores, each with its own L1/L2 cache, sharing the L3 cache.
A program can only get faster when it is parallelized by splitting it into multiple threads.
The CPU can run several instructions at the same time, by using pipelining and superscalar execution, which is why nowadays 2–4 instructions per clock cycle can be achieved.
Another way to make a program faster is using SIMD instructions, which allow one instruction to work on several values in one step.

**Source:**
- R. E. Bryant, D. R. O'Hallaron, *Computer Systems: A Programmer's Perspective*, 2nd edition, Chapter 1: A Tour of Computer Systems. http://csapp.cs.cmu.edu/2e/ch1-preview.pdf

## Task 4: Parallel Pi with Monte Carlo

Code: [`task4/pi_monte_carlo_parallel.cpp`](task4/pi_monte_carlo_parallel.cpp) (original: [`task4/pi_monte_carlo.cpp`](task4/pi_monte_carlo.cpp))

- `#pragma omp parallel` starts a team of threads.
- Each thread creates its own random engine. A shared engine would be a race condition, and identical seeds would make all threads generate the same points. The seed is built with `seed_seq{seed, thread_id}`, because with `default_random_engine` plain `seed + thread_id` gives identical streams for seeds 0 and 1 (found out after discussing the parallelization with Claude, an AI assistant).
- `#pragma omp for` splits the loop iterations evenly between the threads.
- `reduction(+ : counter)` gives every thread a local counter and adds them up at the end, which avoids a race condition on `counter`.

Results for n = 100,000,000, compiled with `g++ -O2 -fopenmp` on an AMD Ryzen 9 7900 (12 cores, 24 threads):

| Threads | Runtime | Speedup | π |
|---|---|---|---|
| 1 | 1.65 s | 1.0× | 3.14164 |
| 2 | 0.83 s | 2.0× | 3.14180 |
| 4 | 0.41 s | 4.0× | 3.14201 |
| 8 | 0.21 s | 7.9× | 3.14193 |
| 12 | 0.14 s | 11.7× | 3.14187 |
| 24 | 0.10 s | 16.3× | 3.14166 |

The speedup is almost linear up to 12 threads, because the threads work independently. Beyond that the gain is smaller, since the extra threads share physical cores (simultaneous multithreading, AMD's version of hyperthreading).
