#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <regex>

#include <bits/getopt_core.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "ideal.hpp"
#include "io_wrap.hpp"
#include "lru.hpp"
#include "page.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

static constexpr auto test_name_pattern_ = "^test_[0-9]+\\.txt$";

std::filesystem::path tests_dir_{TESTS_DIR};
std::filesystem::path keys_dir_{KEYS_DIR};

std::ifstream lru_keys;
std::ifstream ideal_keys;

void PrepareKeys() {
  std::filesystem::path lru_keys_path = keys_dir_ / "lru.txt";
  std::filesystem::path ideal_keys_path = keys_dir_ / "ideal.txt";
  try {
    IOWrap::TryOpenFile(lru_keys, lru_keys_path.string());
  } catch (const std::ios_base::failure &e) {
    std::cerr << "Can't read keys for test in " << lru_keys_path << " with error: " << e.what();
    std::exit(EXIT_FAILURE);
  }
  try {
    IOWrap::TryOpenFile(ideal_keys, ideal_keys_path.string());
  } catch (const std::ios_base::failure &e) {
    std::cerr << "Can't read keys for test in " << ideal_keys_path << " with error: " << e.what();
    std::exit(EXIT_FAILURE);
  }
}

std::vector<std::string> GetTestsInDir() {
  std::vector<std::string> find_tests;
  try {
    for (std::filesystem::directory_iterator iter(tests_dir_), end; iter != end; ++iter) {
      if (is_regular_file(iter->status()) &&
          std::regex_search(iter->path().filename().string(), std::regex(test_name_pattern_))) {
        find_tests.emplace_back(iter->path().string());
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Can't read data for test in " << tests_dir_ << " with error: " << e.what();
    std::exit(EXIT_FAILURE);
  }
  return find_tests;
}

class CacheTest : public ::testing::TestWithParam<std::string> {};

TEST_P(CacheTest, LRUCacheTest) {
  std::string file = GetParam();
  std::ifstream cache_in;

  size_t cache_size = 0, data_amount = 0;
  try {
    IOWrap::TryOpenFile(cache_in, file);
    IOWrap::GetFromInput<size_t>(&cache_size, cache_in);
    IOWrap::GetFromInput<size_t>(&data_amount, cache_in);
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Problem in getting data: " << e.what() << std::endl;
  }
  LRUCache::Cache<Page, size_t> ccache{cache_size};
  size_t hits = 0;
  try {
    for (size_t i = 0; i < data_amount; i++) {
      Page curr_page;
      IOWrap::GetFromInput<size_t>(&curr_page.id, cache_in);
      if (ccache.LookUpUpdate<Page (*)(size_t)>(curr_page.id, &Page::slow_get_page)) {
        hits++;
      }
    }
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Bad input in data: " << e.what() << std::endl;
  }
  // Get expected and compare
  size_t hits_expected = 0;
  try {
    IOWrap::GetFromInput(&hits_expected, lru_keys);
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Problem in key data: " << e.what() << std::endl;
  }
  ASSERT_EQ(hits, hits_expected);
}


TEST_P(CacheTest, IdealCacheTest) {
  std::string file = GetParam();
  std::ifstream cache_in;

  size_t cache_size = 0, data_amount = 0;
  try {
    IOWrap::TryOpenFile(cache_in, file);
    IOWrap::GetFromInput<size_t>(&cache_size, cache_in);
    IOWrap::GetFromInput<size_t>(&data_amount, cache_in);
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Bad input in sizes: " << e.what() << std::endl;
  }
  IdealCache::Cache<Page, size_t> ccache{cache_size};
  std::list<Page> queue;
  try {
    for (size_t i = 0; i < data_amount; i++) {
      Page curr_page;
      IOWrap::GetFromInput<size_t>(&curr_page.id, cache_in);
      ccache.AddFuture(curr_page.id);
      queue.emplace_back(curr_page);
    }
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Bad input in data: " << e.what() << std::endl;
  }
  size_t hits = 0;
  for (std::list<Page>::iterator queue_it = queue.begin(); queue_it != queue.end(); ++queue_it) {
    if (ccache.LookUpUpdate<Page (*)(size_t)>(queue_it->id, &Page::slow_get_page)) {
      hits++;
    }
  }
  // Get expected and compare
  size_t hits_expected = 0;
  try {
    IOWrap::GetFromInput(&hits_expected, ideal_keys);
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Problem in key data: " << e.what() << std::endl;
  }
  ASSERT_EQ(hits, hits_expected);
}

INSTANTIATE_TEST_SUITE_P(E2ETests, CacheTest, ::testing::ValuesIn(GetTestsInDir()));

int main(int argc, char **argv) {
  std::cout << "Tests dir: " << tests_dir_ << std::endl;
  std::cout << "Key dir: " << keys_dir_ << std::endl;
  PrepareKeys();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}