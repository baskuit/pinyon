#pragma once

#include "state.hh"
#include <cstdlib>

class ToyState : public State {
public: 
    char id;
    int pp = 0;
    int length = 0;

    float payoff = .5f;
    float* NE_strategy0, NE_strategy1;

    ToyState () {
        rows = 2;
        cols = 2;
    };
    ToyState (char id, bool terminal, float payoff, int pp, int length) :
        id(id), payoff(payoff), pp(pp), length(length) {
            rows = 2;
            cols = 2;
            terminal = terminal;
            payoff = payoff;
    };

    ToyState* copy() {
        return new ToyState(id, terminal, payoff, pp, length);
    }

    StateTransitionData transition(int row_idx, int col_idx) {
        if (id == 'u') { //sUcker punch
            if (pp == 0) {
                terminal = true;
                payoff = 0;
            } else {
                if (row_idx == 0 && col_idx == 0) {
                    pp -= 1;
                    payoff = pp/(float)(pp+1);
                } else if (row_idx == 0 && col_idx == 1) {
                    terminal = true;
                    payoff = 1;
                } else if (row_idx == 1 && col_idx == 0) {
                    terminal = true;
                    payoff = 1;
                } else {
                    terminal = true;
                    payoff = 0;
                }
            }


        } else if (id == 's') { // play0 chooSes 0 to win
            if (row_idx == 0) {
                if (length == 0) {
                    terminal = true;
                    payoff = 1;
                } else {
                    payoff = 1;
                    length -= 1;
                }
            } else {
                terminal = true;
                payoff = 0;
            }


        } else if (id == 'c') { //Combo game
            if (pp == 0) {
                terminal = true;
                payoff = 0;
            } else {
                if (row_idx == 0 && col_idx == 0) {
                    pp -= 1;
                    payoff = pp/(pp+1);
                } else if (row_idx == 0 && col_idx == 1) {
                    id = 's';
                    payoff = 1;
                } else if (row_idx == 1 && col_idx == 0) {
                    id = 's';;
                    payoff = 1;
                } else {
                    terminal = true;
                    payoff = 0;
                }
            }
        } 
        StateTransitionData result = {0, Rational(1, 1)};
        return result;
    }

    float rollout () {
        std::cout << "derived rollout" << std::endl;
        while (terminal == false) {
            int row_idx = rand() % 2;
            int col_idx = rand() % 2;
            this->transition(row_idx, col_idx);
        }
        return this->payoff;
    }

};