#!/usr/bin/env python3
import argparse
import os
import random
import subprocess
from pathlib import Path
from typing import Tuple, List

def clamp_range(lo: int, hi: int, _name: str) -> Tuple[int, int]:
    if lo > hi:
        lo, hi = hi, lo
    if lo < 0:
        lo = 0
    return lo, hi

def find_binaries(bin_dir: Path) -> List[Path]:
    """Return list of existing, executable cache binaries in priority order."""
    candidates = ["cache_lru", "cache_lfu", "cache_ideal"]
    found = []
    for c in candidates:
        p = bin_dir / c
        if p.exists() and os.access(p, os.X_OK):
            found.append(p)
    # Also allow any extra executables starting with 'cache_'.
    for p in bin_dir.iterdir():
        if p.is_file() and p.name.startswith("cache_") and os.access(p, os.X_OK) and p not in found:
            found.append(p)
    return found

def generate_tests(outdir: Path, tests: int, min_cache: int, max_cache: int,
                   min_n: int, max_n: int, min_key: int, max_key: int, seed: int = None) -> List[Path]:
    if seed is not None:
        random.seed(seed)

    min_cache, max_cache = clamp_range(min_cache, max_cache, "cache")
    min_n, max_n         = clamp_range(min_n, max_n, "n")
    min_key, max_key     = clamp_range(min_key, max_key, "key")

    outdir.mkdir(parents=True, exist_ok=True)
    paths = []

    for i in range(1, tests + 1):
        cache_size = random.randint(min_cache, max_cache)
        n          = random.randint(min_n, max_n)
        key_range  = max(1, random.randint(min_key, max_key))  # at least 1
        data       = [random.randrange(key_range) for _ in range(n)]

        path = outdir / f"test_{i:06d}.txt"
        with path.open("w", encoding="ascii") as f:
            # "<size cache> <size data> <data with amount>"
            f.write(f"{cache_size} {n} " + " ".join(map(str, data)) + "\n")
        paths.append(path)

    print(f"Generated {tests} test files in: {outdir.resolve()}")
    return paths

def run_binaries_on_tests(binaries: List[Path], tests: List[Path], key_dir: Path, append: bool):
    key_dir.mkdir(parents=True, exist_ok=True)

    # Map output file handles per binary
    handles = {}
    try:
        for bin_path in binaries:
            # output file name like lru.txt for cache_lru, etc.
            suffix = bin_path.name.replace("cache_", "")
            out_file = key_dir / f"{suffix}.txt"
            mode = "a" if append else "w"
            handles[bin_path] = out_file.open(mode, encoding="utf-8")

        for idx, tfile in enumerate(tests, start=1):
            for bin_path, hf in handles.items():
                # Run: ./cache_xxx < test_i >> suffix.txt
                with tfile.open("rb") as fin:
                    try:
                        subprocess.run([str(bin_path)], stdin=fin, stdout=hf, stderr=subprocess.PIPE, check=True)
                    except subprocess.CalledProcessError as e:
                        # Write a marker so line counts still match tests; feel free to adjust.
                        hf.write("ERROR\n")
                        print(f"[WARN] {bin_path.name} failed on {tfile.name}: {e.stderr.decode(errors='ignore').strip()}")

            if idx % 25 == 0 or idx == len(tests):
                print(f"Processed {idx}/{len(tests)} tests...")

    finally:
        for hf in handles.values():
            hf.close()

    print(f"Keys written to: {key_dir.resolve()}")
    for bin_path in binaries:
        suffix = bin_path.name.replace("cache_", "")
        print(f"  - {suffix}.txt")

def main():
    p = argparse.ArgumentParser(
        description="Generate cache test files and run cache binaries on them."
    )
    # generation options
    p.add_argument("--outdir", default=".", help="Directory to write test_*.txt files")
    p.add_argument("--tests", type=int, default=1000, help="Number of test files to generate")
    p.add_argument("--min-cache", type=int, default=10, help="Minimum cache size")
    p.add_argument("--max-cache", type=int, default=100, help="Maximum cache size")
    p.add_argument("--min-n", type=int, default=1000, help="Minimum data size (sequence length)")
    p.add_argument("--max-n", type=int, default=100000, help="Maximum data size")
    p.add_argument("--min-key", type=int, default=1, help="Minimum key-space size")
    p.add_argument("--max-key", type=int, default=30, help="Maximum key-space size")
    p.add_argument("--seed", type=int, default=None, help="Random seed for reproducibility")

    # new: where binaries live, where to write key files
    p.add_argument("--bin-dir", default="bin", help="Directory containing cache test binaries (e.g., cache_lru, cache_lfu, cache_ideal)")
    p.add_argument("--key-dir", default="keys", help="Directory to write per-binary key files (e.g., lru.txt, lfu.txt, ideal.txt)")
    p.add_argument("--append", action="store_true", help="Append to existing key files instead of overwriting")

    args = p.parse_args()

    outdir  = Path(args.outdir)
    bin_dir = Path(args.bin_dir)
    key_dir = Path(args.key_dir)

    tests = generate_tests(outdir, args.tests, args.min_cache, args.max_cache,
                           args.min_n, args.max_n, args.min_key, args.max_key, seed=args.seed)

    bins = find_binaries(bin_dir)
    if not bins:
        print(f"[ERROR] No executables named 'cache_*' found in {bin_dir.resolve()}")
        raise SystemExit(1)

    print("Found binaries:")
    for b in bins:
        print(f"  - {b}")

    run_binaries_on_tests(bins, tests, key_dir, append=args.append)

if __name__ == "__main__":
    main()
