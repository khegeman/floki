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

/**
 * simd minmax
 * returns a 2 element tuple where the elements are defined as follows.
 * min(a,b) = (min(a[0],b[0]),min(a[1],b[1]) ...min(a[n],b[n]) )
 * max(a,b) = (max(a[0],b[0]),max(a[1],b[1]) ...max(a[n],b[n]) )
 */
template <typename simd_type>
inline tuple<simd_type, simd_type> minmax(simd_type a, simd_type b)
{
    return make_tuple(min(a, b), max(a, b));
}

/**
 * sort columns
 * returns a 4 element tuple (a',b',c',d') where each element is sorted such
 * that
 * a'[0] <= a'[1] <= a'[2] <= a'[3]
 * b'[0] <= b'[1] <= b'[2] <= b'[3]
 * c'[0] <= c'[1] <= c'[2] <= c'[3]
 * d'[0] <= d'[1] <= d'[2] <= d'[3]
 */
template <typename simd_type>
inline tuple<simd_type, simd_type, simd_type, simd_type>
sort_columns(simd_type a, simd_type b, simd_type c, simd_type d)
{
    simd_type minab, maxab, mincd, maxcd, s, t;

    // sort columns of 4x4 matrix
    tie(minab, maxab) = minmax(a, b);
    tie(mincd, maxcd) = minmax(c, d);

    tie(a, s) = minmax(minab, mincd);
    tie(t, d) = minmax(maxab, maxcd);

    tie(b, c) = minmax(s, t);

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

/*
 * SIMD merge sort routine for 4 element vectors
 * given 2 simd vectors where
 * a[0] <= a[1] <= a[2] <= a[3] and
 * b[0] <= b[1] <= b[2] <= b[3]
 * this function returns a 2 element tuple (a',b') such that
 * a'[0] <= a'[1] <= a'[2] <= a'[3] <= a'[4] <= b'[0] <= b'[1] <= b'[2] <= b'[3]
 */
template <typename simd_type>
inline tuple<simd_type, simd_type> merge_sorted_vectors(simd_type a,
                                                        simd_type b)
{
    simd_type A, B, min2, max2, min3, max3;

    tie(A, B) = minmax(a, b);

    B = shuffle<2, 3, 0, 1>(B);

    tie(min2, max2) = minmax(A, B);

    A = shuffle<0, 1, 3, 7>(min2, max2);
    B = shuffle<5, 2, 6, 4>(min2, max2);

    tie(min3, max3) = minmax(A, B);

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

    tie(a, c) = minmax(a, dr);
    tie(b, d) = minmax(b, cr);

    return make_tuple(a, b, c, d);
}

template <typename simd_type> inline simd_type bitmerge1(simd_type XYZW)
{
    simd_type min1, max1, min2, max2;

    simd_type ZWXY = shuffle<2, 3, 0, 1>(XYZW);

    tie(min1, max1) = minmax(XYZW, ZWXY);

    simd_type XYAB = shuffle<0, 1, 4, 5>(min1, max1);
    simd_type YXBA = shuffle<1, 0, 5, 4>(min1, max1);

    tie(min2, max2) = minmax(XYAB, YXBA);

    return shuffle<0, 4, 2, 6>(min2, max2);
}

template <typename simd_type>
inline tuple<simd_type, simd_type, simd_type, simd_type>
bitonic_merge(simd_type a, simd_type b, simd_type c, simd_type d)
{
    tie(a, b, c, d) = bitmerge3(a, b, c, d);
    // bitmerge2 is the same as minmax
    tie(a, b) = minmax(a, b);
    tie(c, d) = minmax(c, d);
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
