#include "lfu.hpp"
#include "io_wrap.hpp"
#include "page.hpp"
#include <iostream>

int main() {
  size_t cache_size = 0, data_amount = 0;
  try {
    IOWrap::GetFromInput(cache_size, std::cin);
    IOWrap::GetFromInput(data_amount, std::cin);
  } catch (const std::ios_base::failure &e) {
    std::cerr << "Bad input in sizes: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
  LFUCache::Cache<size_t, Page> ccache{cache_size};
  size_t hits = 0;
  try {
    for (size_t i = 0; i < data_amount; i++) {
      // ccache.Dump();
      Page curr_page;
      IOWrap::GetFromInput(curr_page.id, std::cin);
      if (ccache.LookUpUpdate(curr_page.id, Page::slow_get_page)) {
        hits++;
      }
    }
    // ccache.Dump();
  } catch (const std::ios_base::failure &e) {
    std::cerr << "Bad input in data: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
  std::cout << hits << '\n';
  return 0;
}