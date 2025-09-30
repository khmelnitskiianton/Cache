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
  IdealCache::Cache<Page, size_t> ccache{cache_size};
  std::list<Page> queue;
  try {
    for (size_t i = 0; i < data_amount; i++) {
      Page curr_page;
      IOWrap::GetFromInput<size_t>(&curr_page.id, std::cin);
      ccache.AddFuture(curr_page.id);
      queue.emplace_back(curr_page);
    }
  } catch (const std::ios_base::failure &e) {
    std::cerr << "Bad input in data: " << e.what() << std::endl;
    return 0;
  }
  size_t hits = 0;
  for (std::list<Page>::iterator queue_it = queue.begin(); queue_it != queue.end(); ++queue_it) {
    // ccache.Dump();
    if (ccache.LookUpUpdate<Page (*)(size_t)>(queue_it->id, &Page::slow_get_page)) {
      hits++;
    }
  }
  std::cout << hits << std::endl;
  return 0;
}