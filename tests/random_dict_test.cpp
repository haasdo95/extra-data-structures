#include "gtest/gtest.h"
#include "randomdict.h"
using namespace data_structures;

TEST(rd_test, test_simple) {
    RandomDict<std::string, int> rd(123);
    EXPECT_ANY_THROW(rd.random_pair());
    std::vector letters {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"};
    for (int i = 0; i < letters.size(); ++i) {
        rd[letters[i]] = i;
    }
    EXPECT_EQ(rd.count("c"), 1);
    EXPECT_EQ(rd.size(), letters.size());
    std::pair<std::string, int> p = rd.random_pair();
    rd[p.first] = 666;
    EXPECT_EQ(rd.at(p.first), 666);

    p = rd.random_pair();
    EXPECT_EQ(rd.erase(p.first), 1);
    EXPECT_EQ(rd.count(p.first), 0);
    EXPECT_ANY_THROW(rd.at(p.first));
    rd[p.first] = 777;
    EXPECT_EQ(rd.at(p.first), 777);
}

RandomDict<int, int> make_rd(uint32_t seed, size_t size) {
    RandomDict<int, int> rd(seed);
    for (int i = 0; i < size; ++i) {
        rd[i] = i;
    }
    return rd;
}

TEST(rd_test, test_seeding) {
    size_t total_size = 1000;
    auto rd1 = make_rd(123, total_size);  // move construction
    RandomDict<int, int> rd2{0};
    rd2.insert(123, 456);
    rd2.insert(1, 2);
    auto temp = make_rd(123, total_size);  // move construction
    rd2 = std::move(temp);  // move assignment
    auto rd3 = rd1;  // copy construction
    const auto& crd3 = rd3;
    rd1 = rd3;  // copy assignment
    for (int i = 0; i < total_size; ++i) {
        const auto& p1 = rd1.random_pair();
        auto p1_key = p1.first;
        const auto& p2 = rd2.random_pair();
        const auto& p3 = crd3.random_pair();
        EXPECT_EQ(p1, p2);
        EXPECT_EQ(p2.first, p3.first);
        EXPECT_EQ(p2.second, p3.second);
        EXPECT_EQ(rd1.erase(p1_key), 1);
        EXPECT_EQ(rd1.erase(p1_key), 0);  // if p1 is erased, the pair of reference becomes dangling!
        EXPECT_EQ(rd2.erase(p2.first), 1);
        EXPECT_EQ(rd3.erase(p3.first), 1);
    }
}

struct Key {
    std::string id;
    Key()=default;
    explicit Key(std::string id): id(std::move(id)) {}
    friend bool operator==(const Key& lhs, const Key& rhs) {
        return lhs.id==rhs.id;
    }
    struct Hash {
        std::size_t operator()(const Key& key) const noexcept {
            return std::hash<std::string>()(key.id);
        }
    };
};

struct Widget {
    int x;
    int y;
    Widget()=default;
    Widget(int x, int y): x{x}, y{y} {}
};

TEST(rd_test, test_obj) {
    RandomDict<Key, Widget, Key::Hash> rd{123};
    rd["first"];
    const auto& crd = rd;
    EXPECT_EQ(crd.at(Key("first")).x, 0);
    EXPECT_EQ(crd.at(Key("first")).y, 0);
    EXPECT_EQ(rd.insert("first", Widget(1, 1)), 0);  // no-op for dup
    rd.emplace(std::forward_as_tuple("second"), std::forward_as_tuple(123, 456));
    EXPECT_EQ(rd.emplace(std::forward_as_tuple("second"), std::forward_as_tuple(666, 999)), 0);  // no-op for dup
    EXPECT_EQ(rd["second"].y, 456);
}

TEST(rs_test, test_simple) {
    RandomSet<int> rs(123);
    EXPECT_ANY_THROW(rs.random_elem());
    EXPECT_EQ(rs.size(), 0);
    rs.insert(1);
    rs.insert(2);
    rs.insert(3);
    EXPECT_EQ(rs.insert(3), 0);  // no-op for dup
    rs = rs;
    EXPECT_EQ(rs.size(), 3);
    EXPECT_EQ(rs.count(2), 1);
    EXPECT_EQ(rs.count(4), 0);
    rs.insert(3);
    rs = std::move(rs);
    EXPECT_EQ(rs.size(), 3);
    EXPECT_EQ(rs.erase(666), 0);
    EXPECT_EQ(rs.erase(2), 1);
    EXPECT_EQ(rs.count(2), 0);
}

RandomSet<int> make_rs(uint32_t seed, size_t size) {
    RandomSet<int> rs{seed};
    for (int i = 0; i < size; ++i) {
        rs.insert(i);
    }
    return rs;
}

TEST(rs_test, test_seeding) {
    size_t total_size = 1000;
    auto rs1 = make_rs(123, total_size);
    RandomSet<int> rs2{666};
    auto temp = make_rs(123, total_size);
    rs2 = std::move(temp);  // move assignment
    auto rs3 = rs1;  // copy constructed
    const auto& crs3 = rs3;
    rs1 = rs3;  // copy assigned
    for (int i = 0; i < total_size; ++i) {
        const auto& e1 = rs1.random_elem();
        const auto& e2 = rs2.random_elem();
        const auto& e3 = crs3.random_elem();
        EXPECT_EQ(e1, e2);
        EXPECT_EQ(e2, e3);
        EXPECT_EQ(rs1.erase(rs1.random_elem()), 1);
        EXPECT_EQ(rs2.erase(rs2.random_elem()), 1);
        EXPECT_EQ(rs3.erase(rs3.random_elem()), 1);
    }
}
