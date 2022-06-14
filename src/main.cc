#include <iostream>

#include <thread>   

#include "libsurskit/math.hh"
#include "state/state.hh"
#include "tree/node.hh"
#include "model/model.hh"
#include "tree/node.hh"
#include "search/exp3.hh"
#include "search/exp3p.hh"

template <typename Session>
double log_loss(ToyState<9>& state, Session& answer) {
    float loss = 0;
    for (int row_idx = 0; row_idx < state.rows; ++row_idx) {
        //std::cout << state.strategy0[row_idx]  << ' ' << log(answer.strategy0[row_idx] + 1/(double)answer.playouts) << std::endl;
        loss -= state.strategy0[row_idx] * log(answer.strategy0[row_idx] + 1/(double)answer.time);
    }
    for (int col_idx = 0; col_idx < state.cols; ++col_idx) {
        loss -= state.strategy1[col_idx] * log(answer.strategy1[col_idx] + 1/(double)answer.time);
    }
    return loss;
}

template <typename Session>
double evalSearchParams (int trials, double eta, int playouts, ToyState<9> state) {
    double avg_loss = 0;
    for (int i = 0; i < trials; ++i) {
        prng device;
        MatrixNode<9, Exp3pStats<9>> root;
        MonteCarlo<9> model(device);
        Session session(device, state, model, &root, playouts);
        session.search(playouts);
        auto answer = session.answer();
        float ce_loss = log_loss(state, answer); // does it deduce tpye? does this work?
        avg_loss += ce_loss;
    }
    avg_loss /= trials;
    return avg_loss;
}

template <typename Session>
void hyperparameter_search (int samples, int trials, ToyState<9>& state) {
    for (int sample = 0; sample < samples; ++sample) {
        double eta = .5 * state.device.uniform();
        int playouts = pow(2, 12 + state.device.random_int(8));
        double avg_loss = evalSearchParams<Session>(trials, eta, playouts, state);
        std::cout << "Sample " << sample << ": " << playouts << " " << eta << " : " << avg_loss << std::endl;
    }

}

int main () {

    prng device;

    ToyState<9> toy(device, 'w', 4, 2);
    toy.transition(0, 0);
    MonteCarlo<9> model(device);
    MatrixNode<9, Exp3pStats<9>> root;
    int playouts = 10000;
    Exp3pSearchSession<9, ToyState<9>> session (device, toy, model, &root, playouts);

    std::vector<Exp3pAnswer<9>> answers;
    hyperparameter_search<Exp3pSearchSession<9, ToyState<9>>> (30, 8, toy); 
    
    //while (true) {}


}