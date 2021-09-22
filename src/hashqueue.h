#ifndef GSK_HASHQUEUE_H
#define GSK_HASHQUEUE_H

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <iostream>

#include "assertions.h"

namespace data_structures::details {
    template<typename T>
    struct Entry {
        template<typename ...Arg>
        Entry(double time, bool exist, Arg&&... args): time{time}, exist{exist}, payload{std::forward<Arg>(args)...} {}
        double time;
        T payload;
        bool exist;
    };

    template<typename T>
    inline bool compare(Entry<T>* e1, Entry<T>* e2) {
        // true if e1 less than e2 => turn into a min-heap
        return e1->time > e2->time;
    }
}

namespace data_structures {
    template<typename T, typename ID=T>
    class HashQueue {
    private:
        std::optional<std::function<ID(const T& payload)>> id_func;
        size_t _size{};
        std::vector<details::Entry<T>*> _data;
        std::unordered_map<ID, details::Entry<T>*> _m;
    private:
        ID convert(const T& payload) const {  // resolve ID converter
            if (id_func) {
                return id_func.value()(payload);
            } else {
                if constexpr(std::is_convertible_v<T, ID>) {
                    return payload;
                } else if constexpr(std::is_constructible_v<ID, T>) {
                    return static_cast<ID>(payload);
                } else {
                    throw std::runtime_error("unable to convert payload to ID");
                }
            }
        }
    public:
        HashQueue() = default;

        template<typename F, std::enable_if_t<std::is_constructible_v<decltype(id_func), F>, bool> = true>
        explicit HashQueue(F f): id_func{f} {}

        template<typename F>
        HashQueue(F f, const std::vector<std::pair<double, T>>& data): id_func{f} {
            _size = data.size();
            for (const auto& [time, payload]: data) {
                ID id = convert(payload);
                ASSERT(!_m.count(id), "duplicate entries during initialization");
                auto * entry = new details::Entry<T>(time, true, payload);
                _data.push_back(entry);
                _m[std::move(id)] = entry;
            }
            std::make_heap(_data.begin(), _data.end(), details::compare<T>);
        }

        explicit HashQueue(const std::vector<std::pair<double, T>>& data): HashQueue(std::nullopt, data) {}

        ~HashQueue() {
            for (details::Entry<T>* e : _data) {
                delete e;
            }  // don't need to delete for map
        }

        HashQueue(const HashQueue<T>& other) = delete;
        HashQueue(HashQueue<T>&& other) = delete;
        HashQueue& operator=(const HashQueue<T> & other) = delete;
        HashQueue& operator=(HashQueue<T> && other) = delete;

        template<typename ...Arg>
        void push(double time, Arg&&... args) {
            auto* entry = new details::Entry<T>(time, true, std::forward<Arg>(args)...);
            ID id = convert(entry->payload);
            ASSERT(!_m.count(id), "re-adding the existing");
            ++_size;
            _m[std::move(id)] = entry;
            _data.push_back(entry);
            std::push_heap(_data.begin(), _data.end(), details::compare<T>);
        }

        std::pair<double, T> pop() {
            while (_size > 0) {
                std::pop_heap(_data.begin(), _data.end(), details::compare<T>);
                details::Entry<T>* entry = _data.back();
                _data.pop_back();
                details::Entry<T> entry_copy = std::move(*entry); // move the content of entry out
                delete entry;  // and then delete
                if (entry_copy.exist) {
                    _size--;
                    _m.erase(convert(entry_copy.payload));
                    return { entry_copy.time, std::move(entry_copy.payload) };
                }
            }
            throw std::runtime_error("Empty Queue");
        }

        std::pair<double, const T&> peek() {
            while (_size > 0) {
                details::Entry<T>* entry = _data[0];
                if (entry->exist) {
                    return { entry->time, entry->payload };
                }
                // filter out non-existing entries
                std::pop_heap(_data.begin(), _data.end(), details::compare<T>);
                _data.pop_back();
                delete entry; // and then delete
            }
            throw std::runtime_error("Empty Queue");
        }

        void remove(const ID& id) {
            ASSERT(_m.count(id) && _m.at(id)->exist, "removing the non-existent");
            details::Entry<T>* entry = _m.at(id);
            _m.erase(id);
            entry->exist = false; // mark as deleted
            _size--;
        }
        [[nodiscard]] size_t size() const { return _size; }
    };
}

#endif //GSK_HASHQUEUE_H
