#include <iostream>

struct Node {
    Node* left = nullptr;
    Node* right = nullptr;
    Node () {}
    Node (int n) {
        if (n == 0) {
            return;
        }
        right = new Node(n-1);
        right->left = this;
    }
    ~Node () {
        delete right;
        left->right = nullptr;
    }
};

int main () {   
    Node* x = new Node(100);
    Node* x1 = x->right;
    Node* x2 = x1->right;
    std::cout << x << std::endl;
    std::cout << x1 << std::endl;
    std::cout << x2 << std::endl<< std::endl;
    delete x2->right; 
    std::cout << x << std::endl;
    std::cout << x1 << std::endl;
    std::cout << x2 << std::endl<< std::endl;
    delete x2->right;

    return 0;
    std::cout << x1 << std::endl;
    std::cout << x2 << std::endl;

}