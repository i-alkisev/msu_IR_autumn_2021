#include <algorithm>

#include "QTree.hpp"

int QNode::getNext() {
    if (node_type_ == QNode_t::AND) {
        left_val_ = left_->getNext();
        right_val_ = right_->getNext();
        while (left_val_ != -1 && right_val_ != -1 && left_val_ != right_val_){
            if (left_val_ < right_val_) {
                left_val_ = left_->getNext();
            }
            else {
                right_val_ = right_->getNext();
            }
        }
        if (left_val_ == -1 || right_val_ == -1) return -1;
        return left_val_;
    }
    else {
        if (left_val_ != -1 && (left_val_ < right_val_ || right_val_ == -1)) {
            left_val_ = left_->getNext();
        }
        else if (right_val_ != -1 && (right_val_ < left_val_ || left_val_ == -1)) {
            right_val_ = right_->getNext();
        }
        else if (left_val_ != -1 && right_val_ != -1){
            left_val_ = left_->getNext();
            right_val_ = right_->getNext();
        }

        if (left_val_ != -1 && right_val_ != -1) {
            return std::min(left_val_, right_val_);
        }
        if (left_val_ != -1) {
            return left_val_;
        }
        return right_val_;
    }
}

QNode::QNode(QNode &&other) {
    node_type_ = other.node_type_;
    left_ = std::move(other.left_);
    right_ = std::move(other.right_);
    left_val_ = other.left_val_;
    right_val_ = other.right_val_;
}

SimpleQLeaf::SimpleQLeaf(const std::set<int> *posting_list): posting_list_(posting_list) {
    if (posting_list_) {
        iter_ = posting_list_->begin();
    }
}

int SimpleQLeaf::getNext() {
    int ret = -1;
    if (posting_list_ && iter_ != posting_list_->end()) {
        ret = *iter_;
        ++iter_;
    }
    return ret;
}

CompressQLeaf::CompressQLeaf(const std::vector<char> *posting_list): posting_list_(posting_list), pred_(-1), bit_num_(0) {
    if (posting_list_) {
        pred_ = 0;
    }
}

int CompressQLeaf::getNext() {
    if (pred_ == -1) return -1;

    size_t bit_num = bit_num_ % 8;
    size_t byte_num = bit_num_ / 8;

    unsigned char mask = 1 << 7;
    while (byte_num < posting_list_->size()) {
        size_t len = 1;
        while ((byte_num < posting_list_->size()) && ((mask >> bit_num) & ~(*posting_list_)[byte_num])) {
            ++len;
            ++bit_num;
            if (bit_num == 8) {
                ++byte_num;
                bit_num = 0;
            }
        }
        int decompressed = 0;
        size_t i = 0;
        for (; i < len && byte_num < posting_list_->size(); ++i) {
            decompressed <<= 1;
            if ((mask >> bit_num) & (*posting_list_)[byte_num]) {
                decompressed |= 1;
            }
            ++bit_num;
            if (bit_num == 8) {
                ++byte_num;
                bit_num = 0;
            }
        }
        bit_num_ = byte_num * 8 + bit_num;
        if (i == len) {
            pred_ += decompressed; 
            return pred_;
        }
        pred_ = -1;
        return -1;
    }
    pred_ = -1;
    return -1;
}

CompressQLeaf::CompressQLeaf(CompressQLeaf &&other) {
    posting_list_ = std::move(other.posting_list_);
    bit_num_ = other.bit_num_;
}

QTree::QTree(const simple_index &dict, const std::wstring &query) {
    std::vector<std::wstring> rpn = reverse_polish_notation(query);
    std::vector<std::unique_ptr<IQTreeElem>> stack;
    for (size_t i = 0; i < rpn.size(); ++i) {
        if (rpn[i] == L"|") {
            if (stack.size() < 2) throw std::runtime_error("Invalid expression");

            std::unique_ptr<IQTreeElem> left = std::move(stack.back());
            stack.pop_back();
            std::unique_ptr<IQTreeElem> right = std::move(stack.back());
            stack.pop_back();
            stack.emplace_back(new QNode(QNode_t::OR, std::move(left), std::move(right)));
        }
        else if (rpn[i] == L"&") {
            if (stack.size() < 2) throw std::runtime_error("Invalid expression");

            std::unique_ptr<IQTreeElem> left = std::move(stack.back());
            stack.pop_back();
            std::unique_ptr<IQTreeElem> right = std::move(stack.back());
            stack.pop_back();
            stack.emplace_back(new QNode(QNode_t::AND, std::move(left), std::move(right)));
        }
        else {
            const std::set<int> *ptr = nullptr;
            auto it = dict.find(rpn[i]);
            if (it != dict.end()) {
                ptr = &(it->second);
            }
            stack.emplace_back(new SimpleQLeaf(ptr));
        }
    }
    if (stack.size() != 1) throw std::runtime_error("Invalid expression");
    head_ = std::move(stack.back());
}

QTree::QTree(const compressed_index &dict, const std::wstring &query) {
    std::vector<std::wstring> rpn = reverse_polish_notation(query);
    std::vector<std::unique_ptr<IQTreeElem>> stack;
    for (size_t i = 0; i < rpn.size(); ++i) {
        if (rpn[i] == L"|") {
            if (stack.size() < 2) throw std::runtime_error("Invalid expression");

            std::unique_ptr<IQTreeElem> left = std::move(stack.back());
            stack.pop_back();
            std::unique_ptr<IQTreeElem> right = std::move(stack.back());
            stack.pop_back();
            stack.emplace_back(new QNode(QNode_t::OR, std::move(left), std::move(right)));
        }
        else if (rpn[i] == L"&") {
            if (stack.size() < 2) throw std::runtime_error("Invalid expression");

            std::unique_ptr<IQTreeElem> left = std::move(stack.back());
            stack.pop_back();
            std::unique_ptr<IQTreeElem> right = std::move(stack.back());
            stack.pop_back();
            stack.emplace_back(new QNode(QNode_t::AND, std::move(left), std::move(right)));
        }
        else {
            const std::vector<char> *ptr = nullptr;
            auto it = dict.find(rpn[i]);
            if (it != dict.end()) {
                ptr = &(it->second);
            }
            stack.emplace_back(new CompressQLeaf(ptr));
        }
    }
    if (stack.size() != 1) throw std::runtime_error("Invalid expression");
    head_ = std::move(stack.back());
}
