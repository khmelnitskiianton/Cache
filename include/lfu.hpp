#ifndef INCLUDE_LFU_HPP
#define INCLUDE_LFU_HPP

#include <algorithm>
#include <iostream>
#include <list>
#include <ostream>
#include <unordered_map>

namespace LFUCache {

template <typename KeyT, typename T> class Cache {
  private:
    struct CacheNode;

    using Frequency = size_t;
    using ListIt = typename std::list<CacheNode>::iterator;
    using CacheMapIt = typename std::unordered_map<Frequency, std::list<CacheNode>>::iterator;

    struct CacheNode {
        KeyT key;
        T page;
        Frequency freq;
        CacheNode(KeyT nkey, T npage) : key(nkey), page(npage), freq(1) {}
    };

    size_t size_;
    std::unordered_map<KeyT, ListIt> cache_map_;
    std::unordered_map<Frequency, std::list<CacheNode>> freq_cache_map_;

    bool Full() const { return cache_map_.size() == size_; }

    void UpdateCachePage(ListIt find_page) {
      // Add new node to list with freq + 1 list in the beginning, LRU strategy
      CacheNode &move_node = *find_page;
      Frequency freq_move_node = move_node.freq; // save old frequency to erase from it this node

      ++move_node.freq;
      freq_cache_map_[move_node.freq].push_front(move_node);

      // Update cache_map_ with new iterator
      cache_map_[move_node.key] = freq_cache_map_[move_node.freq].begin();

      // Remove page from previous frequency
      freq_cache_map_[freq_move_node].erase(find_page);
      if (freq_cache_map_[freq_move_node].empty()) {
        freq_cache_map_.erase(freq_move_node);
      }
    }

    void RemoveLowFreq() {
      // Find in map list with minimal frequency
      Frequency lowest_freq
          = (*std::min_element(freq_cache_map_.begin(), freq_cache_map_.end(), [](const auto &l, const auto &r) {
              return l.first < r.first;
            })).first;
      std::list<CacheNode> &min_freq_list = freq_cache_map_[lowest_freq];

      // Remove node from cache_map_
      KeyT &key_removed_node = min_freq_list.back().key;
      cache_map_.erase(key_removed_node);

      // Remove from it LRU node and remove list if it becomes empty
      min_freq_list.pop_back();
      if (min_freq_list.empty()) {
        freq_cache_map_.erase(lowest_freq);
      }
    }

    template <typename F> void AddNewNode(const KeyT &key, F slow_get_page) {
      // Create node and push to map with 1 frequency in front of list - LRU startegy
      freq_cache_map_[1].emplace_front(CacheNode{key, slow_get_page(key)});
      cache_map_.emplace(key, freq_cache_map_[1].begin());
    }

  public:
    Cache(size_t size) : size_(size) {}

    template <typename F> bool LookUpUpdate(const KeyT &key, F slow_get_page) {
      if (size_ == 0)
        return false;

      auto hit = cache_map_.find(key);
      // Case no page in cache
      if (hit == cache_map_.end()) {
        if (Full()) {
          RemoveLowFreq();
        }
        AddNewNode(key, slow_get_page);
        return false;
      }
      // Case page in cache
      UpdateCachePage(hit->second);
      return true;
    }

    void Dump() const {
      std::cout << "\n########\n";
      size_t index = 0;
      for (CacheMapIt cache_it = freq_cache_map_.begin(); cache_it != freq_cache_map_.end(); ++cache_it, ++index) {
        std::cout << "[" << cache_it->first << "] ";
        for (ListIt list_it = cache_it->second.begin(); list_it != cache_it->second.end(); ++list_it) {
          std::cout << list_it->key << " ";
        }
        std::cout << '\n';
      }
      std::cout << "########\n\n";
    }
};

} // namespace LFUCache

#endif