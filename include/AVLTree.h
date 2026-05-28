#pragma once
#include "Types.h"
#include <memory>
#include <vector>
#include <functional>

struct Node {
    WordRecord data;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;
    int height;
    
    Node(WordRecord val) : data(val), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
public:
    void insert(const WordRecord& word);
    void remove(const std::string& spanish_word);
    WordRecord* search(const std::string& spanish_word);
    void inOrder(std::function<void(const WordRecord&)> callback) const;
    std::vector<WordRecord> getAll() const;
    void clear();

private:
    std::shared_ptr<Node> root = nullptr;

    int height(std::shared_ptr<Node> n) const;
    int getBalance(std::shared_ptr<Node> n) const;
    std::shared_ptr<Node> rightRotate(std::shared_ptr<Node> y);
    std::shared_ptr<Node> leftRotate(std::shared_ptr<Node> x);
    std::shared_ptr<Node> insert(std::shared_ptr<Node> node, const WordRecord& word);
    std::shared_ptr<Node> minValueNode(std::shared_ptr<Node> node);
    std::shared_ptr<Node> remove(std::shared_ptr<Node> root, const std::string& word);
    void inOrder(std::shared_ptr<Node> root, std::function<void(const WordRecord&)> callback) const;
    WordRecord* search(std::shared_ptr<Node> root, const std::string& word);
};
