

find_path(BANDIT_DIR bandit/bandit.h HINTS bandit/)

if (BANDIT_DIR)

include_directories(${BANDIT_DIR})

add_executable(test_aa_sort test_aa_sort.cpp ../floki/aa_sort.hpp ../floki/detail/aa_sort.hpp)
add_executable(test_kary test_kary.cpp ../floki/btree.hpp ../floki/kary_search.hpp)
add_executable(test_find_if test_find_if.cpp)

enable_testing()
add_test(NAME aa_sort COMMAND test_aa_sort)
add_test(NAME kary COMMAND test_kary)
add_test(NAME find_if COMMAND test_find_if)
endif(BANDIT_DIR)
 





