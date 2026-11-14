#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
#include <limits>
#include <omp.h>
#include <parallel/algorithm>

// compute the average of two integers without overflow
inline int64_t average(int64_t a, int64_t b) {
    return (a & b) + ((a ^ b) >> 1);
}

// partitioning function for quicksort
inline int64_t
partition(int64_t *arr, int64_t left, int64_t right, int64_t pivot, int64_t &smallest, int64_t &biggest) {
    int64_t *left_ptr = &arr[left];
    int64_t *right_ptr = &arr[right];
    while (left_ptr < right_ptr) {
        smallest = (*left_ptr < smallest) ? *left_ptr : smallest;
        biggest = (*left_ptr > biggest) ? *left_ptr : biggest;
        if (*left_ptr > pivot) {
            --right_ptr;
            std::swap(*left_ptr, *right_ptr);
        } else {
            ++left_ptr;
        }
    }
    return left_ptr - arr;
}

inline void insertion_sort(int64_t *arr, int64_t left, int64_t right) {
    for (int64_t i = left + 1; i <= right; i++) {
        int64_t key = arr[i];
        int64_t j = i - 1;
        while (j >= left && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

// the core recursive quicksort function
void qs_core(int64_t *arr, int64_t left, int64_t right, const int64_t pivot) {
    if (right - left < 32) {
        insertion_sort(arr, left, right);
        return;
    }

    int64_t smallest = std::numeric_limits<int64_t>::max();
    int64_t biggest = std::numeric_limits<int64_t>::min();
    int64_t bound = partition(arr, left, right + 1, pivot, smallest, biggest);

    if (smallest == biggest)
        return;

#pragma omp task final(bound - left < 10000)
    qs_core(arr, left, bound - 1, average(smallest, pivot));
    qs_core(arr, bound, right, average(pivot, biggest));
}

// wrapper for the quicksort function
void min_max_quicksort(int64_t *arr, int64_t n, int num_threads = omp_get_max_threads()) {
#pragma omp parallel num_threads(num_threads)
#pragma omp single nowait
    qs_core(arr, 0, n - 1, n > 0 ? arr[average(0, n - 1)] : 0);
}

// class for generating random numbers
class Xoroshiro128Plus {
    uint64_t state[2]{};

    static inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

public:
    explicit Xoroshiro128Plus(uint64_t seed = 0) {
        state[0] = (12345678901234567 + seed) | 0b1001000010000001000100101000000110010010100000011001001010000001ULL;
        state[1] = (98765432109876543 + seed) | 0b0100000011001100100000011001001010000000100100101000000110010010ULL;
        for(int i = 0; i < 10; i++){operator()();}
    }

    uint64_t operator()() {
        const uint64_t s0 = state[0];
        uint64_t s1 = state[1];
        const uint64_t result = s0 + s1;

        s1 ^= s0;
        state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
        state[1] = rotl(s1, 37);
        return result;
    }
};

bool verify_qs_correctness(int64_t size) {
    Xoroshiro128Plus generator(size);
    std::vector<int64_t> data(size);
    for (int64_t i = 0; i < size; ++i) {
        data[i] = generator();
    }
    std::vector<int64_t> data_copy = data;  // Duplicate for std::sort
    min_max_quicksort(&data[0], size);
    std::sort(data_copy.begin(), data_copy.end());
    return data == data_copy;  // check if arrays are equal
}

// median of the measured times, robust against single slow runs (OS noise, frequency changes)
double median(std::vector<double> &t) {
    std::nth_element(t.begin(), t.begin() + t.size() / 2, t.end());
    return t[t.size() / 2];
}

// time one algorithm: every repetition sorts different random data (same data for all algorithms, since the seed
// depends only on the repetition). Re-sorting identical input lets the branch predictor memorize small inputs.
// returns the median runtime in seconds. The first run is checked against std::sort; filling the array is not timed.
template<class Sort>
double bench(int64_t n, int reps, Sort sort) {
    std::vector<int64_t> work(n);
    std::vector<double> times;
    for (int r = 0; r < reps; ++r) {
        Xoroshiro128Plus generator(r + 1);
        for (auto &x: work) x = (int64_t) generator();
        std::vector<int64_t> expected;
        if (r == 0) {
            expected = work;
            std::sort(expected.begin(), expected.end());
        }
        double start = omp_get_wtime();
        sort(work);
        times.push_back(omp_get_wtime() - start);
        if (r == 0 && work != expected) {
            std::cerr << "sorting result is wrong!\n";
            std::exit(1);
        }
    }
    return median(times);
}

// prints one CSV line: threads,n,t_std,t_min_max_quicksort,t_gnu_parallel (seconds, median)
void run(int64_t n, int threads) {
    const int reps = n >= 100000000 ? 5 : (int) std::clamp<int64_t>(30000000 / n, 5, 101);
    omp_set_num_threads(threads); // used by min_max_quicksort and __gnu_parallel::sort

    static int64_t cached_n = -1; // std::sort does not depend on the thread count, measure once per n
    static double cached_t = 0;
    if (cached_n != n) {
        cached_t = bench(n, reps, [](auto &v) { std::sort(v.begin(), v.end()); });
        cached_n = n;
    }
    double t_mmq = bench(n, reps, [](auto &v) { min_max_quicksort(v.data(), v.size()); });
    double t_gnu = bench(n, reps, [](auto &v) { __gnu_parallel::sort(v.begin(), v.end()); });
    std::cout << threads << "," << n << "," << cached_t << "," << t_mmq << "," << t_gnu << std::endl;
}

// usage: ./a.out threads <n>   -> sweep over thread counts for fixed n
//        ./a.out sizes         -> sweep over array sizes with all threads
int main(int argc, char *argv[]) {
    // test correctness of min_max_quicksort
    const std::vector<int64_t> sizes = {0, 1, 23, 133, 1777, 57462, 786453};
    for (const auto &size: sizes) {
        if (!verify_qs_correctness(size)) {
            std::cout << "min_max_quicksort is incorrect for size " << size << "!\n";
            return 1;
        }
    }

    std::cout << "threads,n,std_sort,min_max_quicksort,gnu_parallel_sort" << std::endl;
    const std::string mode = argc > 1 ? argv[1] : "";
    if (mode == "threads" && argc > 2) {
        const int64_t n = std::atoll(argv[2]);
        for (int t: {1, 2, 4, 6, 8, 12, 16, 20, 24}) run(n, t);
    } else if (mode == "sizes") {
        const int threads = omp_get_num_procs(); // all logical cores
        for (int64_t n: {1000, 3000, 10000, 30000, 100000, 300000, 1000000, 3000000, 10000000, 30000000, 100000000})
            run(n, threads);
    } else {
        std::cerr << "usage: " << argv[0] << " threads <n> | sizes\n";
        return 1;
    }
}
