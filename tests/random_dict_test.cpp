#include "gtest/gtest.h"
#include "randomdict.h"
using namespace data_structures;

TEST(rdTest, testSimple) {
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

TEST(rdTest, testSeeding) {
    size_t total_size = 50000;
    auto rd1 = make_rd(123, total_size);
    auto rd2 = make_rd(123, total_size);

    for (int i = 0; i < total_size; ++i) {
        auto p1 = rd1.random_pair();
        auto p2 = rd2.random_pair();
        EXPECT_EQ(p1, p2);
        EXPECT_EQ(rd1.erase(p1.first), 1);
        EXPECT_EQ(rd1.erase(p1.first), 0);
        rd2.erase(p2.first);
    }
}

TEST(rsTest, testSimple) {
    RandomSet<int> rs(123);
    EXPECT_ANY_THROW(rs.random_elem());
    EXPECT_EQ(rs.size(), 0);
    rs.insert(1);
    rs.insert(2);
    rs.insert(3);
    EXPECT_EQ(rs.size(), 3);
    EXPECT_EQ(rs.count(2), 1);
    EXPECT_EQ(rs.count(4), 0);
    rs.insert(3);
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

TEST(rsTest, testSeeding) {
    size_t total_size = 50000;
    auto rs1 = make_rs(123, total_size);
    auto rs2 = make_rs(123, total_size);
    for (int i = 0; i < total_size; ++i) {
        EXPECT_EQ(rs1.random_elem(), rs2.random_elem());
        EXPECT_EQ(rs1.erase(rs1.random_elem()), 1);
        EXPECT_EQ(rs2.erase(rs2.random_elem()), 1);
    }
}
