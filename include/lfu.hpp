#ifndef INCLUDE_LFU_HPP
#define INCLUDE_LFU_HPP

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
      CacheNode move_node = *find_page;
      move_node.freq++;
      freq_cache_map_[move_node.freq].push_front(move_node);
      
      // Update cache_map_ with new iterator
      cache_map_[move_node.key] = freq_cache_map_[move_node.freq].begin();

      // Remove page from previous frequency
      freq_cache_map_[move_node.freq-1].erase(find_page);
      if (freq_cache_map_[move_node.freq-1].empty()) {
        freq_cache_map_.erase(move_node.freq-1);
      }
    }

    void RemoveLowFreq() {
      // Find in map list with minimal frequency
      Frequency lowest_freq = freq_cache_map_.begin()->first;
      for (CacheMapIt it = freq_cache_map_.begin(); it != freq_cache_map_.end(); ++it) {
        if (it->first <= lowest_freq) {
          lowest_freq = it->first;
        }
      }
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

    template <typename F> void AddNewNode(KeyT &key, F slow_get_page) {
      // Create node and push to map with 1 frequency in front of list - LRU startegy 
      freq_cache_map_[1].emplace_front(CacheNode {key, slow_get_page(key)});
      cache_map_.emplace(key, freq_cache_map_[1].begin());
    }

  public:
    Cache(size_t size) : size_(size) {}
    
    template <typename F> bool LookUpUpdate(KeyT &key, F slow_get_page) {
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

    void Dump() {
      std::cout << "########" << std::endl;
      size_t index = 0;
      for (CacheMapIt cache_it = freq_cache_map_.begin(); cache_it != freq_cache_map_.end(); ++cache_it, ++index) {
        std::cout << "[" << cache_it->first << "] ";
        for (ListIt list_it = cache_it->second.begin(); list_it != cache_it->second.end(); ++list_it) {
          std::cout << list_it->key << " ";
        }
        std::cout << std::endl;
      }
      std::cout << "########" << std::endl << std::endl;
    }
};

} // namespace LFUCache

#endif