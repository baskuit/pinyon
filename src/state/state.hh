#pragma once

#include <string.h>

#include "../libsurskit/math.hh"


template <typename _PlayerAction, typename _ChanceAction, typename _Number,
          typename _VectorDouble, typename _VectorInt, typename _VectorAction>
class State
{
public:
    using PlayerAction = _PlayerAction;
    using ChanceAction = _ChanceAction;
    using Number = _Number;
    using VectorDouble = _VectorDouble;
    using VectorInt = _VectorInt;
    using VectorAction = _VectorAction;

    struct TransitionData
    {
        // TransitionData () {}
        // TransitionData (TransitionData &t) {}
        ChanceAction chance_action;
        Number probability;
    };

    TransitionData transition_data;
    // If we want TransitionData, PairActions to be nested classes,
    // then we can't make apply_actions, get_player_actions virtual
    // while still passing references to the output, since arguments are not covariate
    // Rather than resort to outputing these by value, we opt to just store these as members.

    struct PairActions
    {
        // PairActions () {}
        // PairActions (PairActions &t) {}
        int rows;
        int cols;
        VectorAction row_actions;
        VectorAction col_actions;
    };

    PairActions pair_actions;

    double row_payoff;
    double col_payoff;
    bool is_terminal = false;
    // We break from the old convention of using rows * cols = 0 iff terminal.
    // This is because we'd like Surskit to better handle situations where
    // we may not have complete knowledge of our a given player's actions.

    // State () {}
    // State (State &t) {}

    virtual void get_player_actions() = 0;
    virtual void apply_actions(
        PlayerAction row_action, 
        PlayerAction col_action
    ) = 0;
    // TransitionData get_player_actions() = 0;
};

template <int size, typename _PlayerAction, typename _ChanceAction, typename _Probability>
class StateArray : public State<
                       _PlayerAction,
                       _ChanceAction,
                       _Probability,
                       std::array<double, size>,
                       std::array<int, size>,
                       std::array<_PlayerAction, size>>
{
};

/*
This derived class represents states that accept input for the chance player.
The convention by which derived classes in Surskit access type names is displayed here
 `typename Derived::TypeName`
This gives us access to lower level types (State < Model < Algorithm < Node)
while sparing us the use of template arguments, which using the Base class would require.
*/

template <typename _PlayerAction, typename _ChanceAction, typename _Number,
          typename _VectorDouble, typename _VectorInt, typename _VectorAction>
class StateChance : public State< _PlayerAction,  _ChanceAction,  _Number,
           _VectorDouble,  _VectorInt,  _VectorAction>
{
public:
    virtual void apply_actions (
        typename StateChance::PlayerAction row_action,
        typename StateChance::PlayerAction col_action,
        typename StateChance::ChanceAction chance_action,
        typename StateChance::TransitionData &transition_data
    ) = 0;
};
