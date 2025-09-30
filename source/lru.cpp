#include "lru.hpp"
#include "io_wrap.hpp"
#include "page.hpp"
#include <iostream>

int main() {
  size_t cache_size = 0, data_amount = 0;
  try {
    IOWrap::GetFromInput<size_t>(&cache_size, std::cin);
    IOWrap::GetFromInput<size_t>(&data_amount, std::cin);
  } catch (const std::ios_base::failure &e) {
    std::cerr << "Bad input in sizes: " << e.what() << std::endl;
    return 0;
  }
  LRUCache::Cache<Page, size_t> ccache{cache_size};
  size_t hits = 0;
  try {
    for (size_t i = 0; i < data_amount; i++) {
      // ccache.Dump();
      Page curr_page;
      IOWrap::GetFromInput<size_t>(&curr_page.id, std::cin);
      if (ccache.LookUpUpdate<Page (*)(size_t)>(curr_page.id, &Page::slow_get_page)) {
        hits++;
      }
    }
  } catch (const std::ios_base::failure &e) {
    std::cerr << "Bad input in data: " << e.what() << std::endl;
    return 0;
  }
  std::cout << hits << std::endl;
  return 0;
}