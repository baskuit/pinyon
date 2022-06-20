#include <assert.h>
#include <iostream>

#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/exp3p.hh"

template <typename Algorithm, typename Model>
void mean_strategies (Algorithm session, Model model, typename Algorithm::state_t state, int playouts, int rounds, 
                        std::array<double, Algorithm::state_t::size_> strategy0,
                        std::array<double, Algorithm::state_t::size_> strategy1) {


    std::array<double, Algorithm::state_t::size_> current_strategy0;
    std::array<double, Algorithm::state_t::size_> current_strategy1;
    
    for (int r = 0; r < rounds; ++r) {
            
        MatrixNode<Algorithm> root;
        root.stats.t = playouts;
        for (int p = 0; p < playouts; ++p) {
            typename Algorithm::state_t state_ = state;
            session.search(state_, model, &root);
        }


        math::power_norm<int, double, Algorithm::state_t::size_>
        (root.stats.visits0, root.pair.rows, 1, current_strategy0);
        math::power_norm<int, double, Algorithm::state_t::size_>
        (root.stats.visits1, root.pair.cols, 1, current_strategy1);

        for (int row_idx = 0; row_idx < root.pair.rows; ++row_idx) {
            strategy0[row_idx] += current_strategy0[row_idx];
        }
        for (int col_idx = 0; col_idx < root.pair.cols; ++col_idx) {
            strategy1[col_idx] += current_strategy1[col_idx];
        }

    }

    for (int row_idx = 0; row_idx < 2; ++row_idx) {
        strategy0[row_idx] /= (double) rounds;
    }
    for (int col_idx = 0; col_idx < 2; ++col_idx) {
        strategy1[col_idx] /= (double) rounds;
    }

    std::cout << "s0: ";
    for (int i = 0; i < 2; ++i) {
        std::cout << strategy0[i] << ' ';
    }
    std::cout << std::endl;

    std::cout << "s1: ";
    for (int i = 0; i < 2; ++i) {
        std::cout << strategy1[i] << ' ';
    }
    std::cout << std::endl;
}

int main () {

    using State = ToyState<9>;
    using MonteCarlo = MonteCarlo<State>;
    using MonteCarloWithPolicy = MonteCarloWithPolicy<State>;
    using Exp3pWithPolicy = Exp3p<MonteCarloWithPolicy>;
    using Exp3p = Exp3p<MonteCarlo>;
    prng device;
    prng device_;

    MonteCarlo model(device);
    MonteCarloWithPolicy model_(device_);
    
    Exp3p session(device);
    Exp3pWithPolicy session_(device_);  

    // Change State Parameters

    State toy(device, 'w', 5, 2);
        toy.transition(0, 0); // currently necessary since bad strategies on initialization.


    std::cout << "S0: " << toy.strategy0[0] << " " << toy.strategy0[1] << std::endl;
    std::cout << "S1: " << toy.strategy1[0] << " " << toy.strategy1[1] << std::endl;

    int p_tries = 10;
    for (int i = 0; i < p_tries; ++i) {
        double p = .75;//exp(-i*.3);
        int playouts = exp (device.uniform() * 3 + 8);
        std::cout << std::endl << "P: " << p << std::endl;
        std::cout << "playouts: " << playouts << std::endl;

        std::array<double, 9> strategy0 = {0};
        std::array<double, 9> strategy1 = {0};
        std::array<double, 9> strategy0_ = {0};
        std::array<double, 9> strategy1_ = {0};

        int rounds = 30;
        // mean_strategies<Exp3p, MonteCarlo>
        // (session, model, toy, playouts, rounds, strategy0, strategy1);
        mean_strategies<Exp3pWithPolicy, MonteCarloWithPolicy>
        (session_, model_, toy, playouts, rounds, strategy0_, strategy1_);
    }

    

    return 0;
}
