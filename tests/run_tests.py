#!/usr/bin/env python3
"""
Run compiled cache binaries on test files and compare against expected keys.

Directory layout this script expects when placed in the project's top-level `tests/` folder:

tests/
  ├─ run_tests.py
  ├─ keys/
  │    ├─ lru.txt
  │    ├─ lfu.txt
  │    └─ ideal.txt
  └─ tests/
       ├─ test_000001.txt
       ├─ test_000002.txt
       └─ ...

By default it looks for binaries (executables) with these names:
  lru, lfu, ideal

Search order per binary (first match wins):
  1) CLI flags:  --lru-bin, --lfu-bin, --ideal-bin
  2) Environment: LRU_BIN, LFU_BIN, IDEAL_BIN
  3) ../build/<name>, ../<name>, and PATH

Usage:
  python3 run_tests.py
  python3 run_tests.py --timeout 3.0 --stop-on-fail
  python3 run_tests.py --lru-bin ../build/lru --lfu-bin ../cmake-build/lfu --ideal-bin ../cmake-build/ideal
"""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Tuple

# ---- Utilities ----------------------------------------------------------------

def script_dir() -> Path:
    return Path(__file__).resolve().parent

def discover_binary(name: str, cli_value: str | None, env_var: str) -> Path | None:
    """Find an executable by trying CLI, env, common build locations, then PATH."""
    candidates: List[Path] = []
    if cli_value:
        candidates.append(Path(cli_value))
    if os.getenv(env_var):
        candidates.append(Path(os.environ[env_var]))

    base = script_dir().parent  # project root (assuming this script is tests/run_tests.py)
    candidates += [
        base / "build" / name,
        base / name,
    ]

    # Also allow name from PATH
    path_exec = shutil.which(name)
    if path_exec:
        candidates.append(Path(path_exec))

    for c in candidates:
        if c and c.exists() and os.access(c, os.X_OK):
            return c
    return None

def read_key_lines(path: Path) -> List[str]:
    with path.open("r", encoding="utf-8") as f:
        # normalize to "what the program prints when correct"
        return [line.strip() for line in f.readlines()]

def run_once(exe: Path, test_file: Path, timeout: float) -> Tuple[int, str, str, float]:
    """Run exe < test_file; return (returncode, stdout, stderr, seconds)."""
    with test_file.open("rb") as fin:
        t0 = time.perf_counter()
        try:
            cp = subprocess.run(
                [str(exe)],
                stdin=fin,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                timeout=timeout,
                check=False,
            )
        except subprocess.TimeoutExpired as e:
            elapsed = time.perf_counter() - t0
            return 124, "", f"TIMEOUT after {elapsed:.2f}s", elapsed
    elapsed = time.perf_counter() - t0
    out = cp.stdout.decode("utf-8", errors="replace").strip()
    err = cp.stderr.decode("utf-8", errors="replace").strip()
    return cp.returncode, out, err, elapsed

@dataclass
class CaseResult:
    test_name: str
    expected: str
    got: str
    rc: int
    elapsed: float
    stderr: str

