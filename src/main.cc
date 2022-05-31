#include <iostream>

#include <libsurskit/math.hh>
#include "state/toy_state.hh"

void print (SolvedStateInfo& info) {
    std::cout << info.rows << ", " << info.cols << std::endl;
    std::cout << info.strategy0[0] << ", " << info.strategy1[0] << std::endl;
    std::cout << info.terminal << ", " << info.payoff << std::endl;
}

void prng_copy_test () {
    prng x;
    std::cout << x.uniform() << std::endl;
    prng y = x;
    std::cout << x.uniform() << std::endl;
    std::cout << y.uniform() << std::endl;
}

void prng_copy_test2 () {
    prng x;
    std::cout << x.uniform() << std::endl;
    prng y;
    y = x;
    std::cout << x.uniform() << std::endl;
    std::cout << y.uniform() << std::endl;
}

void state_copy_prng_test() {
    ToyState x = ToyState('u', 2, 2, .5f);
    std::cout << x.device.uniform() << std::endl;
    ToyState y = x;
    std::cout << x.device.uniform() << std::endl;
    std::cout << y.device.uniform() << std::endl;
}

void print_state_info (ToyStateInfo info) {
    return;
    
    std::cout << "terminal: " << info.terminal << std::endl;
    return;
    std::cout << "rows: " << info.rows << " cols: " << info.cols << std::endl;
    std::cout << "terminal: " << info.terminal << std::endl;

    std::cout << "payoff: " << info.payoff << std::endl;
    std::cout << "id: " << info.id << std::endl;
    std::cout << "pp: " << info.pp << std::endl;
    std::cout << "length: " << info.length << std::endl;
}

void toy_state_copy_test () {
    int N = 1;
    for (int i = 0; i < N; ++i) {
        ToyState* x = new ToyState('u', 2, 2, .5f);
        ToyStateInfo info = x->info;
        return;
        print_state_info(info);

    }
    std::cout << "good" << std::endl;
}

void simple_state_info_copy () {
    SolvedStateInfo* x = new SolvedStateInfo(2, 2, .5f);
    SolvedStateInfo x_ = *x;


    std::cout << x->strategy0 << std::endl;
    std::cout << x_.strategy0 << std::endl;

}

int main () {
    
    toy_state_copy_test();

}