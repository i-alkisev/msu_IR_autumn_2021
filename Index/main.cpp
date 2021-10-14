#include <iostream>
#include <vector>
#include <chrono>
#include <locale>
#include <codecvt>

#include "Index.hpp"
#include "SimpleQuery.hpp"
#include "QTree.hpp"
#include "Test.hpp"

template<class IndexType>
void run(const IndexType &dict, const id2url &urls, std::vector<int> (*q_processing)(const IndexType&, const std::wstring&)) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    std::wstring query;
    std::string query_src;

    std::cout << "Query > ";
    std::getline(std::cin, query_src);
    query = converter.from_bytes(query_src);

    while (query != L"exit()") {
        std::vector<int> ans = q_processing(dict, query);

        std::cout << "Found " << ans.size() << " pages" << std::endl;
        for (size_t i = 0; i < ans.size(); ++i){
            std::cout << i + 1 << ") " << urls[ans[i]] << std::endl;
        }

        std::cout << "Query > ";
        std::getline(std::cin, query_src);
        query = converter.from_bytes(query_src);
    }
}

void run_simpleidx(int count_files, const char *file_names[]) {
    std::cout << "Creating index..." << std::endl;
    std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

    simple_index dict;
    id2url urls;
    int bad_docs = 0;

    for (int i = 0; i < count_files; ++i) {
        std::cout << "processing \"" + std::string(file_names[i]) +"\"" << std::endl;
        file_processing(dict, urls, file_names[i], bad_docs);
    }

    std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    int64_t sec = elapsed.count() / 1000;
    int64_t msec = elapsed.count() - 1000 * sec;

    size_t memory_usage = 0;
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        memory_usage += it->second.size() * sizeof(int);
    }

    std::cout << "create_time:  " << sec << "s " << msec << "ms" << std::endl;
    std::cout << "memory_usage: " << memory_usage / 1024 << " Kb" << std::endl;
    std::cout << "count_docs:   " << urls.size() << std::endl;
    std::cout << "count_words:  " << dict.size() << std::endl;
    std::cout << "bad_docs:     " << bad_docs << std::endl;

    run(dict, urls, simple_query_processing);
}

void run_simpleidx_qtree(int count_files, const char *file_names[]) {
    std::cout << "Creating index..." << std::endl;
    std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

    simple_index dict;
    id2url urls;
    int bad_docs = 0;

    for (int i = 0; i < count_files; ++i) {
        std::cout << "processing \"" + std::string(file_names[i]) +"\"" << std::endl;
        file_processing(dict, urls, file_names[i], bad_docs);
    }

    std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    int64_t sec = elapsed.count() / 1000;
    int64_t msec = elapsed.count() - 1000 * sec;

    size_t memory_usage = 0;
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        memory_usage += it->second.size() * sizeof(int);
    }

    std::cout << "create_time:  " << sec << "s " << msec << "ms" << std::endl;
    std::cout << "memory_usage: " << memory_usage / 1024 << " Kb" << std::endl;
    std::cout << "count_docs:   " << urls.size() << std::endl;
    std::cout << "count_words:  " << dict.size() << std::endl;
    std::cout << "bad_docs:     " << bad_docs << std::endl;

    run(dict, urls, streaming_query_processing);
}

void run_compressidx_qtree(int count_files, const char *file_names[]) {
    std::cout << "Creating index..." << std::endl;
    std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

    simple_index dict;
    id2url urls;
    int bad_docs = 0;

    for (int i = 0; i < count_files; ++i) {
        std::cout << "processing \"" + std::string(file_names[i]) +"\"" << std::endl;
        file_processing(dict, urls, file_names[i], bad_docs);
    }

    std::chrono::time_point<std::chrono::steady_clock> create_end = std::chrono::steady_clock::now();

    compressed_index cdict = compress_index(dict);

    std::chrono::time_point<std::chrono::steady_clock> compress_end = std::chrono::steady_clock::now();

    auto elapsed4create = std::chrono::duration_cast<std::chrono::milliseconds>(create_end - start);
    auto elapsed4compress = std::chrono::duration_cast<std::chrono::milliseconds>(compress_end - create_end);

    int64_t sec = elapsed4create.count() / 1000;
    int64_t msec = elapsed4create.count() - 1000 * sec;
    std::cout << "create_time:   " << sec << "s " << msec << "ms" << std::endl;
    sec = elapsed4compress.count() / 1000;
    msec = elapsed4compress.count() - 1000 * sec;
    std::cout << "compress_time: " << sec << "s " << msec << "ms" << std::endl;

    size_t memory_usage = 0;
    for (auto it = cdict.begin(); it != cdict.end(); ++it) {
        memory_usage += it->second.size() * sizeof(char);
    }
    std::cout << "memory_usage:  " << memory_usage / 1024 << " Kb" << std::endl;
    std::cout << "count_docs:    " << urls.size() << std::endl;
    std::cout << "count_words:   " << dict.size() << std::endl;
    std::cout << "bad_docs:      " << bad_docs << std::endl;

    run(cdict, urls, streaming_query_processing);
}

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        std::cout << "Not enough arguments" << std::endl;
        return 0;
    }
    if (argv[1] == std::string("--simpleidx")) {
        run_simpleidx(argc - 2, argv + 2);
        return 0;
    }
    if (argv[1] == std::string("--simpleidx_qtree")) {
        run_simpleidx_qtree(argc - 2, argv + 2);
        return 0;
    }
    if (argv[1] == std::string("--compressidx_qtree")) {
        run_compressidx_qtree(argc - 2, argv + 2);
        return 0;
    }
    if (argv[1] == std::string("--test")) {
        run_test(argc - 2, argv + 2);
        return 0;
    }
    std::cout << "Invalid argument: \"" << std::string(argv[1]) << "\"" << std::endl;
    return 0;
}
