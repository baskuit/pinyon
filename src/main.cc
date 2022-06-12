#include <iostream>

#include "libsurskit/math.hh"
#include "state/state.hh"
#include "tree/node.hh"
#include "model/model.hh"
#include "tree/node.hh"
#include "search/exp3.hh"

double log_loss(ToyState<9>& state, Exp3Answer<9>& answer) {
    float loss = 0;
    for (int row_idx = 0; row_idx < state.rows; ++row_idx) {
        //std::cout << state.strategy0[row_idx]  << ' ' << log(answer.strategy0[row_idx] + 1/(double)answer.playouts) << std::endl;
        loss -= state.strategy0[row_idx] * log(answer.strategy0[row_idx] + 1/(double)answer.playouts);
    }
    for (int col_idx = 0; col_idx < state.cols; ++col_idx) {
        loss -= state.strategy1[col_idx] * log(answer.strategy1[col_idx] + 1/(double)answer.playouts);
    }
    return loss;
}

double evalExp3SearchParams (int trials, double eta, int playouts, ToyState<9> state) {
    double avg_loss = 0;
    for (int i = 0; i < trials; ++i) {
        prng device;
        MatrixNode<9, Exp3Stats<9>> root;
        MonteCarlo<9> model(device);
        Exp3SearchSession<9, ToyState<9>> session(device, &root, state, model, eta);
        session.search(playouts);
        auto answer = session.answer();
        // answers.push_back(answer);
        float ce_loss = log_loss(state, answer);
        avg_loss += ce_loss;
    }
    avg_loss /= trials;
    return avg_loss;
}

void hyperparameter_search (int samples, int trials, ToyState<9>& state) {
    for (int sample = 0; sample < samples; ++sample) {
        double eta = .5 * state.device.uniform();
        int playouts = exp(12 + 6*state.device.uniform());
        double avg_loss = evalExp3SearchParams(trials, eta, playouts, state);
        std::cout << "Sample " << sample << ": " << playouts << " " << eta << " : " << avg_loss << std::endl;
    }

}

int main () {

    prng device;

    ToyState<9> toy(device, 'w', 4, 2);
    toy.transition(0, 0);

    std::vector<Exp3Answer<9>> answers;
    hyperparameter_search (20, 10, toy); 
    
    std::cout << evalExp3SearchParams(10, 0.0682911, 58033, toy) << std::endl;
    //while (true) {}


}