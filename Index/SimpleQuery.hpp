#ifndef SIMPLE_QUERY
#define SIMPLE_QUERY

#include "Index.hpp"

std::vector<std::wstring> reverse_polish_notation(const std::wstring &query);

std::vector<int> simple_query_processing(const simple_index &dict, const std::wstring &query);

#endif