#include <iostream>
#include <assert.h> 

#include <libsurskit/math.hh>
#include "state/toy_state.hh"


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


void sucker_rollout_estimate (int pp, int n) {
    float total = 0;
    ToyState* S = new ToyState('u', pp, 0, .5f);
    for (int i = 0; i < n; ++i) {
        // ToyState* S = new ToyState('u', pp, 0, .5f);
        ToyState* S_ = S->copy();
        total += S_->rollout();
        delete S_;
    }
    std::cout << "Sucker estimate - pp: " << pp << "   " << total/n << std::endl; 
}

void sucker_rollout_estimate2 (int pp, int n) {
    float total = 0;
    ToyState* S = new ToyState('u', pp, 0, .5f);
    ToyState* states[n];
    for (int i = 0; i < n; ++i) {
        states[i] = S->copy();
    }
}

int main () {
    
    int n = 1000000;
    for (int pp = 1; pp < 4; ++pp) {
        sucker_rollout_estimate(pp, n);
    }

}