#pragma once

#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>

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

#include <boost/tuple/tuple.hpp>

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

    using boost::tuples::tie;

    using boost::simd::allocator;

    typedef typename RandomAccessIterator::value_type value_type;

    using vector_t = std::vector<value_type, allocator<value_type>>;

    using pack_t = pack<value_type, 4>;

    auto elements = std::distance(first, last);

    //elements not a multiple of 16 must be handled special
    auto non_simd_elements = elements % 16;

    // sorting begins with size 16 blocks.
    auto sort_block_elements = elements - non_simd_elements;

    // typedef typename vector_t::iterator it_t;
    auto vb = input_begin<4>(first);
    auto ve = input_end<4>(first + sort_block_elements);
    auto vo = output_begin<4>(first);

    while (vb < ve) {
        pack_t a = *vb++;
        pack_t b = *vb++;
        pack_t c = *vb++;
        pack_t d = *vb++;

        tie(a, b, c, d) = detail::bitonic_sort_16(a, b, c, d);
        *vo++ = a;
        *vo++ = b;
        *vo++ = c;
        *vo++ = d;
    }

    vector_t temp(sort_block_elements);

    // compute the number of passes
    auto loops = sort_block_elements ? static_cast
                     <int32_t>(std::floor(std::log2(sort_block_elements / 16)))
                               : 0;

    size_t merge_size = 4;

    // now always run iterations per pass

    size_t remainder = 0;
    for (size_t loop = 0; loop < loops - 1; loop += 2) {
        remainder = detail::merge_pass(input_begin<4>(first),
                                        aligned_output_begin<4>(begin(temp)),
                                        sort_block_elements, merge_size,remainder);
        merge_size *= 2;
        remainder = detail::merge_pass(aligned_input_begin<4>(begin(temp)),
                                        output_begin<4>(first), sort_block_elements,
                                        merge_size, remainder);
        merge_size *= 2;
    }

    // run post fix for odd number of loops
    if (loops % 2 == 1) {
        remainder = detail::merge_pass(input_begin<4>(first),
                                        aligned_output_begin<4>(begin(temp)),
                                        sort_block_elements, merge_size,remainder);
        merge_size *= 2;
        if (remainder) {
            //perform a merge of the remaining 2 blocks.
            detail::merge_sort(aligned_input_begin<4>(begin(temp)), aligned_input_begin<4>(end(temp)  - remainder * 4), output_begin<4>(first),
                           merge_size, remainder);
        }
        else {
            std::copy(begin(temp), end(temp), first);
        }
    }
    else {
        if (remainder) {
            //perform a merge of the remaining 2 blocks.
            detail::merge_sort(input_begin<4>(first), input_begin<4>(last - non_simd_elements - (remainder * 4)), aligned_output_begin<4>(begin(temp)),
                           merge_size, remainder);
            std::copy(begin(temp), end(temp), first);
        }
    }

    if (non_simd_elements) {
        //use standard algorithm to finish off if array as not multiple of 16
        std::sort(last - non_simd_elements, last);
        std::inplace_merge(first,last - non_simd_elements, last);
    }
}
};
