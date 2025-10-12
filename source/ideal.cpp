#include "ideal.hpp"
#include "io_wrap.hpp"
#include "page.hpp"
#include <iostream>
#include <ostream>

int main() {
  size_t cache_size = 0, data_amount = 0;
  try {
    IOWrap::GetFromInput<size_t>(&cache_size, std::cin);
    IOWrap::GetFromInput<size_t>(&data_amount, std::cin);
  } catch (const std::ios_base::failure &e) {
    std::cerr << "Bad input in sizes: " << e.what() << std::endl;
    return 0;
  }
  IdealCache::Cache<size_t, Page> ccache{cache_size};
  std::vector<Page> future_queue;
  std::vector<size_t> future_keys;
  try {
    for (size_t i = 0; i < data_amount; i++) {
      Page curr_page;
      IOWrap::GetFromInput<size_t>(&curr_page.id, std::cin);
      future_queue.emplace_back(curr_page);
      future_keys.emplace_back(curr_page.id);
    }
  } catch (const std::ios_base::failure &e) {
    std::cerr << "Bad input in data: " << e.what() << std::endl;
    return 0;
  }
  ccache.SetStream(future_keys);
  size_t hits = 0;
  for (std::vector<Page>::iterator queue_it = future_queue.begin(); queue_it != future_queue.end(); ++queue_it) {
    // ccache.Dump();
    if (ccache.LookUpUpdate<Page (*)(size_t)>(queue_it->id, &Page::slow_get_page)) {
      hits++;
    }
  }
  // ccache.Dump();
  std::cout << hits << std::endl;
  return 0;
}