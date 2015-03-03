#include <bandit/bandit.h>
using namespace bandit;

#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <numeric>

#include <floki/kary_search.hpp>

using namespace std;

go_bandit([]() {

  describe("kary search", []() {

    it("search", [&]() {
      using key_t = int32_t;
      constexpr uint32_t k = 5;

      std::vector<key_t> sorted_values(124);
      std::iota(sorted_values.begin(), sorted_values.end(), 0);

      std::vector<key_t> linearized_values(124);

      std::vector<key_t> test_values(124);
      std::iota(test_values.begin(), test_values.end(), 10);

      floki::bfs::linearize<key_t>(&sorted_values[0], k,
                                   sorted_values.size() + 1,
                                   &linearized_values[0]);

      uint32_t sorted;
      uint32_t linearized;
      using std::placeholders::_1;
      for (auto value : test_values) {
        std::tie(sorted, linearized) = floki::bfs::search<key_t, k>(
            &linearized_values[0], &linearized_values[linearized_values.size()],
            value);
        auto stl = std::find_if(begin(sorted_values), end(sorted_values),
                                std::bind(std::equal_to<int32_t>(), _1, value));
        AssertThat(std::distance(begin(sorted_values), stl), Equals(sorted));
      }
    });
  });
});
int main(int argc, char *argv[]) { return bandit::run(argc, argv); }
