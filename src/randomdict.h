#ifndef GSK_RANDOMDICT_H
#define GSK_RANDOMDICT_H

#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <random>
#include <unordered_set>


namespace data_structures {
    template<typename K,
            typename Hash=std::hash<K>,
            typename KeyEqual=std::equal_to<K>>
    class RandomSet {
    private:
        // this design is possible because unordered_map never invalidates references/pointers
        std::vector<const K*> v;
        using size_type = typename decltype(v)::size_type;
        std::unordered_map<K, size_type, Hash, KeyEqual> m;
        mutable std::mt19937 rng;
    private:
        void align_v() {  // direct entries in v to keys in m
            v.clear();  // defensive coding...
            v.resize(m.size());
            for (const auto& [key, idx]: m) {
                v.at(idx) = &key;  // pointing to the entry in m
            }
        }
    public:
        explicit RandomSet(uint32_t seed): v{}, m{}, rng{seed} {}
        RandomSet() = delete;

        RandomSet(const RandomSet& other): v{}, m{other.m}, rng(other.rng) { align_v(); }
        // nothing is copied or moved by calling merge; meaning that references in v are still valid
        RandomSet(RandomSet&& other) noexcept: v{std::move(other.v)}, m{}, rng{std::move(other.rng)} {
            m.merge(std::move(other.m));
        }

        RandomSet& operator=(const RandomSet& other) {
            if (this!=&other) {
                m = other.m;
                rng = other.rng;
                align_v();
            }
            return *this;
        }

        RandomSet& operator=(RandomSet&& other) noexcept {
            if (this!=&other) {
                v = std::move(other.v);
                rng = std::move(other.rng);
                m.clear();
                m.merge(std::move(other.m));
            }
            return *this;
        }

        ~RandomSet() = default;

        auto count(const K& key) const { return m.count(key); }
        [[nodiscard]] auto size() const noexcept { return v.size(); }
        void clear() noexcept {
            v.clear();
            m.clear();
        }

        template<typename ...KT>
        int insert(KT&&... key) noexcept {
            static_assert(std::is_constructible_v<K, KT...>);
            auto [it, insertion_happens] = m.emplace(std::piecewise_construct,
                                                     std::forward_as_tuple(key...),
                                                     std::forward_as_tuple(v.size()));  // construct key in-place
            if (insertion_happens) {
                v.emplace_back(&(it->first));
                return 1;
            } else {  // value already exists -> no-op
                return 0;
            }
        }

        int erase(const K& key) noexcept {
            auto iter = m.find(key);
            if (iter == m.end()) { return 0; }  // no-op if not found
            size_type index_removed = iter->second;
            m.erase(iter);  // the corresponding ptr in v becomes dangling now
            if (index_removed != v.size() - 1) {  // not the last entry
                v[index_removed] = v.back();  // v entry redirected; not dangling anymore
                m.at(*(v.back())) = index_removed;  // m entry redirected
            }
            v.pop_back();  // shrink the last entry out
            return 1;
        }

        const K& random_elem() const {
            if (v.empty()) { throw std::runtime_error("empty set"); }
            std::uniform_int_distribution<size_type> uniform_distrib(0, v.size() - 1);
            return *(v.at(uniform_distrib(rng)));
        }
    };
}


namespace data_structures {
    template<typename K, typename V,
            typename Hash=std::hash<K>,
            typename KeyEqual=std::equal_to<K>>
    class RandomDict {
    static_assert(std::is_default_constructible_v<V>, "the mapped type, V, should be default constructible");
    private:
        std::vector<std::pair<const K*, V>> v;
        using size_type = typename decltype(v)::size_type;
        std::unordered_map<K, size_type, Hash, KeyEqual> m;
        mutable std::mt19937 rng;
    private:
        void align_v(const RandomDict& other) {
            v.clear();
            v.resize(m.size());
            for (const auto& [key, idx]: m) {
                v.at(idx) = std::make_pair(&key, other.v.at(idx).second);
            }
        }
    public:
        explicit RandomDict(uint32_t seed): v{}, m{}, rng{seed} {}
        RandomDict() = delete;

