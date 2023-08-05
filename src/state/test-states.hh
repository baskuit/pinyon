#pragma once

#include <state/state.hh>

/*
 Large uniform tree for testing etc. So called because it grows until it can't.
*/

template <size_t size>
struct MoldState : SimpleTypes
{
    class State;
    // MoldState (State state) {}

    // using StateTypes = MoldState;

    class State : public PerfectInfoState<SimpleTypes>
    {
    public:
        size_t max_depth = 1;

        State(size_t max_depth) : max_depth((max_depth >= 0) * max_depth)
        {
            this->terminal = (this->max_depth == 0);
            this->init_range_actions(size);
            this->prob = SimpleTypes::Prob{1};
            this->obs = SimpleTypes::Obs{};
        }

        void randomize_transition(SimpleTypes::PRNG &device)
        {
        }

        void get_actions()
        {
        }

        void get_actions(
            SimpleTypes::VectorAction &row_actions,
            SimpleTypes::VectorAction &col_actions)
        {
            row_actions = this->row_actions;
            col_actions = this->col_actions;
        }

        void apply_actions(
            SimpleTypes::Action,
            SimpleTypes::Action)
        {
            --this->max_depth;
            this->terminal = (this->max_depth == 0);
        }

        void get_chance_actions(
            std::vector<SimpleTypes::Obs> &chance_actions,
            SimpleTypes::Action,
            SimpleTypes::Action) const
        {
            chance_actions.resize(1);
        }

        void apply_actions(
            SimpleTypes::Action,
            SimpleTypes::Action,
            SimpleTypes::Obs)
        {
            --this->max_depth;
            this->terminal = (this->max_depth == 0);
        }
    };

    // static_assert(IsStateTypes<MoldState>);
};
