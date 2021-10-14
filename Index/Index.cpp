#include <locale>
#include <codecvt>
#include <regex>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <stdexcept>

#include "Index.hpp"

void lower(std::wstring &str) {
    for (wchar_t &ch : str) {
        if (ch <= L'Я' && ch >= L'А') {
            ch = L'а' + ch - L'А';
        }
        else if (ch <= L'Z' && ch >= L'A') {
            ch = L'a' + ch - L'A';
        }
    }
}

std::tuple<std::string, std::wstring> get_doc(char *data_ptr, int data_len) {
    char url_len = *data_ptr;
    data_ptr += 1;

    std::string url(data_ptr, url_len);
    data_ptr += url_len + 3;

    int content_len = data_len - 4 - url_len;
    std::wstring content;
    if (content_len > 0) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
        content = converter.from_bytes(data_ptr, data_ptr + content_len);
    }
    return std::make_tuple(url, content);
}

void update_index(simple_index &dict, int doc_id, std::wstring &content) {
    lower(content);
    std::wregex words_regex(L"[а-яА-ЯёЁ\\w]+");
    auto words_begin = std::wsregex_iterator(content.begin(), content.end(), words_regex);
    auto words_end = std::wsregex_iterator();
    for (auto word_it = words_begin; word_it != words_end; ++word_it) {
        dict[word_it->str()].insert(doc_id);
    }
}

void file_processing(simple_index &dict, id2url &urls, const char *file_name, int &bad_docs) {
    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        throw std::runtime_error("Error open file \"" + std::string(file_name) + "\"");
    }
    long file_len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    std::unique_ptr<char []> file(new char[file_len]);
    if (read(fd, file.get(), file_len) < file_len) {
        close(fd);
        throw std::runtime_error("Error read file\"" + std::string(file_name) + "\"");
    }
    close(fd);

    char *ptr = file.get();
    while (ptr < file.get() + file_len) {
        int data_len = *(reinterpret_cast<int *>(ptr));
        ptr += 5;
        try {
            std::tuple<std::string, std::wstring> doc = get_doc(ptr, data_len - 1);

            std::string url = std::get<0>(doc);
            urls.push_back(url);

            int doc_id = urls.size();

            std::wstring content = std::get<1>(doc);
            update_index(dict, doc_id, content);

        } catch(std::exception &e) {
            ++bad_docs;
        }
        ptr += data_len - 1;
    }
}

size_t bit_len(int a) {
    size_t len = 0;
    while(a > 0) {
        ++len;
        a >>= 1;
    }
    return len;
}

std::vector<char> compress(const std::vector<int> &ints) {

    size_t bits_per_block = 8;

    std::vector<int> diffs(ints.size());
    std::vector<size_t> lens(ints.size());
    diffs[0] = ints[0];
    size_t count_bits = lens[0] = bit_len(ints[0]);
    for (size_t i = 1; i < ints.size(); ++i) {
        diffs[i] = ints[i] - ints[i - 1];
        lens[i] = bit_len(diffs[i]);
        count_bits += lens[i];
    }
    count_bits += count_bits - ints.size();
    int count_bytes = count_bits / bits_per_block;
    if (count_bits % bits_per_block) ++count_bytes;
    std::vector<char> ret(count_bytes, 0);
    size_t bit_num = 0, byte_num = 0;
    unsigned char mask = 1 << (bits_per_block - 1);
    for (size_t i = 0; i < diffs.size(); ++i) {
        bit_num += lens[i] - 1;
        byte_num += bit_num / bits_per_block;
        bit_num %= bits_per_block;
        for (size_t j = 0; j < lens[i]; ++j) {
            if ((diffs[i] >> (lens[i] - j - 1)) & 1) {
                ret[byte_num] |= (mask >> bit_num);
            }
            ++bit_num;
            if (bit_num == bits_per_block) {
                ++byte_num;
                bit_num = 0;
            }
        }
    }
    return ret;
}

compressed_index compress_index(const simple_index &dict) {
    compressed_index compressed_dict;
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        compressed_dict[it->first] = compress(std::vector<int>{it->second.begin(), it->second.end()});
    }
    return compressed_dict;
}

simple_index_vec vectorize_index(const simple_index &dict) {
    simple_index_vec vec_dict;
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        vec_dict[it->first] = std::vector<int>{it->second.begin(), it->second.end()};
    }
    return vec_dict;
}