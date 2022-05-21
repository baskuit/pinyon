#pragma once

#include <cstdlib>

struct State {
    char id;
    bool terminal;
    float payoff;

    int pp = 0;
    int length = 0;

    int m = 2;
    int n = 2;

    State (char id, bool terminal, float payoff, int pp, int length) :
        id(id), terminal(terminal), payoff(payoff), pp(pp), length(length) {}

    State* copy() {
        return new State(id, terminal, payoff, pp, length);
    }

    
    // I couldn't figure out how to use one super class 'State'
    // and then subclass it with the types of games: "Sucker Punch", "Choose 0 to win", etc
    // Instead I use a string id and change the transition rules thusly

    int transition(int i, int j) {
        if (id == 'u') { //sUcker punch
            if (pp == 0) {
                terminal = true;
                payoff = 0;
            } else {
                if (i == 0 && j == 0) {
                    pp -= 1;
                    payoff = pp/(float)(pp+1);
                } else if (i == 0 && j == 1) {
                    terminal = true;
                    payoff = 1;
                } else if (i == 1 && j == 0) {
                    terminal = true;
                    payoff = 1;
                } else {
                    terminal = true;
                    payoff = 0;
                }
            }
        } else if (id == 's') { // play0 chooSes 0 to win
            if (i == 0) {
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

        // Combo of stochastic and sucker. If player0 is to collect his win- "Knock on Stealthrock"
        // He must also play the winning stochastic game 
        } else if (id == 'c') { //Combo game
            if (pp == 0) {
                terminal = true;
                payoff = 0;
            } else {
                if (i == 0 && j == 0) {
                    pp -= 1;
                    payoff = pp/(pp+1);
                } else if (i == 0 && j == 1) {
                    id = 's';
                    payoff = 1;
                } else if (i == 1 && j == 0) {
                    id = 's';;
                    payoff = 1;
                } else {
                    terminal = true;
                    payoff = 0;
                }
            }
        } 

        return 0; //transition key. These games are deterministic, hence always return 0
    }

    float rollout () {
        while (terminal == false) {
            int i = rand() % 2;
            int j = rand() % 2;
            transition(i, j);
        }
        return payoff;
    }

};