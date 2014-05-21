#pragma once

using boost::simd::interleave_first;
using boost::simd::interleave_second;
using boost::simd::shuffle;
using boost::simd::pack;
using boost::simd::reverse;
using boost::simd::broadcast;
using boost::simd::min;
using boost::simd::max;
using boost::simd::is_less;
using boost::simd::input_begin;
using boost::simd::output_begin;

template <typename simd_type>
inline void sort_columns(simd_type &a, simd_type &b, simd_type &c, simd_type &d)
{

    // sort columns of 4x4 matrix
    simd_type minab = min(a, b);
    simd_type maxab = max(a, b);
    simd_type mincd = min(c, d);
    simd_type maxcd = max(c, d);

    a = min(minab, mincd);

    d = max(maxab, maxcd);

    simd_type s = max(minab, mincd);
    simd_type t = min(maxab, maxcd);

    b = min(s, t);
    c = max(s, t);
}

template <typename simd_type>
inline void transpose4(simd_type &a, simd_type &b, simd_type &c, simd_type &d)
{

    simd_type x = interleave_first(a, b);
    simd_type y = interleave_second(a, b);
    simd_type z = interleave_first(c, d);
    simd_type w = interleave_second(c, d);

    a = shuffle<0, 1, 4, 5>(x, z);
    b = shuffle<2, 3, 6, 7>(x, z);
    c = shuffle<0, 1, 4, 5>(y, w);
    d = shuffle<2, 3, 6, 7>(y, w);
}

template <typename simd_type>
inline void merge_sorted_vectors(simd_type &a, simd_type &b)
{
    simd_type A = min(a, b);
    simd_type B = max(a, b);

    B = shuffle<2, 3, 0, 1>(B);

    simd_type min2 = min(A, B);
    simd_type max2 = max(A, B);

    A = shuffle<0, 1, 3, 7>(min2, max2);

    B = shuffle<5, 2, 6, 4>(min2, max2);

    simd_type min3 = min(A, B);
    simd_type max3 = max(A, B);

    a = shuffle<0, 1, 5, 2>(min3, max3);
    b = shuffle<6, 3, 7, 4>(min3, max3);
}

template <typename simd_type>
inline void bitmerge3(simd_type &a, simd_type &b, simd_type &c, simd_type &d)
{
    simd_type cr = reverse(c);
    simd_type dr = reverse(d);

    c = max(a, dr);
    d = max(b, cr);
    a = min(a, dr);
    b = min(b, cr);
}

// same as compare_swap from aA sort
template <typename simd_type> inline void bitmerge2(simd_type &a, simd_type &b)
{
    simd_type ta = min(a, b);
    b = max(a, b);
    a = ta;
}

template <typename simd_type> inline void bitmerge1(simd_type &XYZW)
{
    simd_type ZWXY = shuffle<2, 3, 0, 1>(XYZW);
    simd_type min1 = min(XYZW, ZWXY);
    simd_type max1 = max(XYZW, ZWXY);

    simd_type XYAB = shuffle<0, 1, 4, 5>(min1, max1);
    simd_type YXBA = shuffle<1, 0, 5, 4>(min1, max1);
    simd_type min2 = min(XYAB, YXBA);
    simd_type max2 = max(XYAB, YXBA);

    XYZW = shuffle<0, 4, 2, 6>(min2, max2);
}

template <typename simd_type>
inline void bitonic_merge(simd_type &a, simd_type &b, simd_type &c,
                          simd_type &d)
{
    bitmerge3(a, b, c, d);
    bitmerge2(a, b);
    bitmerge2(c, d);
    bitmerge1(a);
    bitmerge1(b);
    bitmerge1(c);
    bitmerge1(d);
}

template <typename iterator_type>
inline void merge_and_write(iterator_type &dest,
                            typename iterator_type::value_type &a1,
                            typename iterator_type::value_type &a2,
                            typename iterator_type::value_type b1,
                            typename iterator_type::value_type b2)
{
    bitonic_merge(a1, a2, b1, b2);
    *dest++ = a1;
    *dest++ = a2;
    a1 = b1;
    a2 = b2;
}

template <typename iterator_type>
inline void read_and_move_iterator(iterator_type &iter,
                                   typename iterator_type::value_type &value_1,
                                   typename iterator_type::value_type &value_2)
{
    value_1 = *iter++;
    value_2 = *iter++;
}

template <typename input_type, typename output_type>
inline void merge_sort(input_type a, input_type b, output_type dest,
                       uint32_t merge_size)
{
    using simd_type_t = typename input_type::value_type;
    auto a_end = a + merge_size;
    auto b_end = b + merge_size;

    simd_type_t a1;
    simd_type_t a2;
    read_and_move_iterator(a, a1, a2);

    simd_type_t b1;
    simd_type_t b2;
    read_and_move_iterator(b, b1, b2);

    do {

        merge_and_write(dest, a1, a2, b1, b2);

        // reference the underlying iterator
        if ((*a.base()) < (*b.base())) {
            b1 = *a++;
            b2 = *a++;
        } else {
            b1 = *b++;
            b2 = *b++;
        }

    } while (a != a_end && b != b_end);

    merge_and_write(dest, a1, a2, b1, b2);

    while (a != a_end) {
        read_and_move_iterator(a, b1, b2);
        merge_and_write(dest, a1, a2, b1, b2);
    }
    while (b != b_end) {
        read_and_move_iterator(b, b1, b2);
        merge_and_write(dest, a1, a2, b1, b2);
    }

    *dest++ = a1;
    *dest++ = a2;
}

template <typename simd_type>
inline void bitonic_sort_16(simd_type &a, simd_type &b, simd_type &c,
                            simd_type &d)
{

    // sort 16 values

    sort_columns(a, b, c, d);
    transpose4(a, b, c, d);

    merge_sorted_vectors(a, b);
    merge_sorted_vectors(c, d);
    bitonic_merge(a, b, c, d);
}

template <class InputIterator, class OutputIterator>
inline size_t merge_pass(InputIterator input, OutputIterator output,
                         size_t elements, size_t merge_size)
{

    size_t i = 0;
    for (; i < elements / 4 - merge_size; i += 2 * merge_size) {
        detail::merge_sort(input + i, input + (i + merge_size), output + i,
                           merge_size);
    }

    return merge_size * 2;
}
