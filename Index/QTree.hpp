#ifndef QTREE_HPP
#define QTREE_HPP

#include <memory>

#include "Index.hpp"
#include "SimpleQuery.hpp"

class IQTreeElem {
public:
    virtual int getNext() = 0;
    virtual ~IQTreeElem() = default;
};

enum class QNode_t {AND, OR};

class QNode: public IQTreeElem {
public:
    QNode(QNode_t node_type, std::unique_ptr<IQTreeElem> &&left, std::unique_ptr<IQTreeElem> &&right):
        node_type_(node_type), left_(std::move(left)), right_(std::move(right)) {}
    int getNext() override;
    QNode(QNode &&other);
    ~QNode() = default;

private:
    QNode_t node_type_;
    std::unique_ptr<IQTreeElem> left_, right_;
    int left_val_, right_val_;
};

class SimpleQLeaf: public IQTreeElem {
public:
    SimpleQLeaf(const std::vector<int> *posting_list);
    int getNext() override ;
    SimpleQLeaf(SimpleQLeaf&& other);
    ~SimpleQLeaf() = default;

private:
    const std::vector<int> *posting_list_;
    std::vector<int>::const_iterator iter_;
};

class CompressQLeaf: public IQTreeElem {
public:
    CompressQLeaf(const std::vector<char> *posting_list);
    int getNext() override;
    CompressQLeaf(CompressQLeaf &&other);
    ~CompressQLeaf() = default;

private:
    const std::vector<char> *posting_list_;
    int pred_;
    size_t bit_num_;
};

class QTree {
public:
    QTree(const simple_index_vec &dict, const std::wstring &query);
    QTree(const compressed_index &dict, const std::wstring &query);
    int getNext() { return head_->getNext(); }

private:
    std::unique_ptr<IQTreeElem> head_;
};

template<class IndexType>
std::vector<int> streaming_query_processing(const IndexType &dict, const std::wstring &query) {
    QTree tree(dict, query);
    int doc_id = tree.getNext();
    std::vector<int> ret;
    while(doc_id != -1) {
        ret.push_back(doc_id);
        doc_id = tree.getNext();
    }
    return ret;
}

#endif