# Cache

[![MIPT](https://img.shields.io/endpoint?style=plastic&url=https%3A%2F%2Fraw.githubusercontent.com%2Fkhmelnitskiianton%2FCache%2Fmain%2F.github%2Fbadge%2Fmipt-badge.json)](#)

[![License](https://img.shields.io/github/license/khmelnitskiianton/mega-humidifier)](#)
[![GitHub Actions](https://img.shields.io/badge/GitHub_Actions-2088FF?logo=github-actions&logoColor=white)](#)

[![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white)](#)
[![CMake](https://img.shields.io/badge/CMake-064F8C?logo=cmake&logoColor=fff)](#)
[![Markdown](https://img.shields.io/badge/Markdown-%23000000.svg?logo=markdown&logoColor=white)](#)
![PythonAnywhere](https://img.shields.io/badge/pythonanywhere-%232F9FD7.svg?style=for-the-badge&logo=pythonanywhere&logoColor=151515)


Researching and analysing the cache performance of three algorithms:
- LRU (Least recently used)
- LFU (Least frequently used)
- Ideal (Belady's optimal algorithm)

## Dependencies

Compiler c/c++(clang preferable), cmake, python, gtest

```shell
apt-get install build-essential clang ninja-build make cmake python3
apt-get install libgtest-dev libgmock-dev libtbb-dev
```

## Building

### Config

*Debug:*
```shell
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=clang++ -S . -B build
```
*Release:*
```shell
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -S . -B build
```

### Build
```
cmake --build build
```

Binaries are located in `build/bin/cache_*`

## Tests

Sources of tests are located in `tests/` and script for generating random tests and run caches on it in `tests/gen_tests.py`.

```shell
python3 ./tests/gen_tests.py --outdir data_test --tests 1000 --bin-dir build/bin --key-dir keys
```

Google Tests, for run do:

```shell
ctest --test-dir build/tests --output-on-failure
```

Python Tests, for run do:

```shell
python3 tests/run_tests.py --lru-bin build/bin/cache_lru --lfu-bin build/bin/cache_lfu --ideal-bin build/bin/cache_ideal
```

## Comparing cache's perfomance

Overall results:

![](.github/images/results.png)

Comparing LFU vs LRU:

![](.github/images/lru_vs_lfu.png)

## Workflow

Pipeline runs in 2 stages:
- `build`: debug build project & create artifact.
- `tests`: run tests for build