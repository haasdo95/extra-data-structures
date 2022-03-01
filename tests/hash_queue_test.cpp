#include <gtest/gtest.h>
#include "hashqueue.h"

#include <vector>
#include <random>
#include <unordered_set>
#include <memory>

using namespace data_structures;

TEST(hq_test, test_cons) {
    LazyQueue<std::string> hEmpty;
    EXPECT_EQ(0, hEmpty.size());
    EXPECT_ANY_THROW(hEmpty.pop());
    EXPECT_ANY_THROW(hEmpty.peek());

    hEmpty.push(6, "hello");
    EXPECT_EQ(1, hEmpty.size());
    ASSERT_DEATH(hEmpty.push(12, "hello"), "");
    ASSERT_DEATH(hEmpty.remove("welcome"), "");

    hEmpty.remove("hello");
    EXPECT_EQ(0, hEmpty.size());
    EXPECT_ANY_THROW(hEmpty.pop());

    std::vector wrong {std::make_pair(3.5, "id"), std::make_pair(6.9, "id")};
    ASSERT_DEATH(LazyQueue hWrong{wrong}, "");

    std::vector<std::pair<double, std::string>> v;
    std::vector<double> v1 {3, 1, 2, 5, 4};
    std::vector<std::string> v2 {"a", "b", "c", "d", "e"};
    for (int i=0; i<v1.size(); i++) {
        v.emplace_back(v1[i], v2[i]);
    }
    LazyQueue<std::string> h(v);
    EXPECT_EQ(v.size(), h.size());
    h.remove("e");
    h.remove("b");
    EXPECT_EQ(v.size()-2, h.size());
    // check min-heap
    h.remove("c");
    EXPECT_EQ(h.peek().first, v[0].first);
    EXPECT_EQ(h.peek().second, v[0].second);
    EXPECT_EQ(v[0], h.pop());
    EXPECT_EQ(v[3], h.pop());
}

template<typename QT>
void test_sorting() {
    std::default_random_engine generator;
    std::exponential_distribution<double> distribution(0.1);
    int num_rolls = 1000;
    std::vector<std::pair<double, int>> nums;
    for (int i = 0; i < num_rolls; ++i) {
        double n = distribution(generator);
        auto entry = std::make_pair(n, i);
        nums.push_back(entry);
    }
    // indices to remove
    std::mt19937 rng(123);
    std::uniform_int_distribution<int> gen(0, num_rolls-1);
    int num_remove = 500;
    std::unordered_set<int> removed_indices;
    for (int i = 0; i < num_remove; ++i) {
        removed_indices.insert(i);
    }
    // remove indices from vec
    std::vector<std::pair<double, int>> after_remove;
    for (int i = 0; i < nums.size(); ++i) {
        if (!removed_indices.count(i)) {
            after_remove.emplace_back(nums[i]);
        }
    }
    std::sort(after_remove.begin(), after_remove.end());  // sort after removing the same indices from heap
    QT hq;
    for (auto& [n, i]: nums) {
        hq.push(n, i);
    }
    for (int ri : removed_indices) {
        hq.remove(ri);
    }
    // check eq
    int i=0;
    while (hq.size()) {
        EXPECT_EQ(after_remove[i], hq.pop());
        i++;
    }
}

TEST(hq_test, test_fix) {
    EagerQueue<int> q;
    q.push(0, 0);
    q.push(1, 1);
    q.push(2, 2);
    ASSERT_EQ(q.peek().first, 0);
    ASSERT_TRUE(q.contains(1));
    q.reschedule(1, 666);
    q.pop();
    ASSERT_EQ(q.peek().first, 2);
    q.pop();
    ASSERT_EQ(q.peek().first, 666);
    ASSERT_EQ(q.size(), 1);
}

TEST(hq_test, test_sort) {
    test_sorting<LazyQueue<int>>();
    test_sorting<EagerQueue<int>>();
}

struct A {
    int x, y;
    A(int x, int y): x{x}, y{y} {}
    [[nodiscard]] std::string id() const {
        std::stringstream ss;
        ss << "A(" << x << ", " << y << ")";
        return ss.str();
    }
};
struct B {
    double x, y, z;
    B(double x, double y, double z): x{x}, y{y}, z{z} {}
    [[nodiscard]] std::string id() const {
        std::stringstream ss;
        ss << "B(" << x << ", " << y << ", " << z << ")";
        return ss.str();
    }
};

TEST(hq_test, test_obj) {
    LazyQueue<A, std::string> cannot_convert{};
    EXPECT_DEATH(cannot_convert.push(0.0, 0, 1), "");
    auto get_id = [](const auto& p){ return p.id(); };
    LazyQueue<std::variant<A, B>, std::string> q{
        [&](const std::variant<A, B>& p) {
            return std::visit(get_id, p);
        }
    };
    q.push(1.0, A(0, 0));
    q.push(2.0, A(0, 1));
    q.push(3.0, A(1, 0));
    q.push(4.0, B(2, 1, 0));
    q.remove("A(1, 0)");
    q.remove("A(0, 1)");
    std::stringstream ss;
    while (q.size()) {
        auto [time, p] = q.pop();
        ss << std::visit(get_id, p)  << " @ " << time << '\n';
    }
    ASSERT_EQ(ss.str(), "A(0, 0) @ 1\n"
                        "B(2, 1, 0) @ 4\n");
}

TEST(hq_test, test_obj_simple) {
    LazyQueue<A, std::string> q{[](const A& a) { return a.id(); }};
    q.push(1.0, 0, 0);
    q.push(2.0, 0, 1);
    q.push(3.0, 1, 0);
    q.push(4.0, 1, 1);
    q.remove("A(1, 0)");
    q.remove("A(0, 1)");
    std::stringstream ss;
    while (q.size()) {
        auto [time, val] = q.pop();
        ss << val.id() << " @ " << time << '\n';
    }
    ASSERT_EQ(ss.str(), "A(0, 0) @ 1\n"
                        "A(1, 1) @ 4\n");
}