"""Plot the CSV files written by min_max_quicksort: python3 plot.py threads.csv sizes.csv"""
import csv
import sys

import matplotlib.pyplot as plt


def read(path):
    with open(path) as f:
        return [{k: float(v) for k, v in row.items()} for row in csv.DictReader(f)]


def plot(rows, x, xlabel, title, out, log_x=False):
    xs = [r[x] for r in rows]
    for col, label in (("min_max_quicksort", "min_max_quicksort"), ("gnu_parallel_sort", "gnu_parallel::sort")):
        plt.plot(xs, [r["std_sort"] / r[col] for r in rows], marker="o", label=label)
    plt.axhline(1, color="gray", linestyle="--", label="std::sort")
    if log_x:
        plt.xscale("log")
    plt.xlabel(xlabel)
    plt.ylabel("speedup over std::sort")
    plt.title(title)
    plt.grid(alpha=0.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out, dpi=150)
    plt.clf()


threads, sizes = read(sys.argv[1]), read(sys.argv[2])
plot(threads, "threads", "number of threads", f"Speedup vs. threads (n = {int(threads[0]['n']):,})", "speedup_threads.png")
plot(sizes, "n", "array size n", f"Speedup vs. array size ({int(sizes[0]['threads'])} threads)", "speedup_sizes.png", log_x=True)
