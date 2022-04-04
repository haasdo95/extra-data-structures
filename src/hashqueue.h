#ifndef GSK_HASHQUEUE_H
#define GSK_HASHQUEUE_H

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>

namespace data_structures::detail {
    template<typename T>
    struct EntryBase {
        template<typename ...Arg>
        explicit EntryBase(double time, Arg&&... args): time{time}, payload{std::forward<Arg>(args)...} {}
        double time;
        T payload;
    };

    template<typename T>
    struct LazyEntry: public EntryBase<T> {
        template<typename ...Arg>
        explicit LazyEntry(double time, Arg&&... args): EntryBase<T>(time, std::forward<Arg>(args)...), exist{true} {}
        bool exist;
    };

    template<typename T>
    struct EagerEntry: public EntryBase<T> {
        using size_type = typename std::vector<EagerEntry<T>*>::size_type;
        template<typename ...Arg>
        EagerEntry(size_type loc, double time, Arg&&... args): loc{loc}, EntryBase<T>(time, std::forward<Arg>(args)...){}
        size_type loc;  // an eager entry keeps track of its index in the vector
    };

    template<typename T>
    inline bool compare(const EntryBase<T>* e1, const EntryBase<T>* e2) {
        // true if e1 less than e2 => turn into a min-heap
        return e1->time > e2->time;
    }
}

namespace data_structures {
    template<typename T, typename ID=T, typename Hash=std::hash<ID>, typename KeyEqual=std::equal_to<ID>>
    class QueueBase {
    protected:
        std::optional<std::function<ID(const T& payload)>> id_func;

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

        QueueBase() = default;

        template<typename F, std::enable_if_t<std::is_constructible_v<decltype(id_func), F>, bool> = true>
        explicit QueueBase(F f): id_func{f} {}
    };
}

namespace data_structures {
    template<typename T, typename ID=T, typename Hash=std::hash<ID>, typename KeyEqual=std::equal_to<ID>>
    class EagerQueue: public QueueBase<T, ID, Hash, KeyEqual> {
    private:
        std::vector<detail::EagerEntry<T>*> _data;
        using size_type = typename decltype(_data)::size_type;
        std::unordered_map<ID, detail::EagerEntry<T>*, Hash, KeyEqual> _m;
    private:
        size_type left(size_type i) const { return 2*i+1; }
        size_type right(size_type i) const { return 2*i+2; }
        size_type parent(size_type i) const { return (i-1)/2; }  // UB when i==0
        bool perc_down(const size_type idx) {  // return true if any perc-down actually happens
            size_type min_idx = idx;
            const size_type left_idx = left(idx);
            const size_type right_idx = right(idx);
            if (left_idx < _data.size() && _data[left_idx]->time < _data[min_idx]->time) {
                min_idx = left_idx;
            }
            if (right_idx < _data.size() && _data[right_idx]->time < _data[min_idx]->time) {
                min_idx = right_idx;
            }
            if (min_idx == idx) {  // heap property intact on idx; no pd needed
                return false;
            } else {  // swap and update loc
                std::swap(_data[idx], _data[min_idx]);
                _data[min_idx]->loc = min_idx;
                _data[idx]->loc = idx;
                perc_down(min_idx);
                return true;
            }
        }
        void perc_up(const size_type idx) {
            if (idx == 0) { return; }
            const size_type parent_idx = parent(idx);
            if (_data[idx]->time < _data[parent_idx]->time) {
                std::swap(_data[idx], _data[parent_idx]);
                _data[idx]->loc = idx;
                _data[parent_idx]->loc = parent_idx;
                perc_up(parent_idx);
            }
        }
        // the same functionality as heap.Fix in Golang
        // restores heap property after ONE change of priority/time at idx
        void fix(const size_type idx) {
            if (!perc_down(idx)) {
                perc_up(idx);
            }
        }

    public:
        EagerQueue() = default;

        EagerQueue(const EagerQueue<T>& other) = delete;
        EagerQueue(EagerQueue<T>&& other) = delete;
        EagerQueue& operator=(const EagerQueue<T> & other) = delete;
        EagerQueue& operator=(EagerQueue<T> && other) = delete;

        ~EagerQueue() {
            for (auto* entry: _data) {
                delete entry;
            }
        }

        template<typename F>
        explicit EagerQueue(F f): QueueBase<T, ID, Hash, KeyEqual>{f} {}

        template<typename ...Arg>
        void push(double time, Arg&&... args) {
            auto* entry = new detail::EagerEntry<T>(_data.size(), time, std::forward<Arg>(args)...);
            ID id = this->convert(entry->payload);
            assert(!_m.count(id));
            _data.push_back(entry);
            _m.emplace(std::move(id), entry);
            perc_up(_data.size()-1);
        }

