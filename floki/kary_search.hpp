#pragma once

#include <cmath>
#include <cassert>
#include <tuple>
#include <iostream>

#include "algorithms.hpp"

#include <boost/iterator.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/permutation_iterator.hpp>
#include <boost/functional.hpp>
#include <boost/bind.hpp>
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

struct doP {
  doP(uint32_t kk, uint32_t NN) : k(kk), N(NN) {}
  typedef uint32_t argument_type;
  typedef uint32_t result_type;

  result_type operator()(uint32_t p) const { return P(p, 0, k, N); }
  uint32_t k;
  uint32_t N;
};
using transform_t = doP;

using ci_t = boost::counting_iterator<uint32_t>;
using ti_t = boost::transform_iterator<transform_t, ci_t>;

template <typename InputIterator>
boost::permutation_iterator<InputIterator, ti_t>
make_iterator(InputIterator values, uint32_t k, uint32_t N, uint32_t position) {
  return boost::make_permutation_iterator(
      values,
      boost::make_transform_iterator(ci_t(position), transform_t(k, N)));
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
  std::copy(sorted_array, sorted_array + (N - 1),
            make_iterator(linearized_array, k, N, 0));
}

/**
 * Breadth first search algorithm 5
 * returns a tuple containing the index in the original sorted array
 * and the index in the linearized array
 */

template <typename T, uint32_t k>
inline uint32_t search(const T *begin, const T *end,
                                             T key) {
  uint32_t sorted_position = 0;
  uint32_t level_count = 1;

  auto base_ptr = begin;
  auto key_ptr = begin;

  while (base_ptr < end) {
    key_ptr = base_ptr + sorted_position * (k - 1);
    sorted_position *= k;
    using std::placeholders::_1;
    auto ge = floki::find_if(key_ptr, key_ptr + (k - 1),
                             std::bind(floki::greater_equal(), _1, key));

    auto position = std::distance(key_ptr, ge);
    assert(position < k);

    sorted_position += position;
    base_ptr += level_count * (k - 1);
    level_count *= k;
  }

  return sorted_position;
}

template <uint32_t k, typename InputIt, typename OutputIt>
void linearize(InputIt first, InputIt last, OutputIt d_first) {

  auto N = std::distance(first, last) + 1;
  assert(N >= k);
  std::copy(first, last, make_iterator(&d_first[0], k, N, 0));
}

/**
 * immutable kary search tree
 */

template <typename T, uint32_t k, uint32_t N> class kary_tree {

public:
  using value_t = T;
  const static uint32_t length = N - 1;
  template <typename InputIt> kary_tree(InputIt begin, InputIt end) {

    linearize<k>(begin, end, m_values);
  }

  using const_iterator_t = decltype(make_iterator<const value_t *>(0, k, N, 0));

  const_iterator_t begin() const {
    return make_iterator(&m_values[0], k, N, 0);
  }

  const_iterator_t end() const {
    return make_iterator(&m_values[0], k, N, N - 1);
  }

  const_iterator_t search(T key) const {
    return make_iterator(m_values, k, N, std::min(bfs::search<T, k>(&m_values[0], &m_values[N - 1], key), N - 1));
  }

private:
  value_t m_values[length];
};
}
}
