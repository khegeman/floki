#include <iostream>

#include <algorithm>
#include <numeric>

#include <floki/algorithms.hpp>

#include <bandit/bandit.h>

using namespace std;
using namespace bandit;
go_bandit([]() {

  describe("test find if", []() {

    it("test find if equal to", [&]() {

      int32_t values[] = { 1, 2, 3, 4, 8, 9, 17, 1, 2 };
      using std::placeholders::_1;
      int32_t test_values[] = { 18, 23, 35, 4,  8,  9, 17, 18,
                                19, 28, 44, 54, 11, 3, 2,  1 };

      for (auto value : test_values) {
        auto simd = floki::find_if(begin(values), end(values),
                                   std::bind(floki::equal_to(), _1, value));
        auto stl = std::find_if(begin(values), end(values),
                                std::bind(std::equal_to<int32_t>(), _1, value));
        AssertThat(stl, Equals(simd));
      }
    });

    it("test find if greater equal", [&]() {

      uint32_t values[9];
      std::iota(begin(values), end(values),
                std::numeric_limits<uint32_t>::max() - 20);
      using std::placeholders::_1;
      uint32_t test_values[50];
      std::iota(begin(test_values), end(test_values),
                std::numeric_limits<uint32_t>::max() - 50);

      for (auto value : test_values) {
        auto simd =
            floki::find_if(begin(values), end(values),
                           std::bind(floki::greater_equal(), _1, value));
        auto stl =
            std::find_if(begin(values), end(values),
                         std::bind(std::greater_equal<uint32_t>(), _1, value));
        AssertThat(stl, Equals(simd));
      }
    });
    it("test greater_equal uint64_t", [&]() {

      uint64_t values[9];
      std::iota(begin(values), end(values),
                std::numeric_limits<uint64_t>::max() - 20);
      using std::placeholders::_1;
      uint32_t test_values[50];
      std::iota(begin(test_values), end(test_values),
                std::numeric_limits<uint64_t>::max() - 50);

      for (auto value : test_values) {
        auto simd =
            floki::find_if(begin(values), end(values),
                           std::bind(floki::greater_equal(), _1, value));
        auto stl =
            std::find_if(begin(values), end(values),
                         std::bind(std::greater_equal<uint64_t>(), _1, value));
        AssertThat(stl, Equals(simd));
      }
    });
  });
});
int main(int argc, char *argv[]) { return bandit::run(argc, argv); }
