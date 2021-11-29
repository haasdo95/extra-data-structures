#ifndef GSK_HASHQUEUE_H
#define GSK_HASHQUEUE_H

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>

namespace data_structures::detail {
    template<typename T>
    struct Entry {
        template<typename ...Arg>
        explicit Entry(double time, Arg&&... args): time{time}, payload{std::forward<Arg>(args)...}, exist{true} {}
        double time;
        T payload;
        bool exist;
    };

    template<typename T>
    inline bool compare(const Entry<T>* e1, const Entry<T>* e2) {
        // true if e1 less than e2 => turn into a min-heap
        return e1->time > e2->time;
    }
}

namespace data_structures {
    template<typename T, typename ID=T, typename Hash=std::hash<ID>, typename KeyEqual=std::equal_to<ID>>
    class HashQueue {
    private:
        std::optional<std::function<ID(const T& payload)>> id_func;
        std::vector<detail::Entry<T>*> _data;
        using size_type = typename decltype(_data)::size_type;
        size_type _size{};
        std::unordered_map<ID, detail::Entry<T>*, Hash, KeyEqual> _m;
    private:
        ID convert(const T& payload) const {  // resolve ID converter
            if (id_func) {
                return id_func.value()(payload);
            } else {
                if constexpr(std::is_convertible_v<T, ID>) {  // merely a copy if T=ID
                    return payload;
                } else if constexpr(std::is_constructible_v<ID, T>) {
                    return static_cast<ID>(payload);
                } else {
                    assert(false);
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
                assert(!_m.count(id));
                auto * entry = new detail::Entry<T>(time, payload);
                _data.push_back(entry);
                _m.emplace(std::move(id), entry);
            }
            std::make_heap(_data.begin(), _data.end(), detail::compare<T>);
        }

        explicit HashQueue(const std::vector<std::pair<double, T>>& data): HashQueue(std::nullopt, data) {}

        ~HashQueue() {
            for (detail::Entry<T>* e : _data) {
                delete e;
            }  // don't need to delete for map
        }

        HashQueue(const HashQueue<T>& other) = delete;
        HashQueue(HashQueue<T>&& other) = delete;
        HashQueue& operator=(const HashQueue<T> & other) = delete;
        HashQueue& operator=(HashQueue<T> && other) = delete;


        template<typename ...Arg>
        void push(double time, Arg&&... args) {
            auto* entry = new detail::Entry<T>(time, std::forward<Arg>(args)...);
            ID id = convert(entry->payload);
            assert(!_m.count(id));
            ++_size;
            _data.push_back(entry);
            _m.emplace(std::move(id), entry);
            std::push_heap(_data.begin(), _data.end(), detail::compare<T>);
        }

        std::pair<double, T> pop() {
            while (_size > 0) {
                std::pop_heap(_data.begin(), _data.end(), detail::compare<T>);
                detail::Entry<T>* entry = _data.back();
                _data.pop_back();
                detail::Entry<T> entry_copy = std::move(*entry); // move the content of entry out
                delete entry;  // and then delete
                if (entry_copy.exist) {
                    _size--;
                    assert(_m.count(convert(entry_copy.payload)));
                    _m.erase(convert(entry_copy.payload));
                    return { entry_copy.time, std::move(entry_copy.payload) };
                }
            }
            throw std::runtime_error("Empty Queue");
        }

        std::pair<double, const T&> peek() {
            while (_size > 0) {
                detail::Entry<T>* entry = _data[0];  // where the min time is
                if (entry->exist) {
                    return { entry->time, entry->payload };
                }
                // filter out non-existing entries
                std::pop_heap(_data.begin(), _data.end(), detail::compare<T>);
                _data.pop_back();
                delete entry; // and then delete
            }
            throw std::runtime_error("Empty Queue");
        }

        void remove(const ID& id) {
            auto it = _m.find(id);
            assert(it!=_m.end() && it->second->exist);
            detail::Entry<T>* entry = it->second;
            _m.erase(it);
            entry->exist = false; // mark as deleted
            _size--;
        }

        [[nodiscard]] auto size() const noexcept { return _size; }
    };
}

#endif //GSK_HASHQUEUE_H
