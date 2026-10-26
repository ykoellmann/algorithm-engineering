# Lecture 01 – Course Introduction and Getting Started With OpenMP

## Task 1: The Free Lunch Is Over

In the year 2005, Herb Sutter published an article in the Dr. Dobb's Journal in which he talked about the end of an era, in which software was getting faster for "free".
The so called "free lunch" was referencing the fact that old code was able to run faster and faster by taking advantage of newer hardware which came out year after year, without changes to the code.
For years, clock speed, execution optimization and bigger caches were able to make sequential code run faster. Sutter claimed that this was over, because clock speeds had stopped increasing around 2003. The reason for Sutter's claim were different physical limitations of hardware. 

Higher clock speeds lead to more power consumption which leads to more heat. Chips cannot be cooled beyound ~100 W with normal cooling methods.

Until 2005 smaller transistors also used less voltage, so more transistors could be packed onto the same chip and run at higher clock speeds with the same heat output. But after 2005 the voltage could not be reduced anymore because transistors do not work correctly when using less than about 1 V.

While CPUs were getting much faster, the access time of random access memory was not improving as fast, which led to a bottleneck when code was waiting for memory.

Sutter also hints at a more fundamental limit: light is not getting any faster. At 10 GHz, light only travels about 3 cm per clock cycle, and electrical signals on a chip are even slower.

Researchers at UC Berkeley (Asanović, Patterson et al., 2006) combined these limitations as Power Wall + Memory Wall + ILP Wall (Instruction Level Parallelism) = "Brick Wall". The ILP Wall means that CPUs cannot find much more parallelism inside sequential code to execute at the same time. The only possibility to overcome this is using more transistors on more cores instead of higher clock speeds.

This is why Sutter called concurrency the next big revolution in software development. Since single cores no longer get much faster, programs have to be split into multiple threads to benefit from new hardware. Tools like OpenMP, which Sutter already mentioned in his article, make this easier, which is exactly what this course is about.

**Sources:**
- H. Sutter, *The Free Lunch Is Over: A Fundamental Turn Toward Concurrency in Software*, Dr. Dobb's Journal 30(3), March 2005. http://www.gotw.ca/publications/concurrency-ddj.htm
- K. Asanović et al., *The Landscape of Parallel Computing Research: A View from Berkeley*, EECS Technical Report UCB/EECS-2006-183, University of California, Berkeley, December 18, 2006.

## Task 2: Computer Systems: A Programmer's Perspective, Chapter 1



## Task 3: Coding Warm-up (optional)

## Task 4: Parallel Pi with Monte Carlo
