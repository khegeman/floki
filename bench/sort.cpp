
#include <limits>
#include <vector>
#include <chrono>
#include <iostream>
#include <random>
#include <functional>

#ifdef SIMD_BENCH
#include <floki/aa_sort.hpp>
#else
#include <algorithm>
#endif

using namespace std::chrono;

template <typename T> void random_test(size_t elements,size_t iteratations, const char* description)
{
    std::vector<T> values(elements);
    typedef typename std::conditional
        <std::is_integral<T>::value, typename std::uniform_int_distribution<T>,
         typename std::uniform_real_distribution<T>>::type distribution_t;
    distribution_t distribution;
    std::mt19937 engine;
    auto generator = std::bind(distribution, engine);
    std::generate_n(begin(values), elements, generator);

    double total = 0;

    std::cout << "starting benchmark sorting " << elements << " " << description << "'s for " << iteratations << " iterations. " << std::endl;


    for (int i = 0; i < iteratations; ++i)
    {
        std::random_shuffle(values.begin(),values.end());
        auto start = system_clock::now();
#ifdef SIMD_BENCH
        floki::sort(values.begin(), values.end());
#else
        std::sort(values.begin(), values.end());
#endif

        auto end = system_clock::now();
        total += (duration_cast<duration<float, std::milli>>(end - start)).count();
    }
    std::cout << "Sorted " << elements << " " << iteratations << " times in " << total
              << " ms. mean " << total / iteratations <<  "ms. first value " << values[0]  <<  std::endl;
}

int main(int argc, char **argv)
{
    size_t elements = 65536;
    size_t iterations = 1;
    uint32_t mode = 0;

    if (argc > 1)
        elements = atoi(argv[1]);
    if (argc > 2)
        iterations = atoi(argv[2]);
    if (argc > 3)
        mode = atoi(argv[3]);

    switch (mode) {
    case 1:
        random_test<float>(elements,iterations,"float");
        break;
    case 2:
        random_test<double>(elements,iterations,"double");
        break;
    default:
        random_test<int32_t>(elements,iterations,"int32_t");
    }

    return 0;
}
