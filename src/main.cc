#include <iostream>

#include "libsurskit/math.hh"
#include "state/toy_state.hh"
#include "model/monte_carlo.hh"
#include "tree/node.hh"
#include "search/session.hh"


int main () {
    prng device;

    ToyState x = ToyState(&device, 'u', 3, 0, .5f);
    
    Action actions0[2];
    Action actions1[2];
    PairActions pair(2, 2, actions0, actions1);

    float strategy0[2];
    float strategy1[2];
    InferenceData inference_data = InferenceData(strategy0, strategy1);

    Model* model = new MonteCarlo(&device);
    
    int playouts = 1000000;
    float total = 0.f;

    for (int playout = 0; playout < playouts; ++playout) {

        //prng device_; SLOW!!!
        ToyState x_ = ToyState(&device);
        x.copy(&x_);

        model->inference(&x_, pair, inference_data);

        total += inference_data.value_estimate0;
    }
    std::cout << total / playouts << std::endl;

 
    return 0;
}