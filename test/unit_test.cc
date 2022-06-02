#include <iostream>
#include <assert.h> 

#include <libsurskit/math.hh>
#include "state/toy_state.hh"


void print (SolvedStateInfo& info) {
    std::cout << info.rows << ", " << info.cols << std::endl;
    std::cout << info.strategy0[0] << ", " << info.strategy1[0] << std::endl;
    std::cout << info.terminal << ", " << info.payoff << std::endl;
}

void assert_prng_equal (prng* x, prng* y, int delta) {
    
    for (int i = 0; i < delta; ++i) {
        x->uniform();
    }
    for (int i = 0; i < 1000000; ++i) {
        assert (x->uniform() == y->uniform());
    }
}   

void prng_copy_test () {
    prng x;
    int n = 1000;
    for (int i = 0; i < n; ++i) {
        x.uniform();
    }
    prng y = x; // hard copy
    prng z = x.copy();

    for (int i = 0; i < n; ++i) {
        z.uniform();
    }
    for (int i = 0; i < n; ++i) {
        float a = x.uniform();
        float b = y.uniform();
        float c = z.uniform();
        assert (a == b);
        assert (b == c);
    }
}

void state_copy_prng_test() {
    ToyState x = ToyState('u', 2, 2, .5f);
    std::cout << x.device.uniform() << std::endl;
    ToyState y = x;
    std::cout << x.device.uniform() << std::endl;
    std::cout << y.device.uniform() << std::endl;
}

void toy_state_copy_test () {
    int N = 1000;
    for (int i = 0; i < N; ++i) {
        ToyState x = ToyState('u', 2, 2, .5f);
        ToyState y = x;
        ToyState* z = x.copy();

        if (x.rollout() != y.rollout()) {
        std::cout << "bad" << std::endl;
        }
    }
    std::cout << "good" << std::endl;
}

void sucker_rollout_estimate (int pp, int n) {
    float total = 0;
    ToyState* S = new ToyState('u', pp, 0, .5f);
    for (int i = 0; i < n; ++i) {
        ToyState* S_ = new ToyState(S->info);
        std::cout << S_->info->terminal << std::endl;
        total += S_->rollout();
    }
    std::cout << "Sucker estimate - pp: " << pp << "   " << total/n << std::endl; 
}

int main () {
    
    sucker_rollout_estimate(2, 100);

}