#ifndef TEST_HPP
#define TEST_HPP

#include <vector>
#include <cstdint>

int64_t mean(const std::vector<int64_t> v);

void run_test(int count_files, const char *file_names[]);

#endif