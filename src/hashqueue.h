//
// Created by dwd on 4/5/21.
//

#ifndef GSK_HASHQUEUE_H
#define GSK_HASHQUEUE_H

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <iostream>

// T better have a working copy cons...
template<typename T>
struct Entry {
    double time;
    T payload;
    bool exist;
};


template<typename T>
class HashQueue {
private:
    size_t _size;
    std::vector<Entry<T>*> _data;
    std::unordered_map<T, Entry<T>*> _m;
public:
    HashQueue(): _size{0}, _data{}, _m{} {};

    explicit HashQueue(const std::vector<std::pair<double, T>> & data);
    ~HashQueue();
    HashQueue(const HashQueue<T> & other) = delete;
    HashQueue(HashQueue<T> && other) = delete;
    HashQueue& operator=(const HashQueue<T> & other) = delete;
    HashQueue& operator=(HashQueue<T> && other) = delete;

    void push(double time, const T& payload);
    std::pair<double, T> pop();
    std::pair<double, T> peek();
    void remove(const T& payload);
    size_t size() { return _size; }

};

template<typename T>
bool compare(Entry<T>* & e1, Entry<T>* & e2) {
    // true if e1 less than e2 => turn into a min-heap
    return e1->time > e2->time;
}

template<typename T>
HashQueue<T>::HashQueue(const std::vector<std::pair<double, T>> & data) {
    _size = data.size();
    for (auto const& d: data) {
        double time;
        T payload;
        std::tie(time, payload) = d;
        auto * entry = new Entry<T> {
                time, payload, true
        };
        _data.push_back(entry);
        if(_m.count(payload)) {
            throw std::runtime_error("duplicate entries during initialization");
        }
        _m[payload] = entry;
    }
    std::make_heap(_data.begin(), _data.end(), compare<T>);
}

template<typename T>
HashQueue<T>::~HashQueue() {
    for (Entry<T>* e : _data) {
        delete e;
    }  // don't need to worry about map
}

template<typename T>
void HashQueue<T>::push(double time, const T& payload) {
    if (_m.count(payload)) {
        throw std::runtime_error("re-adding the existing");
    }
    _size++;
    auto* entry = new Entry<T> {
            time, payload, true
    };
    _m[payload] = entry;
    _data.push_back(entry);
    std::push_heap(_data.begin(), _data.end(), compare<T>);
}

template<typename T>
std::pair<double, T> HashQueue<T>::pop() {
    while (_size > 0) {
        std::pop_heap(_data.begin(), _data.end(), compare<T>);
        Entry<T>* entry = _data.back();
        _data.pop_back();
        Entry<T> entry_copy = *entry; // make a copy
        delete entry; // and then delete
        if (entry_copy.exist) {
            _size--;
            _m.erase(entry_copy.payload);
            return std::make_pair(entry_copy.time, entry_copy.payload);
        }
    }
    throw std::runtime_error("Empty Queue");
}

template<typename T>
std::pair<double, T> HashQueue<T>::peek() {
    while (_size > 0) {
        Entry<T>* entry = _data[0];
        if (entry->exist) {
            return std::make_pair(entry->time, entry->payload);
        }
        // filter out non-existing entries
        std::pop_heap(_data.begin(), _data.end(), compare<T>);
        _data.pop_back();
        delete entry; // and then delete
    }
    throw std::runtime_error("Empty Queue");
}


template<typename T>
void HashQueue<T>::remove(const T& payload) {
    if(!_m.count(payload) || !_m.at(payload)->exist) {
        throw std::runtime_error("removing the non-existent");
    }
    Entry<T>* entry = _m.at(payload);
    _m.erase(payload);
    entry->exist = false; // mark as deleted
    _size--;
}



#endif //GSK_HASHQUEUE_H
