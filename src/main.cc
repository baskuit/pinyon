#include <iostream>

#include "libsurskit/math.hh"
#include "state/toy_state.hh"
#include "tree/node.hh"


int main () {

    ToyState* state = new ToyState('u', 8, 0, .5f);
    MatrixNode* root = new MatrixNode();
    Action a0 = 0;
    Action a1 = 0;
    StateTransitionData data;
    ChanceNode* c0 = root->access(a0, a1);
    MatrixNode* m0 = c0->access(data);

    root->print(0);
    delete root;
    root->print(0);

    return 0;
    std::cout << root << std::endl;
    std::cout << (root->child) << std::endl;
    std::cout << (c0) << std::endl;
    std::cout << (c0->child) << std::endl;
    std::cout << (m0) << std::endl;
    delete root;
    std::cout << root << std::endl;
    std::cout << (root->child) << std::endl;
    std::cout << (c0) << std::endl;
    std::cout << (c0->child) << std::endl;
    std::cout << (m0) << std::endl;

    return 0;
}