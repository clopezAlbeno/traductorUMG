#include "AVLTree.h"
#include <algorithm>

int AVLTree::height(std::shared_ptr<Node> n) const {
    if (n == nullptr) return 0;
    return n->height;
}

int AVLTree::getBalance(std::shared_ptr<Node> n) const {
    if (n == nullptr) return 0;
    return height(n->left) - height(n->right);
}

std::shared_ptr<Node> AVLTree::rightRotate(std::shared_ptr<Node> y) {
    std::shared_ptr<Node> x = y->left;
    std::shared_ptr<Node> T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = std::max(height(y->left), height(y->right)) + 1;
    x->height = std::max(height(x->left), height(x->right)) + 1;

    return x;
}

std::shared_ptr<Node> AVLTree::leftRotate(std::shared_ptr<Node> x) {
    std::shared_ptr<Node> y = x->right;
    std::shared_ptr<Node> T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = std::max(height(x->left), height(x->right)) + 1;
    y->height = std::max(height(y->left), height(y->right)) + 1;

    return y;
}

std::shared_ptr<Node> AVLTree::insert(std::shared_ptr<Node> node, const WordRecord& word) {
    if (node == nullptr)
        return std::make_shared<Node>(word);

    if (word < node->data)
        node->left = insert(node->left, word);
    else if (word > node->data)
        node->right = insert(node->right, word);
    else // Duplicate words are not allowed (or could update)
        return node;

    node->height = 1 + std::max(height(node->left), height(node->right));

    int balance = getBalance(node);

    // Left Left Case
    if (balance > 1 && word < node->left->data)
        return rightRotate(node);

    // Right Right Case
    if (balance < -1 && word > node->right->data)
        return leftRotate(node);

    // Left Right Case
    if (balance > 1 && word > node->left->data) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    // Right Left Case
    if (balance < -1 && word < node->right->data) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

void AVLTree::insert(const WordRecord& word) {
    root = insert(root, word);
}

std::shared_ptr<Node> AVLTree::minValueNode(std::shared_ptr<Node> node) {
    std::shared_ptr<Node> current = node;
    while (current->left != nullptr)
        current = current->left;
    return current;
}

std::shared_ptr<Node> AVLTree::remove(std::shared_ptr<Node> root, const std::string& word) {
    if (root == nullptr)
        return root;

    if (word < root->data.spanish)
        root->left = remove(root->left, word);
    else if (word > root->data.spanish)
        root->right = remove(root->right, word);
    else {
        if ((root->left == nullptr) || (root->right == nullptr)) {
            std::shared_ptr<Node> temp = root->left ? root->left : root->right;
            if (temp == nullptr) {
                temp = root;
                root = nullptr;
            } else
                *root = *temp; 
        } else {
            std::shared_ptr<Node> temp = minValueNode(root->right);
            root->data = temp->data;
            root->right = remove(root->right, temp->data.spanish);
        }
    }

    if (root == nullptr)
        return root;

    root->height = 1 + std::max(height(root->left), height(root->right));
    int balance = getBalance(root);

    if (balance > 1 && getBalance(root->left) >= 0)
        return rightRotate(root);

    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }

    if (balance < -1 && getBalance(root->right) <= 0)
        return leftRotate(root);

    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

void AVLTree::remove(const std::string& spanish_word) {
    root = remove(root, spanish_word);
}

WordRecord* AVLTree::search(std::shared_ptr<Node> node, const std::string& word) {
    if (node == nullptr)
        return nullptr;
    if (node->data.spanish == word)
        return &node->data;
    if (node->data.spanish > word)
        return search(node->left, word);
    return search(node->right, word);
}

WordRecord* AVLTree::search(const std::string& spanish_word) {
    return search(root, spanish_word);
}

void AVLTree::inOrder(std::shared_ptr<Node> root, std::function<void(const WordRecord&)> callback) const {
    if (root != nullptr) {
        inOrder(root->left, callback);
        callback(root->data);
        inOrder(root->right, callback);
    }
}

void AVLTree::inOrder(std::function<void(const WordRecord&)> callback) const {
    inOrder(root, callback);
}

std::vector<WordRecord> AVLTree::getAll() const {
    std::vector<WordRecord> result;
    inOrder([&result](const WordRecord& word) {
        result.push_back(word);
    });
    return result;
}

void AVLTree::clear() {
    root = nullptr;
}