        std::pair<double, T> pop() {
            if (_data.empty()) {
                throw std::runtime_error("Empty Queue");
            }
            auto* top = _data[0];
            _data[0] = _data.back();
            _data.pop_back();
            _data[0]->loc = 0;
            perc_down(0);
            detail::EagerEntry<T> top_copy = std::move(*top);
            delete top;
            assert(_m.count(this->convert(top_copy.payload)));
            _m.erase(this->convert(top_copy.payload));
            return { top_copy.time, std::move(top_copy.payload) };
        }

        std::pair<double, const T&> peek() const {
            auto* entry = _data[0];
            return { entry->time, entry->payload };
        }

        void remove(const ID& id) {
            auto it = _m.find(id);
            assert(it!=_m.end());
            assert(_data.size());
            auto* removed_entry = it->second;
            _m.erase(it);
            auto loc = removed_entry->loc;
            delete removed_entry;
            if (loc==_data.size()-1) { _data.pop_back(); return; }  // will seg-fault at _data[loc]->loc otherwise
            _data[loc] = _data.back();
            _data.pop_back();
            _data[loc]->loc = loc;
            fix(loc);
        }

        void reschedule(const ID& id, double new_time) {
            auto it = _m.find(id);
            assert(it!=_m.end());
            it->second->time = new_time;
            fix(it->second->loc);
        }

        [[nodiscard]] bool contains(const ID& id) const {
            return _m.find(id) != _m.end();
        }

        [[nodiscard]] auto size() const noexcept { return _data.size(); }

    };
}

namespace data_structures {
    template<typename T, typename ID=T, typename Hash=std::hash<ID>, typename KeyEqual=std::equal_to<ID>>
    class LazyQueue: public QueueBase<T, ID, Hash, KeyEqual> {
    private:
        std::vector<detail::LazyEntry<T>*> _data;
        using size_type = typename decltype(_data)::size_type;
        size_type _size{};
        std::unordered_map<ID, detail::LazyEntry<T>*, Hash, KeyEqual> _m;
    public:
        LazyQueue() = default;

        template<typename F>
        explicit LazyQueue(F f): QueueBase<T, ID, Hash, KeyEqual>{f} {}

        template<typename F>
        LazyQueue(F f, const std::vector<std::pair<double, T>>& data): QueueBase<T, ID, Hash, KeyEqual>{f} {
            _size = data.size();
            for (const auto& [time, payload]: data) {
                ID id = this->convert(payload);
                assert(!_m.count(id));
                auto * entry = new detail::LazyEntry<T>(time, payload);
                _data.push_back(entry);
                _m.emplace(std::move(id), entry);
            }
            std::make_heap(_data.begin(), _data.end(), detail::compare<T>);
        }

        explicit LazyQueue(const std::vector<std::pair<double, T>>& data): LazyQueue(std::nullopt, data) {}

        ~LazyQueue() {
            for (detail::LazyEntry<T>* e : _data) {
                delete e;
            }  // don't need to delete for map
        }

        LazyQueue(const LazyQueue<T>& other) = delete;
        LazyQueue(LazyQueue<T>&& other) = delete;
        LazyQueue& operator=(const LazyQueue<T> & other) = delete;
        LazyQueue& operator=(LazyQueue<T> && other) = delete;


        template<typename ...Arg>
        void push(double time, Arg&&... args) {
            auto* entry = new detail::LazyEntry<T>(time, std::forward<Arg>(args)...);
            ID id = this->convert(entry->payload);
            assert(!_m.count(id));
            ++_size;
            _data.push_back(entry);
            _m.emplace(std::move(id), entry);
            std::push_heap(_data.begin(), _data.end(), detail::compare<T>);
        }

        std::pair<double, T> pop() {
            while (_size > 0) {
                std::pop_heap(_data.begin(), _data.end(), detail::compare<T>);
                detail::LazyEntry<T>* entry = _data.back();
                _data.pop_back();
                detail::LazyEntry<T> entry_copy = std::move(*entry); // move the content of entry out
                delete entry;  // and then delete
                if (entry_copy.exist) {
                    _size--;
                    assert(_m.count(this->convert(entry_copy.payload)));
                    _m.erase(this->convert(entry_copy.payload));
                    return { entry_copy.time, std::move(entry_copy.payload) };
                }
            }
            throw std::runtime_error("Empty Queue");
        }

        std::pair<double, const T&> peek() {
            while (_size > 0) {
                detail::LazyEntry<T>* entry = _data[0];  // where the min time is
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
            detail::LazyEntry<T>* entry = it->second;
            _m.erase(it);
            entry->exist = false; // mark as deleted
            _size--;
        }

        [[nodiscard]] auto size() const noexcept { return _size; }
    };
}

#endif //GSK_HASHQUEUE_H
