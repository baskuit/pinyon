#pragma once

#include "libsurskit/math.hh"
#include "libsurskit/vector.hh"

#include <concepts>
struct AbstractTypeList
{
};
// "_Name" so that the Type name does not shadow the template
template <typename _Action,
          typename _Observation,
          typename _Probability,
          typename _Real,
          typename _VectorAction,
          typename _VectorReal,
          typename _VectorInt,
          typename _MatrixReal,
          typename _MatrixInt>
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
    using MatrixReal = _MatrixReal;
    using MatrixInt = _MatrixInt;
};

template <class _TypeList>
class AbstractState
{
public:
    struct Types : _TypeList
    {
        using TypeList = _TypeList;
    };
    struct Transition
    {
    };
    struct Actions
    {
    };
};

/*
Default State. Pretty much every implementation of anything so far assumes that the State object derives from this.
This is where our most basic assumptions about a "State" manifest. However, I'm not sure if the rest of Surskit makes assumptions
about this State being totally observed.
Indeed, the Node access() methods simply assume that the same chance node must be the same

We assume that calculating actions takes work, so we make it explicit.
The BanditTree algorithm is safe because it always calls actions before running inference or applying actions.
There may be a way to statically guarantee this.
*/

template <class TypeList>
class DefaultState : public AbstractState<TypeList>
{
    static_assert(std::derived_from<TypeList, AbstractTypeList>);

public:
    struct Transition;
    struct Actions;
    struct Types : AbstractState<TypeList>::Types
    {
        using Transition = DefaultState::Transition;
        using Actions = DefaultState::Actions;
    };

    struct Transition : AbstractState<TypeList>::Transition
    {
        typename Types::Observation obs;
        typename Types::Probability prob;
    };

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

    bool is_terminal = false;
    typename Types::Real row_payoff, col_payoff;
    Transition transition;
    Actions actions;

    void get_actions();
    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action);
};

/*
Handy alias.
The Real number data type is assumed to be double and Vector, Matrix types are handled with Arrays.
*/

template <int size, typename Action, typename Observation, typename Probability>
using StateArray = DefaultState<TypeList<
    Action,
    Observation,
    Probability,
    double,
    Array<Action, size>,
    Array<double, size>,
    Array<int, size>,
    Linear::Matrix<double, size>,
    Linear::Matrix<int, size>>>;

template <int size, typename Action, typename Observation, typename Probability>
using StateVector = DefaultState<TypeList<
    Action,
    Observation,
    Probability,
    double,
    Vector<Action>,
    Vector<double>,
    Vector<int>,
    Linear::Matrix<double, size>,
    Linear::Matrix<int, size>>>;

template <class TypeList>
class SolvedState : public DefaultState<TypeList>
{
    static_assert(std::derived_from<TypeList, AbstractTypeList>);

public:
    struct Types : DefaultState<TypeList>::Types
    {
    };
    typename Types::VectorReal row_strategy, col_strategy;
};

template <int size, typename Action, typename Observation, typename Probability>
using SolvedStateArray = SolvedState<TypeList<
    Action,
    Observation,
    Probability,
    double,
    Array<Action, size>,
    Array<double, size>,
    Array<int, size>,
    Linear::Matrix<double, size>,
    Linear::Matrix<int, size>>>;

/*
This represents states that accept input for the chance player.
Since this uses Obs as chance action, this must be fully observed.

Currently not used by any Search algorithms, but I have ideas.
*/

template <class TypeList>
class StateChance : public DefaultState<TypeList>
{
    static_assert(std::derived_from<TypeList, AbstractTypeList>);

public:
    struct Types : DefaultState<TypeList>::Types
    {
    };
    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action,
        typename Types::Observation chance_action);
};

/*
Tests:
Initializing ToyStates with different type lists to make sure std::vector and cheeky bool/rational implementations are working.
*/