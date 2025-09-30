#!/usr/bin/env python3
import argparse
import os
import random

def clamp_range(lo, hi, name):
    if lo > hi:
        lo, hi = hi, lo
    if lo < 0:
        lo = 0
    return lo, hi

def main():
    p = argparse.ArgumentParser(description="Generate cache test files: test_{i}.txt")
    p.add_argument("--outdir", default=".", help="Output directory")
    p.add_argument("--tests", type=int, default=10, help="Number of test files to generate")
    p.add_argument("--min-cache", type=int, default=1, help="Minimum cache size")
    p.add_argument("--max-cache", type=int, default=64, help="Maximum cache size")
    p.add_argument("--min-n", type=int, default=50, help="Minimum data size (sequence length)")
    p.add_argument("--max-n", type=int, default=500, help="Maximum data size")
    p.add_argument("--min-key", type=int, default=4, help="Minimum key-space size")
    p.add_argument("--max-key", type=int, default=200, help="Maximum key-space size")
    args = p.parse_args()

    # sanitize ranges
    min_cache, max_cache = clamp_range(args.min_cache, args.max_cache, "cache")
    min_n, max_n         = clamp_range(args.min_n, args.max_n, "n")
    min_key, max_key     = clamp_range(args.min_key, args.max_key, "key")

    os.makedirs(args.outdir, exist_ok=True)

    for i in range(1, args.tests + 1):
        cache_size = random.randint(min_cache, max_cache)
        n          = random.randint(min_n, max_n)
        key_range  = max(1, random.randint(min_key, max_key))  # at least 1

        # generate random sequence of length n with values in [0, key_range-1]
        data = [random.randrange(key_range) for _ in range(n)]

        path = os.path.join(args.outdir, f"test_{i:06d}.txt")
        with open(path, "w", encoding="ascii") as f:
            # "<size cache> <size data> <data with amount>"
            f.write(f"{cache_size} {n} " + " ".join(map(str, data)) + "\n")

    print(f"Generated {args.tests} test files in: {os.path.abspath(args.outdir)}")

if __name__ == "__main__":
    main()
