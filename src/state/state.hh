#pragma once

#include "../libsurskit/math.hh"

#include <concepts>
struct AbstractTypeList {};
// _Name so that the Type name does not shadow the template
template <typename _Action,
          typename _Observation,
          typename _Probability,
          typename _Real,
          typename _VectorAction,
          typename _VectorReal,
          typename _VectorInt>
// TODO requires std::floating_point<_Real>
struct TypeList : AbstractTypeList
{
    using Action = _Action;
    using Observation = _Observation;
    using Probability = _Probability;
    using Real = _Real;
    using VectorAction = _VectorAction;
    using VectorReal = _VectorReal;
    using VectorInt = _VectorInt;
};

template <class _TypeList>
// underscore when going up a level, since you don't want to shadow template param with Type
class AbstractState
{
public:
    struct Types : _TypeList
    {
        using TypeList = _TypeList;
    };
    struct Transition {};
    struct Actions {};
};

/*
Default State
*/
template <class TypeList>
class State : public AbstractState<TypeList>
{
static_assert(std::derived_from<TypeList, AbstractTypeList> == true);
public:
    struct Transition;
    struct Actions;
    struct Types : AbstractState<TypeList>::Types
    {
        using Transition = State::Transition;
        using Actions = State::Actions;
    };

    bool is_terminal = false;
    typename Types::Real row_payoff, col_payoff;

    struct Transition : AbstractState<TypeList>::Transition
    {
        typename Types::Observation obs;
        typename Types::Probability prob;
    };
    Transition transition;

    struct Actions : AbstractState<TypeList>::Actions
    {
        typename Types::VectorAction row_actions;
        typename Types::VectorAction col_actions;
        int rows;
        int cols;

        void print()
        {
            std::cout << "row_actions: ";
            for (int i = 0; i < rows; ++i)
            {
                std::cout << row_actions[i] << ", ";
            }
            std::cout << std::endl;
            std::cout << "col_actions: ";
            for (int j = 0; j < cols; ++j)
            {
                std::cout << col_actions[j] << ", ";
            }
            std::cout << std::endl;
        }
    };
    Actions actions;

    void get_actions();
    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action);
};
/*
Handy alias
*/
template <int size, typename Action, typename Observation, typename Probability>
using StateArray = State<TypeList<Action, Observation, Probability, double, std::array<Action, size>, std::array<double, size>, std::array<int, size>>>;

/*
This represents states that accept input for the chance player.
*/

template <class TypeList>
class StateChance : public State<TypeList>
{
static_assert(std::derived_from<TypeList, AbstractTypeList> == true);
public:
    struct Types : State<TypeList>::Types
    {
    };
    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action,
        typename Types::Observation chance_action);
};

template <class TypeList>
class SolvedState : public State<TypeList>
{
static_assert(std::derived_from<TypeList, AbstractTypeList> == true);
public:
    struct Types : State<TypeList>::Types
    {
    };
    typename Types::VectorReal row_strategy, col_strategy;
};