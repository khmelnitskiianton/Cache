#ifndef INCLUDE_LRU_HPP
#define INCLUDE_LRU_HPP

#include <iostream>
#include <list>
#include <unordered_map>

namespace LRUCache {

template <typename T, typename KeyT> class Cache {
    using ListIt = typename std::list<std::pair<KeyT, T>>::iterator;

  private:
    size_t size_;
    std::list<std::pair<KeyT, T>> cache_;
    std::unordered_map<KeyT, ListIt> cache_map_;

    bool Full() const { return cache_.size() == size_; }

  public:
    Cache(size_t size) : size_(size) {}

    template <typename F> bool LookUpUpdate(KeyT key, F slow_get_page) {
      if (size_ == 0)
        return false;

      auto hit = cache_map_.find(key);
      // Case no page in cache
      if (hit == cache_map_.end()) {
        if (Full()) {
          cache_map_.erase(cache_.back().first);
          cache_.pop_back();
        }
        cache_.emplace_front(key, slow_get_page(key));
        cache_map_.emplace(key, cache_.begin());
        return false;
      }
      // Case page in cache
      auto find_it = hit->second;
      if (find_it != cache_.begin()) {
        cache_.splice(cache_.begin(), cache_, find_it);
      }
      return true;
    }

    void Dump() {
      std::cout << "########" << std::endl;
      size_t index = 0;
      for (ListIt it = cache_.begin(); it != cache_.end(); ++it, ++index) {
        std::pair<KeyT, T> curr_pair = *it;
        std::cout << "[" << index << "]["
                  << "key: " << curr_pair.first << "]" << std::endl;
      }
      std::cout << "########" << std::endl;
    }
};

} // namespace LRUCache

#endif