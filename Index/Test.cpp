#include <iostream>
#include <cassert>
#include <chrono>

#include "Index.hpp"
#include "SimpleQuery.hpp"
#include "QTree.hpp"
#include "Test.hpp"

int64_t mean(const std::vector<int64_t> v) {
    int64_t m = 0;
    for (int64_t val: v) {
        m += val;
    }
    return m / v.size();
}

void run_test(int count_files, const char *file_names[]) {
    std::vector<std::wstring> test_queries = {
        L"рыба",
        L"путин & медведев & ( новость | новости | news )",
        L"а & в & и & но & как",
        L"( да | нет | не ) & ( ( может | хочет | будет ) | ( никто & кто ) ) & ( а | я | как | и )",
        L"и | а | я | мы | ты | вы | в | на | около",
        L"носок & краснобуромалиновый",
    };

    std::cout << "Creating index..." << std::endl;
    std::chrono::time_point<std::chrono::steady_clock> create_start = std::chrono::steady_clock::now();

    simple_index dict;
    id2url urls;
    int bad_docs = 0;

    for (int i = 0; i < count_files; ++i) {
        std::cout << "processing \"" + std::string(file_names[i]) +"\"" << std::endl;
        file_processing(dict, urls, file_names[i], bad_docs);
    }
    std::cout << std::endl;

    std::chrono::time_point<std::chrono::steady_clock> create_end = std::chrono::steady_clock::now();
    std::cout << "Compression..." << std::endl << std::endl;

    compressed_index cdict = compress_index(dict);

    std::chrono::time_point<std::chrono::steady_clock> compress_end = std::chrono::steady_clock::now();

    std::cout << "Index info:" << std::endl;

    auto elapsed4create = std::chrono::duration_cast<std::chrono::milliseconds>(create_end - create_start);
    auto elapsed4compress = std::chrono::duration_cast<std::chrono::milliseconds>(compress_end - create_end);

    int64_t sec = elapsed4create.count() / 1000;
    int64_t msec = elapsed4create.count() - 1000 * sec;
    std::cout << "create_time:    " << sec << "s " << msec << "ms" << std::endl;
    sec = elapsed4compress.count() / 1000;
    msec = elapsed4compress.count() - 1000 * sec;
    std::cout << "compress_time:  " << sec << "s " << msec << "ms" << std::endl;
    std::cout << "count_docs:     " << urls.size() << std::endl;
    std::cout << "bad_docs:       " << bad_docs << std::endl;
    std::cout << "count_words:    " << dict.size() << std::endl;

    size_t memory_usage = 0;
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        memory_usage += it->second.size() * sizeof(int);
    }
    std::cout << "\nBefore compression\n memory_usage: " << memory_usage / 1024 << " Kb" << std::endl;

    memory_usage = 0;
    for (auto it = cdict.begin(); it != cdict.end(); ++it) {
        memory_usage += it->second.size() * sizeof(char);
    }
    std::cout << "\nAfter Compression\n memory_usage: " << memory_usage / 1024 << " Kb" << std::endl;

    std::cout << "\nTest results:" << std::endl;
    
    std::vector<std::vector<int64_t>> qtimes(3, std::vector<int64_t>{});
    std::chrono::time_point<std::chrono::steady_clock> start, end;
    std::chrono::microseconds elapsed;
    std::cout << "\t\tSimpleIdx\t\tSimpleIdx+QTree\t\tCompressedIdx+QTree" << std::endl;
    for (size_t i = 0; i < test_queries.size(); ++i) {
        auto query = test_queries[i];

        std::cout << "Test " << (i + 1) << ": ";

        start = std::chrono::steady_clock::now();
        std::vector<int> ans1 = simple_query_processing(dict, query);
        end = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        qtimes[0].push_back(elapsed.count());

        start = std::chrono::steady_clock::now();
        std::vector<int> ans2 = streaming_query_processing(dict, query);
        end = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        qtimes[1].push_back(elapsed.count());

        start = std::chrono::steady_clock::now();
        std::vector<int> ans3 = streaming_query_processing(cdict, query);
        end = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        qtimes[2].push_back(elapsed.count());

        assert(ans1.size() == ans2.size() && ans1.size() == ans3.size());
        for (size_t i = 0; i < ans1.size(); ++i) {
            assert(ans1[i] == ans2[i] && ans1[i] == ans3[i]);
        }

        std::cout << "\t" << qtimes[0].back() << " microsec\t\t" << qtimes[1].back() << " microsec\t\t" << qtimes[2].back() << " microsec\t\t";
        std::cout << "Found " << ans1.size() << " pages" << std::endl;
    }
    std::cout << std::string(110, '_') << std::endl;
    std::cout << "Average: \t" << mean(qtimes[0]) << " microsec\t\t" << mean(qtimes[1]) << " microsec\t\t" << mean(qtimes[2]) << " microsec" << std::endl;
}