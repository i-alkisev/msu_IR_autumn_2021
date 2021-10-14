#include <regex>

#include "SimpleQuery.hpp"

std::vector<std::wstring> reverse_polish_notation(const std::wstring &query) {
    std::vector<std::wstring> ret;
    std::vector<std::wstring> stack;

    std::wregex words_regex(L"[\\S]+");
    auto words_begin = std::wsregex_iterator(query.begin(), query.end(), words_regex);
    auto words_end = std::wsregex_iterator();
    for (auto word_it = words_begin; word_it != words_end; ++word_it) {
        std::wstring s = word_it->str();
        if (s == L"&") {
            while(!stack.empty() && stack.back() == L"&") {
                ret.push_back(stack.back());
                stack.pop_back();
            }
            stack.push_back(s);
        }
        else if (s == L"|") {
            while(!stack.empty() && (stack.back() == L"&" || stack.back() == L"|")) {
                ret.push_back(stack.back());
                stack.pop_back();
            }
            stack.push_back(s);
        }
        else if (s == L"(") {
            stack.push_back(s);
        }
        else if (s == L")") {
            while(!stack.empty() && stack.back() != L"(") {
                ret.push_back(stack.back());
                stack.pop_back();
            }
            if (stack.empty()) throw std::runtime_error("Invalid expression (incorrect brackets)");
            stack.pop_back();
        }
        else {
            lower(s);
            ret.push_back(s);
        }
    }
    while (!stack.empty()) {
        ret.push_back(stack.back());
        stack.pop_back();
    }
    return ret;
}

std::vector<int> simple_query_processing(const simple_index &dict, const std::wstring &query) {
    std::vector<std::set<int>> stack;
    std::vector<std::wstring> rpn = reverse_polish_notation(query);
    for (auto &s: rpn) {
        if (s == L"&") {
            if (stack.size() < 2) throw std::runtime_error("Invalid expression (\'&\' nedeed 2 operands)");
            std::set<int> &first_pl = stack.back();
            std::set<int> &second_pl = stack[stack.size() - 2];
            std::vector<int> tmp_res;
            std::set_intersection(first_pl.begin(), first_pl.end(),
                                  second_pl.begin(), second_pl.end(),                  
                                  std::back_inserter(tmp_res));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(std::set<int>{tmp_res.begin(), tmp_res.end()});
        }
        else if (s == L"|") {
            if (stack.size() < 2) throw std::runtime_error("Invalid expression (\'|\' nedeed 2 operands)");
            std::set<int> &first_pl = stack.back();
            std::set<int> &second_pl = stack[stack.size() - 2];
            std::vector<int> tmp_res;
            std::set_union(first_pl.begin(), first_pl.end(),
                           second_pl.begin(), second_pl.end(),                  
                           std::back_inserter(tmp_res));
            stack.pop_back();
            stack.pop_back();
            stack.push_back(std::set<int>{tmp_res.begin(), tmp_res.end()});
        }
        else {
            if (dict.find(s) != dict.end()) {
                stack.push_back(dict.at(s));
            }
            else {
                return std::vector<int>();
            }
        }
    }
    if (stack.size() != 1) throw std::runtime_error("Invalid expression");
    return std::vector<int>{stack.back().begin(), stack.back().end()};
}