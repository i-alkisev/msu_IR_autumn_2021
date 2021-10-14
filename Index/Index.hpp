#ifndef INDEX_HPP
#define INDEX_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <set>

using simple_index = std::unordered_map<std::wstring, std::set<int>>;
using compressed_index = std::unordered_map<std::wstring, std::vector<char>>;
using id2url = std::vector<std::string>;

void lower(std::wstring &str);

std::tuple<std::string, std::wstring> get_doc(char *data_ptr, int data_len);

void update_index(simple_index &dict, int doc_id, std::wstring &content);

void file_processing(simple_index &dict, id2url &urls, const char *file_name, int &bad_docs);

size_t bit_len(int a);

std::vector<char> compress(const std::vector<int> &ints);

compressed_index compress_index(const simple_index &dict);

#endif