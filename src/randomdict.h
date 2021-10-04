#ifndef GSK_RANDOMDICT_H
#define GSK_RANDOMDICT_H

#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <random>
#include <unordered_set>

#include "assertions.h"

namespace data_structures {
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
        void clear() noexcept;

        size_t insert(const K& key);
        size_t erase(const K & key);
        const K & random_elem() const;
        K & random_elem();
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
    void RandomSet<K>::clear() noexcept {
        _data.clear();
        _m.clear();
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
    const K & RandomSet<K>::random_elem() const {
        if (_data.empty()) {
            throw std::runtime_error("empty dictionary");
        }
        std::uniform_int_distribution<size_t> uniform_distrib(0, _data.size()-1);
        return _data[uniform_distrib(rng)];
    }

    template<typename K>
    K & RandomSet<K>::random_elem() {
        return const_cast<K&>(const_cast<const RandomSet<K> *>(this)->random_elem());
    }


}


namespace data_structures {
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
        void clear() noexcept;

        V& at(const K& key);
        const V& at(const K& key) const; // get
        V& operator[](const K& key); // set
        size_t insert(const K& key, const V& val);
        size_t erase(const K& key);
        const std::pair<K,V>& random_pair() const;
        std::pair<K,V> & random_pair();
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
    void RandomDict<K, V>::clear() noexcept {
        _data.clear();
        _m.clear();
    }

    template<typename K, typename V>
    const V& RandomDict<K, V>::at(const K &key) const {
        auto const & itr = _m.find(key);
        if (itr == _m.end()) {
            throw std::runtime_error("key not found");
        }
        return _data[itr->second].second;
    }

    template<typename K, typename V>
    V &RandomDict<K, V>::at(const K &key) {
        return const_cast<V&>(const_cast<const RandomDict*>(this)->at(key));
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
            _m[_data[index_removed].first] = index_removed;  // redirect to index_removed
        }
        _data.pop_back();  // shrink the last entry out
        return 1;
    }

    template<typename K, typename V>
    const std::pair<K, V> & RandomDict<K, V>::random_pair() const {
        if (_data.empty()) {
            throw std::runtime_error("empty dictionary");
        }
        std::uniform_int_distribution<size_t> uniform_distrib(0, _data.size()-1);
        return _data[uniform_distrib(rng)];
    }

    template<typename K, typename V>
    std::pair<K, V> &RandomDict<K, V>::random_pair() {
        return const_cast<std::pair<K,V>&>(const_cast<const RandomDict*>(this)->random_pair());
    }
}

#endif //GSK_RANDOMDICT_H
