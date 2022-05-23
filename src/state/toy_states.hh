#pragma once

#include "state.hh"
#include <cstdlib>

class ToyState : public State {
public: 
    bool terminal;
    char id;
    int pp = 0;
    int length = 0;

    float payoff;
    float* NE_strategy0, NE_strategy1;

    int rows = 2;
    int cols = 2;

    ToyState ();
    ToyState (char id, bool terminal, float payoff, int pp, int length) :
        id(id), terminal(terminal), payoff(payoff), pp(pp), length(length) {}

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
        while (terminal == false) {
            int row_idx = rand() % 2;
            int col_idx = rand() % 2;
            transition(row_idx, col_idx);
        }
        return payoff;
    }

};