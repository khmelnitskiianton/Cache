#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <regex>

#include <stdio.h>
#include <stdlib.h>

#include "ideal.hpp"
#include "io_wrap.hpp"
#include "lfu.hpp"
#include "lru.hpp"
#include "page.hpp"

#include <gtest/gtest.h>

static constexpr auto test_name_pattern_ = "^test_[0-9]+\\.txt$";
static constexpr auto test_number_pattern_ = R"(^test_(\d{6})\.txt$)";
static constexpr auto lru_keys_filename = "lru.txt";
static constexpr auto ideal_keys_filename = "ideal.txt";
static constexpr auto lfu_keys_filename = "lfu.txt";

std::filesystem::path tests_dir_{TESTS_DIR};
std::filesystem::path keys_dir_{KEYS_DIR};

size_t GetTestKey(std::string test_file, std::string keys_file_name) {
  size_t number_of_test = 1;
  std::string test_file_name = std::filesystem::path(test_file).filename();
  static const std::regex pat(test_number_pattern_);
  std::smatch m;
  if (std::regex_match(test_file_name, m, pat)) {
    number_of_test = static_cast<unsigned>(std::stoul(m[1].str()));
  }
  size_t key_hits = 0;
  std::ifstream keys_stream;
  std::filesystem::path keys_path = keys_dir_ / std::filesystem::path(keys_file_name);
  try {
    IOWrap::TryOpenFile(keys_stream, keys_path.string());
    for (size_t i = 0; i < number_of_test; i++) {
      IOWrap::GetFromInput(key_hits, keys_stream);
    }
  } catch (const std::ios_base::failure &e) {
    std::cerr << "Can't read keys for test in " << keys_path << " with error: " << e.what();
    std::exit(EXIT_FAILURE);
  }
  return key_hits;
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
    IOWrap::GetFromInput(cache_size, cache_in);
    IOWrap::GetFromInput(data_amount, cache_in);
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Problem in getting data: " << e.what() << std::endl;
  }
  LRUCache::Cache<size_t, Page> ccache{cache_size};
  size_t hits = 0;
  try {
    for (size_t i = 0; i < data_amount; i++) {
      Page curr_page;
      IOWrap::GetFromInput(curr_page.id, cache_in);
      if (ccache.LookUpUpdate(curr_page.id, Page::slow_get_page)) {
        hits++;
      }
    }
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Bad input in data: " << e.what() << std::endl;
  }
  // Get expected and compare
  size_t hits_expected = 0;
  try {
    hits_expected = GetTestKey(file, lru_keys_filename);
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Problem in key data: " << e.what() << std::endl;
  }
  ASSERT_EQ(hits, hits_expected);
}

TEST_P(CacheTest, LFUCacheTest) {
  std::string file = GetParam();
  std::ifstream cache_in;

  size_t cache_size = 0, data_amount = 0;
  try {
    IOWrap::TryOpenFile(cache_in, file);
    IOWrap::GetFromInput(cache_size, cache_in);
    IOWrap::GetFromInput(data_amount, cache_in);
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Problem in getting data: " << e.what() << std::endl;
  }
  LFUCache::Cache<size_t, Page> ccache{cache_size};
  size_t hits = 0;
  try {
    for (size_t i = 0; i < data_amount; i++) {
      Page curr_page;
      IOWrap::GetFromInput(curr_page.id, cache_in);
      if (ccache.LookUpUpdate(curr_page.id, Page::slow_get_page)) {
        hits++;
      }
    }
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Bad input in data: " << e.what() << std::endl;
  }
  // Get expected and compare
  size_t hits_expected = 0;
  try {
    hits_expected = GetTestKey(file, lfu_keys_filename);
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
    IOWrap::GetFromInput(cache_size, cache_in);
    IOWrap::GetFromInput(data_amount, cache_in);
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Bad input in sizes: " << e.what() << std::endl;
  }
  IdealCache::Cache<size_t, Page> ccache{cache_size};
  std::vector<Page> future_queue;
  std::vector<size_t> future_keys;
  try {
    for (size_t i = 0; i < data_amount; ++i) {
      Page curr_page;
      IOWrap::GetFromInput(curr_page.id, cache_in);
      future_queue.emplace_back(curr_page);
      future_keys.emplace_back(curr_page.id);
    }
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Bad input in data: " << e.what() << std::endl;
  }
  ccache.SetStream(future_keys);
  size_t hits = 0;
  for (std::vector<Page>::iterator queue_it = future_queue.begin(); queue_it != future_queue.end(); ++queue_it) {
    if (ccache.LookUpUpdate(queue_it->id, Page::slow_get_page)) {
      hits++;
    }
  }
  // Get expected and compare
  size_t hits_expected = 0;
  try {
    hits_expected = GetTestKey(file, ideal_keys_filename);
  } catch (const std::ios_base::failure &e) {
    FAIL() << "Problem in key data: " << e.what() << std::endl;
  }
  ASSERT_EQ(hits, hits_expected);
}

INSTANTIATE_TEST_SUITE_P(E2ETests, CacheTest, ::testing::ValuesIn(GetTestsInDir()));

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}