        RandomDict(const RandomDict& other): v{}, m{other.m}, rng{other.rng} { align_v(other); }
        RandomDict(RandomDict&& other) noexcept: v{std::move(other.v)}, m{}, rng(std::move(other.rng)) {
            m.merge(std::move(other.m));
        }

        RandomDict& operator=(const RandomDict& other) {
            if (this!=&other) {
                m = other.m;
                rng = other.rng;
                align_v(other);
            }
            return *this;
        }
        RandomDict& operator=(RandomDict&& other) noexcept {
            if (this!=&other) {
                v = std::move(other.v);
                rng = std::move(other.rng);
                m.clear();
                m.merge(std::move(other.m));
            }
            return *this;
        }

        auto count(const K& key) const { return m.count(key); }
        [[nodiscard]] auto size() const noexcept { return v.size(); };
        void clear() noexcept {
            v.clear();
            m.clear();
        }

        V& at(const K& key) { return v.at(m.at(key)).second; }
        const V& at(const K& key) const { return v.at(m.at(key)).second; }

        template<typename KT>
        V& operator[](KT&& key) {
            static_assert(std::is_constructible_v<K, decltype(key)>);
            auto [iter, insertion_happens] = m.emplace(std::forward<KT>(key), v.size());
            if (insertion_happens) {  // V is default-constructed here
                return v.emplace_back(std::piecewise_construct,
                                      std::forward_as_tuple(&(iter->first)), std::forward_as_tuple()).second;
            } else {
                return v.at(iter->second).second;  // ref to existing elem
            }
        }

        // use piecewise as a separator for two variadic params
        template<typename ...KT, typename ...VT>
        int emplace(std::tuple<KT...> key_args, std::tuple<VT...> val_args) {
            static_assert(std::is_constructible_v<K, KT...> and std::is_constructible_v<V, VT...>);
            auto [iter, insertion_happens] = m.emplace(std::piecewise_construct,
                                                       std::move(key_args), std::forward_as_tuple(v.size()));
            if (insertion_happens) {
                v.emplace_back(std::piecewise_construct, std::forward_as_tuple(&(iter->first)), std::move(val_args));
                return 1;
            } else {
                return 0;
            }
        }

        template<typename KT, typename VT>
        int insert(KT&& key, VT&& val) noexcept {
            static_assert(std::is_constructible_v<K, decltype(key)> and std::is_constructible_v<V, decltype(val)>);
            auto [iter, insertion_happens] = m.emplace(std::forward<KT>(key), v.size());
            if (insertion_happens) {
                v.emplace_back(&(iter->first), std::forward<VT>(val));
                return 1;
            } else {
                return 0;
            }
        }

        int erase(const K& key) {
            auto iter = m.find(key);
            if (iter == m.end()) { return 0; }  // no-op if not found
            size_type index_removed = iter->second;
            m.erase(iter);
            if (index_removed != v.size() - 1) { // not the last entry
                v.at(index_removed) = std::move(v.back());  // redirect v
                m.at(*(v.at(index_removed).first)) = index_removed;  // redirect m
            }
            v.pop_back();  // shrink the last entry out
            return 1;
        }

        std::pair<const K&, const V&> random_pair() const {
            if (v.empty()) { throw std::runtime_error("empty dictionary"); }
            std::uniform_int_distribution<size_type> uniform_distrib(0, v.size() - 1);
            auto it = std::next(v.begin(), uniform_distrib(rng));
            return {std::cref(*(it->first)), std::cref(it->second)};
        }

        std::pair<const K&, V&> random_pair() {
            if (v.empty()) { throw std::runtime_error("empty dictionary"); }
            std::uniform_int_distribution<size_type> uniform_distrib(0, v.size() - 1);
            auto it = std::next(v.begin(), uniform_distrib(rng));
            return {std::cref(*(it->first)), std::ref(it->second)};
        }
    };
}

#endif //GSK_RANDOMDICT_H