# ---- Main runner ---------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description="Run lru/lfu/ideal on test vectors and compare with keys.")
    ap.add_argument("--tests-dir", default=str(script_dir() / "tests"),
                    help="Directory with test_*.txt files (default: tests/ under this script).")
    ap.add_argument("--keys-dir", default=str(script_dir() / "keys"),
                    help="Directory with {lru,lfu,ideal}.txt key files (default: keys/ under this script).")
    ap.add_argument("--lru-bin", default=None, help="Path to lru executable.")
    ap.add_argument("--lfu-bin", default=None, help="Path to lfu executable.")
    ap.add_argument("--ideal-bin", default=None, help="Path to ideal executable.")
    ap.add_argument("--timeout", type=float, default=2.0, help="Per-test timeout in seconds (default: 2.0).")
    ap.add_argument("--stop-on-fail", action="store_true", help="Stop at first failure.")
    ap.add_argument("--verbose", "-v", action="store_true", help="Print every case result.")
    args = ap.parse_args()

    tests_dir = Path(args.tests_dir)
    keys_dir  = Path(args.keys_dir)

    # Locate test files
    test_files = sorted(tests_dir.glob("test_*.txt"))
    if not test_files:
        print(f"[ERROR] No test_*.txt found in {tests_dir}")
        return 2

    # Locate binaries
    bins: Dict[str, Path | None] = {
        "lru":   discover_binary("lru",   args.lru_bin,   "LRU_BIN"),
        "lfu":   discover_binary("lfu",   args.lfu_bin,   "LFU_BIN"),
        "ideal": discover_binary("ideal", args.ideal_bin, "IDEAL_BIN"),
    }

    missing = [k for k, v in bins.items() if v is None]
    if missing:
        print("[ERROR] Could not find executables for:", ", ".join(missing))
        print("Hint: pass --lru-bin/--lfu-bin/--ideal-bin or set LRU_BIN/LFU_BIN/IDEAL_BIN.")
        return 3

    # Load keys
    key_sets: Dict[str, List[str]] = {}
    for name in ("lru", "lfu", "ideal"):
        key_path = keys_dir / f"{name}.txt"
        if not key_path.exists():
            print(f"[ERROR] Missing key file: {key_path}")
            return 4
        key_sets[name] = read_key_lines(key_path)

        if len(key_sets[name]) != len(test_files):
            print(f"[ERROR] Keys/tests mismatch for {name}: {len(key_sets[name])} keys vs {len(test_files)} tests.")
            print(f"        Make sure {key_path.name} has one line per test, in the same sorted order.")
            return 5

    # Execute
    overall_failures = 0
    total_cases = 0
    per_prog_summary: Dict[str, Dict[str, int]] = {n: {"ok": 0, "fail": 0, "timeout": 0, "nonzero_rc": 0} for n in bins}

    print(f"Running {len(test_files)} tests for programs: {', '.join(bins.keys())}\n")

    for idx, tpath in enumerate(test_files):
        if args.verbose:
            print(f"==> {tpath.name} ({idx+1}/{len(test_files)})")

        for prog_name, exe in bins.items():
            assert exe is not None
            expected = key_sets[prog_name][idx]
            rc, out, err, elapsed = run_once(exe, tpath, args.timeout)
            total_cases += 1

            ok = (rc == 0) and (out == expected)
            if rc == 124:
                per_prog_summary[prog_name]["timeout"] += 1
            if rc != 0:
                per_prog_summary[prog_name]["nonzero_rc"] += 1
            if ok:
                per_prog_summary[prog_name]["ok"] += 1
                if args.verbose:
                    print(f"  [{prog_name}] OK  rc=0  {elapsed:.3f}s  out={out}")
            else:
                per_prog_summary[prog_name]["fail"] += 1
                overall_failures += 1
                print(f"  [{prog_name}] FAIL rc={rc} {elapsed:.3f}s; expected='{expected}' got='{out}'")
                if err:
                    print(f"           stderr: {err}")
                if args.stop_on_fail:
                    print("\nStopping early due to --stop-on-fail.")
                    return 1

    # Summary
    print("\nSummary:")
    for prog_name, stats in per_prog_summary.items():
        ok = stats["ok"]
        fail = stats["fail"]
        tmo = stats["timeout"]
        nz  = stats["nonzero_rc"]
        print(f"  {prog_name:>5}: ok={ok}  fail={fail}  timeouts={tmo}  nonzero_rc={nz}")

    if overall_failures == 0:
        print(f"\nALL GOOD ({total_cases} cases)")
        return 0
    else:
        print(f"\nFAILED({overall_failures} failing case{'s' if overall_failures != 1 else ''} / {total_cases})")
        return 1

if __name__ == "__main__":
    sys.exit(main())
