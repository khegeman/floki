

find_path(BANDIT_DIR bandit/bandit.h HINTS bandit/)

if (BANDIT_DIR)

include_directories(${BANDIT_DIR})

add_executable(test_aa_sort test_aa_sort.cpp ../floki/aa_sort.hpp ../floki/detail/aa_sort.hpp)

enable_testing()
add_test(NAME aa_sort COMMAND test_aa_sort)
endif(BANDIT_DIR)
 





