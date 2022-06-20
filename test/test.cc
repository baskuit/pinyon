#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p.hh"

int main () {

    using State = ToyState<9>;
    using MonteCarlo = MonteCarlo<State>;
    using Exp3p = Exp3p<MonteCarlo>;

    prng device;

    MatrixNode<Exp3p> node;
    State::transition_data_t x;
    ChanceNode<Exp3p>* chance = node.access(0, 0);
    MatrixNode<Exp3p>* node_ = chance->access(x);
    auto node_1 = node.access(1, 1);
    auto node_1_ = node_1->access(x);

    std::cout << nullptr << std::endl;
    std::cout << &node << std::endl;
    std::cout << node.child << std::endl;
     std::cout << node.child->child << std::endl;
    std::cout << node.child->child->child << std::endl;

        std::cout << node_1 << std::endl;
            std::cout << node_1_ << std::endl;
            std::cout << chance->next << std::endl;
    return 0;
}