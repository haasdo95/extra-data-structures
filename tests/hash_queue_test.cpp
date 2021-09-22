#include <gtest/gtest.h>
#include "hashqueue.h"

#include <vector>
#include <random>
#include <unordered_set>
#include <memory>

using namespace data_structures;

TEST(hqTest, testCons) {
    HashQueue<std::string> hEmpty;
    EXPECT_EQ(0, hEmpty.size());
    EXPECT_ANY_THROW(hEmpty.pop());

    hEmpty.push(6, "hello");
    EXPECT_EQ(1, hEmpty.size());
    ASSERT_DEATH(hEmpty.push(12, "hello"), ".*Assertion `false' failed");
    ASSERT_DEATH(hEmpty.remove("welcome"), ".*Assertion `false' failed");

    hEmpty.remove("hello");
    EXPECT_EQ(0, hEmpty.size());
    EXPECT_ANY_THROW(hEmpty.pop());

    std::vector wrong {std::make_pair(3.5, "id"), std::make_pair(6.9, "id")};
    ASSERT_DEATH(HashQueue hWrong{wrong}, ".*Assertion `false' failed");

    std::vector<std::pair<double, std::string>> v;
    std::vector<double> v1 {3, 1, 2, 5, 4};
    std::vector<std::string> v2 {"a", "b", "c", "d", "e"};
    for (int i=0; i<v1.size(); i++) {
        v.emplace_back(v1[i], v2[i]);
    }
    HashQueue<std::string> h(v);
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

TEST(hqTest, testSort) {
    std::default_random_engine generator;
    std::exponential_distribution<double> distribution(0.1);
    size_t num_rolls = 500000;
    std::vector<std::pair<double, int>> nums;
    for (int i = 0; i < num_rolls; ++i) {
        double n = distribution(generator);
        auto entry = std::make_pair(n, i);
        nums.push_back(entry);
    }
    // indices to remove
    std::mt19937 rng(123);
    std::uniform_int_distribution<int> gen(0, num_rolls-1);
    int num_remove = 10000;
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
    std::sort(after_remove.begin(), after_remove.end());  // sort after removing
    // remove the same indices from heap
    HashQueue hq(nums);
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

struct Base {
public:
    [[nodiscard]] virtual std::string id() const = 0;
};
struct A final: public Base {
    int x, y;
    A(int x, int y): x{x}, y{y} {}
    [[nodiscard]] std::string id() const override {
        std::stringstream ss;
        ss << "A(" << x << ", " << y << ")";
        return ss.str();
    }
};
struct B final: public Base {
    double x, y, z;
    B(double x, double y, double z): x{x}, y{y}, z{z} {}
    [[nodiscard]] std::string id() const override {
        std::stringstream ss;
        ss << "B(" << x << ", " << y << ", " << z << ")";
        return ss.str();
    }
};

TEST(hqTest, testObj) {
    HashQueue<std::unique_ptr<Base>, std::string> q{[](const std::unique_ptr<Base>& p) { return p->id(); }};
    q.push(0.0, std::make_unique<A>(0, 1));
    q.push(1.0, std::make_unique<B>(0, 1, 2));
    q.push(2.0, std::make_unique<A>(1, 0));
    q.push(3.0, std::make_unique<B>(2, 1, 0));
    q.remove("A(1, 0)");
    std::stringstream ss;
    while (q.size()) {
        auto [time, p] = q.pop();
        ss << p->id()  << " @ " << time << '\n';
    }
    ASSERT_EQ(ss.str(), "A(0, 1) @ 0\n"
                        "B(0, 1, 2) @ 1\n"
                        "B(2, 1, 0) @ 3\n");
}