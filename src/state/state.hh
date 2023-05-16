    #pragma once

    #include "types.hh"

    template <class _Types>
    class State {
        struct Actions;
        struct Transition;
        struct Types : _Types {
            using Actions = State::Actions;
            using Transition = State::Transition;
        };
        struct Actions {
            typename Types::VectorAction row_actions;
            typename Types::VectorAction col_actions;

        };

        typename Types::Seed seed;

        void get_actions ();

        void apply_actions(
            typename Types::Action row_action, 
            typename Types::Action col_action);

        void set_seed (typename Types::PRNG &device) {};

        typename Types::Payoff row_payoff, col_payoff;

        bool is_terminal = false;


    };
