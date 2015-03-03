#pragma once

#include <cmath>
#include <cassert>
#include <tuple>
#include <iostream>

#include "algorithms.hpp"

namespace floki {

namespace {

/*
 * Linearization functions
 */
inline uint32_t S(uint32_t R, uint32_t k, uint32_t N) {
  return N / (std::pow(k, R + 1));
}
}

namespace bfs {
/*
 * Linearization function for breadth first kary search
 * section 3.2 equation 1.
 */
namespace {
inline uint32_t P(uint32_t p, uint32_t R, uint32_t k, uint32_t N) {
  auto S_R = S(R, k, N);
  auto t = (p + 1) % S_R;
  uint32_t ret = 0;
  if (t == 0) {
    ret = ((p + 1) / S(R - 1, k, N)) * (k - 1) + (((p + 1) % (S_R * k)) / S_R) -
          1;
  } else {
    ret = P(p, R + 1, k, N) + std::pow(k, R) * (k - 1);
  }
  assert(ret < N - 1);
  return ret;
}
}

/**
 * @brief linearize a sorted array for bfs
 * @details both input and output arrays should have N-1 elements
 *
 */
template <typename key_type>
void linearize(const key_type *sorted_array, uint32_t k, uint32_t N,
               key_type *linearized_array) {
  for (uint32_t i = 0; i < N - 1; ++i) {
    linearized_array[P(i, 0, k, N)] = sorted_array[i];
  }
}

/**
 * Breadth first search algorithm 5
 * returns a tuple containing the index in the original sorted array
 * and the index in the linearized array
 */
template <typename T, uint32_t k>
inline std::tuple<uint32_t, uint32_t> search(const T *begin, const T *end,
                                             T key) {
  uint32_t sorted_position = 0;
  uint32_t level_count = 1;

  auto base_ptr = begin;
  auto key_ptr = begin;

  uint32_t position = 0;
  while (base_ptr < end) {
    key_ptr = base_ptr + sorted_position * (k - 1);
    sorted_position *= k;
    using std::placeholders::_1;
    auto ge = floki::find_if(key_ptr, key_ptr + (k - 1),
                             std::bind(floki::greater_equal(), _1, key));

    position = std::distance(key_ptr, ge);
    assert(position < k);

    sorted_position += position;
    base_ptr += level_count * (k - 1);
    level_count *= k;
  }

  return std::make_tuple(sorted_position,
                         std::distance(begin, key_ptr) + position);
}
}
}
