#include <bandit/bandit.h>
using namespace bandit;

#include <algorithm>
#include <limits>
#include <vector>
#include <random>
#include <floki/aa_sort.hpp>
#include <boost/simd/memory/input_iterator.hpp>
#include <boost/simd/memory/output_iterator.hpp>
#include <boost/simd/include/functions/shuffle.hpp>
#include <boost/simd/include/functions/min.hpp>
#include <boost/simd/include/functions/max.hpp>
#include <boost/simd/include/functions/broadcast.hpp>
#include <boost/simd/memory/allocator.hpp>

using namespace floki;

using namespace boost::simd;

using boost::tuples::tie;

template <typename element_type> void random_test(size_t elements = 16 * 256)
{
    std::vector<element_type> values(elements);
    using distribution_t = typename std::conditional
        <std::is_integral<element_type>::value,
         typename std::uniform_int_distribution<element_type>,
         typename std::uniform_real_distribution<element_type>>::type;
    distribution_t distribution;
    std::mt19937 engine;
    auto generator = std::bind(distribution, engine);
    std::generate_n(values.begin(), elements, generator);

    auto sorted_values = values;

    std::sort(begin(sorted_values), end(sorted_values));

    floki::sort(begin(values), end(values));

    AssertThat(values, EqualsContainer(sorted_values));
}

// snowhouse container equality check.  For unit testing only
template <typename pack_t>
static bool are_packs_equal(const pack_t &lhs, const pack_t &rhs)
{
    for (int i = 0; i < pack_t::static_size; ++i)
        if (lhs[i] != rhs[i])
            return false;

    return true;
}

