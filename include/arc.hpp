#ifndef INCLUDE_ARC_HPP
#define INCLUDE_ARC_HPP

#include <iostream>
#include <list>
#include <unordered_map>

namespace ARCCache {

template <typename T, typename KeyT> struct Cache {
    using ListIt = typename std::list<std::pair<KeyT, T>>::iterator;

    size_t size_;
    std::list<std::pair<KeyT, T>> cache_;
    std::unordered_map<KeyT, ListIt> hash_map_;

    Cache(size_t size) : size_(size) {}

    bool Full() const { return cache_.size() == size_; }

    template <typename F> bool LookUpUpdate(KeyT key, F slow_get_page) {
      auto hit = hash_map_.find(key);
      // case no page in cache
      if (hit == hash_map_.end()) {
        if (Full()) {
          hash_map_.erase(cache_.back().first);
          cache_.pop_back();
        }
        cache_.emplace_front(key, slow_get_page(key));
        hash_map_.emplace(key, cache_.begin());
        return false;
      }
      // case page in cache
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

} // namespace ARCCache

#endif