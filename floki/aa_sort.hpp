#pragma once

#include <vector>
#include <cassert>
#include <iostream>

#include <boost/simd/memory/allocator.hpp>

#include <boost/simd/include/functions/simd/interleave_first.hpp>
#include <boost/simd/include/functions/simd/interleave_second.hpp>
#include <boost/simd/include/functions/simd/reverse.hpp>
#include <boost/simd/include/functions/simd/shuffle.hpp>
#include <boost/simd/include/functions/simd/min.hpp>
#include <boost/simd/include/functions/simd/max.hpp>

#include <boost/simd/memory/input_iterator.hpp>
#include <boost/simd/memory/output_iterator.hpp>

#include <boost/simd/memory/aligned_input_iterator.hpp>
#include <boost/simd/memory/aligned_output_iterator.hpp>

namespace floki
{

namespace detail
{
#include <floki/detail/aa_sort.hpp>
}

/**
 * sorts vector in place using AA sort algorithm.
 * http://seven-degrees-of-freedom.blogspot.com/2010/07/question-of-sorts.html
 *
 * Note: vector size must be power of 2 currently to use SIMD path, std::sort is
 *used as a fallback.
 */
template <class RandomAccessIterator>
inline void sort(RandomAccessIterator first, RandomAccessIterator last)
{
    using boost::simd::pack;
    using boost::simd::input_end;
    using boost::simd::input_begin;
    using boost::simd::output_begin;

    using boost::simd::aligned_input_begin;
    using boost::simd::aligned_output_begin;

    using boost::simd::allocator;

    typedef typename RandomAccessIterator::value_type value_type;

    using vector_t = std::vector<value_type, allocator<value_type>>;

    using pack_t = pack<value_type, 4>;

    auto elements = std::distance(first, last);

    // sorting begins with size 16 blocks.
    auto simd_elements = (elements / 16) * 16;

    // must be power of 2 for now
    bool is_power_2
        = (!(simd_elements == 0) && !(simd_elements & (simd_elements - 1)));
    if (!is_power_2) {
        std::cerr << "Warning: sort not vectorized.  vector size must be a "
                     "power of 2." << std::endl;
        std::sort(first, last);
        return;
    }

    // typedef typename vector_t::iterator it_t;
    auto vb = input_begin<4>(first);
    auto ve = input_end<4>(first + simd_elements);
    auto vo = output_begin<4>(first);

    while (vb < ve) {
        pack_t a = *vb++;
        pack_t b = *vb++;
        pack_t c = *vb++;
        pack_t d = *vb++;

        detail::bitonic_sort_16(a, b, c, d);
        *vo++ = a;
        *vo++ = b;
        *vo++ = c;
        *vo++ = d;
    }

    vector_t temp(simd_elements);

    // compute the number of passes
    auto loops = simd_elements ? static_cast
                     <int32_t>(std::floor(std::log2(simd_elements / 16)))
                               : 0;

    size_t merge_size = 4;

    // now always run iterations per pass

    for (size_t loop = 0; loop < loops - 1; loop += 2) {
        merge_size = detail::merge_pass(input_begin<4>(first),
                                        aligned_output_begin<4>(begin(temp)),
                                        simd_elements, merge_size);
        merge_size = detail::merge_pass(aligned_input_begin<4>(begin(temp)),
                                        output_begin<4>(first), simd_elements,
                                        merge_size);
    }

    // run post fix for odd number of loops
    if (loops % 2 == 1) {
        merge_size = detail::merge_pass(input_begin<4>(first),
                                        aligned_output_begin<4>(begin(temp)),
                                        simd_elements, merge_size);
        std::copy(begin(temp), begin(temp) + simd_elements, first);
    }
}
};
