#ifndef INCLUDE_IDEAL_HPP
#define INCLUDE_IDEAL_HPP

#include <cstdint>
#include <iostream>
#include <list>
#include <ostream>
#include <unordered_map>

namespace IdealCache {

template <typename T, typename KeyT> class Cache {
    using CacheListIt = typename std::list<std::pair<KeyT, T>>::iterator;

  private:
    std::list<KeyT> future_;

    size_t size_;
    std::list<std::pair<KeyT, T>> cache_;
    std::unordered_map<KeyT, CacheListIt> cache_map_;

    void FindFarestFuture(KeyT &farest_key) {
      KeyT best_key = cache_.front().first;
      size_t best_pos = 0;
      bool initialized = false;
      for (const auto &kv : cache_) {
        const KeyT &k = kv.first;
        size_t pos = NextUseIndex(k);
        if (pos == SIZE_MAX) {
          // Case we have key that will not appear
          farest_key = k;
          return;
        }
        if (!initialized || pos > best_pos) {
          best_pos = pos;
          best_key = k;
          initialized = true;
        }
      }
      farest_key = best_key;
    }

    bool InCache(KeyT key) const { return cache_map_.find(key) != cache_map_.end(); }

    size_t NextUseIndex(const KeyT &key) const {
      // Find when key will appear another time 
      size_t idx = 0;
      for (const auto &k : future_) {
        if (k == key)
          return idx;
        ++idx;
      }
      return SIZE_MAX;
    }

    bool Full() const { return cache_.size() == size_; }

  public:
    Cache(size_t size) : size_(size) {}

    void AddFuture(KeyT key) { future_.emplace_back(key); }

    void UpdateFuture() { future_.pop_front(); }

    template <typename F> bool LookUpUpdate(KeyT key, F slow_get_page) {
      if (size_ == 0)
        return false;
      
      auto hit = cache_map_.find(key);
      // Case no page in cache
      if (hit == cache_map_.end()) {
        if (Full()) {
          // Find rarest and remove it
          KeyT latest_in_future;
          FindFarestFuture(latest_in_future);
          auto it = cache_map_.find(latest_in_future);
          cache_.erase(it->second);
          cache_map_.erase(it);
        }
        // Emplace new at the front
        cache_.emplace_front(key, slow_get_page(key));
        cache_map_.emplace(key, cache_.begin());
        UpdateFuture();
        return false;
      }
      // case page in cache
      auto find_it = hit->second;
      if (find_it != cache_.begin()) {
        cache_.splice(cache_.begin(), cache_, find_it);
      }
      UpdateFuture();
      return true;
    }

    void Dump() const {
      std::cout << std::endl << "########" << std::endl;
      std::cout << "Cache:" << std::endl;
      size_t index = 0;
      for (CacheListIt it = cache_.begin(); it != cache_.end(); ++it, ++index) {
        std::pair<KeyT, T> curr_pair = *it;
        std::cout << "[" << index << "]["
                  << "key: " << curr_pair.first << "]" << std::endl;
      }
      std::cout << "########" << std::endl << std::endl;
    }
};

} // namespace IdealCache

#endif