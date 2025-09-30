# Cache

Realisation of the cache in three algorithms: LRU, LFU and Belady's optimal algorithm.

## Comparing



## Dependencies

compiler c/c++, cmake, python, gtest

```shell
apt-get install clang make cmake python3
apt-get install libgtest-dev libgmock-dev libtbb-dev
```

## Build

```shell
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=clang++ -S . -B build
cmake --build build
```

Binaries are located in `build/bin/cache_*`

## Tests

Sources of tests are located in `tests/` and script for generating random tests in `tests/gen_tests.py` - dont forget to write  answers in `keys/*.txt`

```shell
python tests/tests.py --outdir tests --tests 5 --min-cache 2 --max-cache 8 --min-n 30 --max-n 60 --min-key 5 --max-key 20
```

Google Tests, for run do:

```shell
./build/gtests
```