go_bandit([]() {

    describe("test aa sort algorithm", []() {

        it("test column sort", [&]() {

            using pack_t = pack<int32_t, 4>;

            std::vector<pack_t> values{ { 50, 40, 30, 20 },
                                        { 10, 0, 60, 55 },
                                        { 88, 22, 44, 96 },
                                        { 24, 6, 4, 3 } };

            std::vector<pack_t> sorted_values{ { 10, 0, 4, 3 },
                                               { 24, 6, 30, 20 },
                                               { 50, 22, 44, 55 },
                                               { 88, 40, 60, 96 } };

            tie(values[0], values[1], values[2], values[3])
                = floki::detail::sort_columns(values[0], values[1], values[2],
                                              values[3]);

            AssertThat(values,
                       EqualsContainer(sorted_values, are_packs_equal<pack_t>));
        });

        it("test minmax", [&]() {
            using pack_t = pack<int32_t, 4>;

            std::vector<pack_t> values{ { 50, 40, 30, 20 },
                                        { 10, 0, 60, 55 },
                                        { 88, 22, 44, 96 },
                                        { 24, 6, 4, 3 } };

            std::vector<pack_t> swapped_values{ { 10, 0, 30, 20 },
                                                { 50, 40, 60, 55 },
                                                { 24, 6, 4, 3 },
                                                { 88, 22, 44, 96 } };

            tie(values[0], values[1])
                = floki::detail::minmax(values[0], values[1]);
            tie(values[2], values[3])
                = floki::detail::minmax(values[2], values[3]);

            AssertThat(values, EqualsContainer(swapped_values,
                                               are_packs_equal<pack_t>));
        });

        it("test bitonic sort", [&]() {

            using pack_t = pack<int32_t, 4>;

            std::vector<pack_t> values{ { 50, 40, 30, 20 },
                                        { 10, 0, 60, 55 },
                                        { 88, 22, 44, 96 },
                                        { 24, 6, 4, 3 } };

            std::vector<pack_t> sorted_values{ { 0, 3, 4, 6 },
                                               { 10, 20, 22, 24 },
                                               { 30, 40, 44, 50 },
                                               { 55, 60, 88, 96 } };

            tie(values[0], values[1], values[2], values[3])
                = floki::detail::bitonic_sort_16(values[0], values[1],
                                                 values[2], values[3]);

            AssertThat(values,
                       EqualsContainer(sorted_values, are_packs_equal<pack_t>));
        });

        it("test merge sorted vectors", [&]() {
            using pack_t = pack<int32_t, 4>;

            std::vector<pack_t> values{ { 7, 12, 32, 66 }, { 1, 22, 23, 76 } };
            std::vector
                <pack_t> sorted_values{ { 1, 7, 12, 22 }, { 23, 32, 66, 76 } };

            tie(values[0], values[1])
                = floki::detail::merge_sorted_vectors(values[0], values[1]);

            AssertThat(values,
                       EqualsContainer(sorted_values, are_packs_equal<pack_t>));
        });

        it("test merge sort", [&]() {

            std::vector<int32_t> values
                = { 0,    4,    6,    20,   40,   60,   90,   155,
                    1188, 2002, 2244, 2296, 3124, 3226, 3334, 4443,
                    10,   24,   46,   120,  140,  260,  390,  455,
                    2188, 3002, 4244, 5296, 6124, 6226, 6334, 6443, };

            auto vo = output_begin<4>(begin(values));
            auto va = input_begin<4>(begin(values));
            auto vb = input_begin<4>(begin(values) + 16);

            auto sorted_values = values;
            std::sort(begin(sorted_values), end(sorted_values));

            floki::detail::merge_sort(va, vb, vo, 4, 4);

            AssertThat(values, EqualsContainer(sorted_values));
        });

        it("test merge sort uneven", [&]() {

            std::vector<int32_t> values
                = { 0,    4,    6,    20,   40,   60,   90,   155,
                    1188, 2002, 2244, 2296, 3124, 3226, 3334, 4443,
                    11188, 12002, 12244, 12296, 13124, 13226, 13334, 14443,
                    21188, 22002, 22244, 22296, 23124, 23226, 23334, 24443,
                    10,   24,   46,   120,  140,  260,  390,  455,
                    2188, 3002, 4244, 5296, 6124, 6226, 6334, 6443, };

            auto sorted_values = values;
            std::sort(begin(sorted_values), end(sorted_values));

            auto va = input_begin<4>(begin(values));
            auto vb = input_begin<4>(begin(values) + 32);

            auto output_values = values;
            auto vo = output_begin<4>(begin(output_values));

            floki::detail::merge_sort(va, vb, vo, 8, 4);

            AssertThat(output_values, EqualsContainer(sorted_values));
        });

        it("test sort reverse sorted array", [&]() {
            const size_t elements = 8 * 2048;
            int n(elements);
            std::vector<uint32_t> values(elements);

            std::generate(begin(values), end(values), [&] { return n--; });

            auto sorted_values = values;

            std::sort(begin(sorted_values), end(sorted_values));

            floki::sort(begin(values), end(values));

            AssertThat(values, EqualsContainer(sorted_values));
        });

        it("test sort sorted array", [&]() {
            const size_t elements = 8 * 2048;
            int n(0);
            std::vector<uint32_t> values(elements);

            std::generate(begin(values), end(values), [&] { return n++; });

            auto sorted_values = values;

            std::sort(begin(sorted_values), end(sorted_values));

            floki::sort(begin(values), end(values));

            AssertThat(values, EqualsContainer(sorted_values));
        });

        it("test sort random int32_t 112",
           [&]() { random_test<int32_t>(112); });

        it("test sort random int32_t 96",
           [&]() { random_test<int32_t>(96); });

        it("test sort random int32_t 97",
           [&]() { random_test<int32_t>(97); });

        it("test sort random int32_t 111",
           [&]() { random_test<int32_t>(111); });

        it("test sort random int32_t 144",
           [&]() { random_test<int32_t>(144); });

        it("test sort random int32_t 176",
           [&]() { random_test<int32_t>(176); });

        it("test sort random int32_t 128",
           [&]() { random_test<int32_t>(256); });

        it("test sort random int32_t", [&]() { random_test<int32_t>(); });

        it("test sort random uint32_t", [&]() { random_test<int32_t>(); });

        it("test sort random float", [&]() { random_test<float>(); });

        it("test sort random double", [&]() { random_test<double>(); });
    });
});
int main(int argc, char *argv[])
{
    return bandit::run(argc, argv);
}
