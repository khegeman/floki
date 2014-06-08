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
using boost::tuples::tuple;
using boost::tuples::tie;
using boost::tuples::make_tuple;
template <typename simd_type>
inline tuple<simd_type, simd_type, simd_type, simd_type>
sort_columns(simd_type a, simd_type b, simd_type c, simd_type d)
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
    return make_tuple(a, b, c, d);
}

template <typename simd_type>
inline tuple<simd_type, simd_type, simd_type, simd_type>
transpose4(simd_type a, simd_type b, simd_type c, simd_type d)
{

    simd_type x = interleave_first(a, b);
    simd_type y = interleave_second(a, b);
    simd_type z = interleave_first(c, d);
    simd_type w = interleave_second(c, d);

    a = shuffle<0, 1, 4, 5>(x, z);
    b = shuffle<2, 3, 6, 7>(x, z);
    c = shuffle<0, 1, 4, 5>(y, w);
    d = shuffle<2, 3, 6, 7>(y, w);
    return make_tuple(a, b, c, d);
}

template <typename simd_type>
inline tuple<simd_type, simd_type> merge_sorted_vectors(simd_type a,
                                                        simd_type b)
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
    return make_tuple(a, b);
}

template <typename simd_type>
inline tuple<simd_type, simd_type, simd_type, simd_type>
bitmerge3(simd_type a, simd_type b, simd_type c, simd_type d)
{
    simd_type cr = reverse(c);
    simd_type dr = reverse(d);

    c = max(a, dr);
    d = max(b, cr);
    a = min(a, dr);
    b = min(b, cr);
    return make_tuple(a, b, c, d);
}

// same as compare_swap from aa sort
template <typename simd_type>
tuple<simd_type, simd_type> bitmerge2(simd_type a, simd_type b)
{
    return make_tuple(min(a, b), max(a, b));
}

template <typename simd_type> inline simd_type bitmerge1(simd_type XYZW)
{
    simd_type ZWXY = shuffle<2, 3, 0, 1>(XYZW);
    simd_type min1 = min(XYZW, ZWXY);
    simd_type max1 = max(XYZW, ZWXY);

    simd_type XYAB = shuffle<0, 1, 4, 5>(min1, max1);
    simd_type YXBA = shuffle<1, 0, 5, 4>(min1, max1);
    simd_type min2 = min(XYAB, YXBA);
    simd_type max2 = max(XYAB, YXBA);

    return shuffle<0, 4, 2, 6>(min2, max2);
}

template <typename simd_type>
inline tuple<simd_type, simd_type, simd_type, simd_type>
bitonic_merge(simd_type a, simd_type b, simd_type c, simd_type d)
{
    tie(a, b, c, d) = bitmerge3(a, b, c, d);
    tie(a, b) = bitmerge2(a, b);
    tie(c, d) = bitmerge2(c, d);
    a = bitmerge1(a);
    b = bitmerge1(b);
    c = bitmerge1(c);
    d = bitmerge1(d);
    return make_tuple(a, b, c, d);
}

template <typename input_type, typename output_type>
inline void merge_sort(input_type a, input_type b, output_type dest,
                       uint32_t merge_size)
{
    using simd_type_t = typename input_type::value_type;
    auto a_end = a + merge_size;
    auto b_end = b + merge_size;

    simd_type_t a1 = *a++;
    simd_type_t a2 = *a++;

    simd_type_t b1 = *b++;
    simd_type_t b2 = *b++;

    do {

        tie(a1, a2, b1, b2) = bitonic_merge(a1, a2, b1, b2);
        *dest++ = a1;
        *dest++ = a2;
        a1 = b1;
        a2 = b2;

        // reference the underlying iterator
        if ((*a.base()) < (*b.base())) {
            b1 = *a++;
            b2 = *a++;
        } else {
            b1 = *b++;
            b2 = *b++;
        }

    } while (a != a_end && b != b_end);

    tie(a1, a2, b1, b2) = bitonic_merge(a1, a2, b1, b2);
    *dest++ = a1;
    *dest++ = a2;
    a1 = b1;
    a2 = b2;

    while (a != a_end) {
        b1 = *a++;
        b2 = *a++;
        tie(a1, a2, b1, b2) = bitonic_merge(a1, a2, b1, b2);
        *dest++ = a1;
        *dest++ = a2;
        a1 = b1;
        a2 = b2;
    }
    while (b != b_end) {
        b1 = *b++;
        b2 = *b++;
        tie(a1, a2, b1, b2) = bitonic_merge(a1, a2, b1, b2);
        *dest++ = a1;
        *dest++ = a2;
        a1 = b1;
        a2 = b2;
    }

    *dest++ = a1;
    *dest++ = a2;
}

template <typename simd_type>
inline tuple<simd_type, simd_type, simd_type, simd_type>
bitonic_sort_16(simd_type a, simd_type b, simd_type c, simd_type d)
{

    // sort 16 values

    tie(a, b, c, d) = sort_columns(a, b, c, d);
    tie(a, b, c, d) = transpose4(a, b, c, d);

    tie(a, b) = merge_sorted_vectors(a, b);
    tie(c, d) = merge_sorted_vectors(c, d);
    return bitonic_merge(a, b, c, d);
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
