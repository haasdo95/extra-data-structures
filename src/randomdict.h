//
// Created by dwd on 4/6/21.
//

#ifndef GSK_RANDOMDICT_H
#define GSK_RANDOMDICT_H

#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <random>
#include <unordered_set>

#include "assertions.h"


template<typename K>
class RandomSet {
    std::vector<K> _data;
    std::unordered_map<K, size_t> _m;
    mutable std::mt19937 rng;
public:
    explicit RandomSet(uint32_t seed);
    RandomSet() = delete;
    size_t count(const K & key) const;
    [[nodiscard]] size_t size() const;
    size_t insert(const K & key);
    size_t erase(const K & key);
    const K & randomElem() const;
    K & randomElem();
};

template<typename K>
RandomSet<K>::RandomSet(uint32_t seed): _data{}, _m{}, rng{seed} {}

template<typename K>
size_t RandomSet<K>::count(const K &key) const {
    return _m.count(key);
}

template<typename K>
size_t RandomSet<K>::size() const {
    return _data.size();
}

template<typename K>
size_t RandomSet<K>::erase(const K & key) {
    auto const & iter = _m.find(key);
    if (iter == _m.end()) { // no-op_ if not found
        return 0;
    }
    size_t index_removed = iter->second;
    _m.erase(key);
    if (index_removed != _data.size()-1) { // not the last entry
        _data[index_removed] = std::move(_data.back());
        ASSERT(_m.at(_data[index_removed]) == _data.size()-1, "unsuccessful move");
        _m[_data[index_removed]] = index_removed;  // redirect to index_removed
    }
    _data.pop_back();  // shrink the last entry out
    return 1;
}

template<typename K>
size_t RandomSet<K>::insert(const K & key) {
    if (!_m.count(key)) {  // no-op_ if already existing
        _data.emplace_back(key);
        _m[key] = _data.size()-1;
        return 1;
    }
    return 0;
}

template<typename K>
const K & RandomSet<K>::randomElem() const {
    if (_data.empty()) {
        throw std::runtime_error("empty dictionary");
    }
    std::uniform_int_distribution<size_t> uniform_distrib(0, _data.size()-1);
    return _data[uniform_distrib(rng)];
}

template<typename K>
K & RandomSet<K>::randomElem() {
    return const_cast<K&>(const_cast<const RandomSet<K>*>(this)->randomElem());
}


template<typename K, typename V>
class RandomDict {
private:
    std::vector<std::pair<K, V>> _data;
    std::unordered_map<K, size_t> _m;
    mutable std::mt19937 rng;
public:
    explicit RandomDict(uint32_t seed);
    RandomDict() = delete;

    size_t count(const K & key) const;
    [[nodiscard]] size_t size() const;
    // CAVEAT: these two have the same weird behavior as std::map and std::unordered_map
    // rd[key] will always insert a pair (key, NodeType{}) if key is not found
    V at(const K & key) const; // get
    V& operator[](const K & key); // set
    size_t insert(const K & key, const V & val);
    size_t erase(const K & key);
    const std::pair<K,V> & randomPair() const;
    std::pair<K,V> & randomPair();
};

template<typename K, typename V>
RandomDict<K, V>::RandomDict(uint32_t seed): _data{}, _m{}, rng{seed} {}

template<typename K, typename V>
size_t RandomDict<K, V>::count(const K &key) const {
    return _m.count(key);
}

template<typename K, typename V>
size_t RandomDict<K, V>::size() const {
    return _data.size();
}

template<typename K, typename V>
V RandomDict<K, V>::at(const K &key) const {
    auto const & itr = _m.find(key);
    if (itr == _m.end()) {
        throw std::runtime_error("key not found");
    }
    return _data[itr->second].second;
}

template<typename K, typename V>
V &RandomDict<K, V>::operator[](const K &key) {
    if (!_m.count(key)) {  // inserting a new key
        _data.emplace_back(key, V{});
        _m[key] = _data.size()-1; // map points to the last entry
        return _data[_data.size()-1].second;
    } else { // modifying an existing key
        return _data[_m.at(key)].second;
    }
}

template<typename K, typename V>
size_t RandomDict<K, V>::insert(const K & key, const V & val) {
    if (_m.count(key)) {
        return 0;
    }
    this->operator[](key) = val;
    return 1;
}

template<typename K, typename V>
size_t RandomDict<K, V>::erase(const K &key) {
    auto const & iter = _m.find(key);
    if (iter == _m.end()) { // no-op_ if not found
        return 0;
    }
    size_t index_removed = iter->second;
    _m.erase(key);
    if (index_removed != _data.size()-1) { // not the last entry
        _data[index_removed] = std::move(_data.back());
        ASSERT(_m.at(_data[index_removed].first) == _data.size()-1, "unsuccessful move");
        _m[_data[index_removed].first] = index_removed;  // redirect to index_removed
    }
    _data.pop_back();  // shrink the last entry out
    return 1;
}

template<typename K, typename V>
const std::pair<K, V> & RandomDict<K, V>::randomPair() const {
    if (_data.empty()) {
        throw std::runtime_error("empty dictionary");
    }
    std::uniform_int_distribution<size_t> uniform_distrib(0, _data.size()-1);
    return _data[uniform_distrib(rng)];
}

template<typename K, typename V>
std::pair<K, V> &RandomDict<K, V>::randomPair() {
    return const_cast<std::pair<K,V>&>(const_cast<const RandomDict<K,V>*>(this)->randomPair());
}


#endif //GSK_RANDOMDICT_H