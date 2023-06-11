#pragma once

#include <type_traits>

#define SEARCH_PARAMS(State, SolvedState, _Model, BanditAlgorithm, TreeBandit)                                                                                      \
    template <                                                                                                                                                      \
        class State,                                                                                                                                                \
        template <                                                                                                                                                  \
            class S>                                                                                                                                                \
        class _Model,                                                                                                                                               \
        template <                                                                                                                                                  \
            class __Model,                                                                                                                                          \
            template <class ___Model, class _BanditAlgorithm, template <class ____Model> class _Outcome, template <class A> class _MNode, template <class A> class> \
            class _TreeBandit,                                                                                                                                      \
            template <class A> class _MNode,                                                                                                                        \
            template <class A> class _CNode>                                                                                                                        \
        class BanditAlgorithm,                                                                                                                                      \
        template <                                                                                                                                                  \
            class M,                                                                                                                                                \
            class BA,                                                                                                                                               \
            template <class _M> class _Outcome,                                                                                                                     \
            template <class A> class _MNode,                                                                                                                        \
            template <class A> class _CNode>                                                                                                                        \
        class TreeBandit>

#define SEARCH_PARAMS_ENABLE(State, _Model, BanditAlgorithm, TreeBandit, Enable)                                                                                    \
    template <                                                                                                                                                      \
        class State,                                                                                                                                                \
        template <                                                                                                                                                  \
            class S>                                                                                                                                                \
        class _Model,                                                                                                                                               \
        template <                                                                                                                                                  \
            class __Model,                                                                                                                                          \
            template <class ___Model, class _BanditAlgorithm, template <class ____Model> class _Outcome, template <class A> class _MNode, template <class A> class> \
            class _TreeBandit,                                                                                                                                      \
            template <class A> class _MNode,                                                                                                                        \
            template <class A> class _CNode>                                                                                                                        \
        class BanditAlgorithm,                                                                                                                                      \
        template <                                                                                                                                                  \
            class M,                                                                                                                                                \
            class BA,                                                                                                                                               \
            template <class _M> class _Outcome,                                                                                                                     \
            template <class A> class _MNode,                                                                                                                        \
            template <class A> class _CNode>                                                                                                                        \
        class TreeBandit,                                                                                                                                           \
        typename Enable = void>

struct S // TODO Rename
{
    virtual void run(
        size_t iterations) = 0;

    virtual void get_strategies(
        std::vector<double> &row_strategy,
        std::vector<double> &col_strategy)
    {
    }

    virtual double get_exploitability() = 0;

    virtual void forward()
    {
        // search and advance
    }
};

// Declaration

SEARCH_PARAMS(State, SolvedState = State, _Model, BanditAlgorithm, TreeBandit)
struct Search : S
{
    using Types = State::Types;
    using Model = _Model<State>;
    using Algorithm = BanditAlgorithm<Model, TreeBandit, MatrixNode, ChanceNode>;

    typename State::Types::PRNG device;
    State *state;
    TraversedState<Model> *solved_state{nullptr};
    Model *model;
    Algorithm session;
    MatrixNode<Algorithm> root;

    Search(State &state, Model &model) : state{&state}, model{&model}
    {
    }

    Search(State &state, Model &model, TraversedState<Model> &solved_state)
        : state{&state}, model{&model}, solved_state{&solved_state}
    {
    }

    Search(State *state, Model *model) : state{state}, model{model}
    {
    }

    template <class... Args>
    Search(State *state, Model *model, Args... args) : state(state), model(model), session(args...)
    {
    }

    Search &operator()(State *state, Model *model)
    {
        root = MatrixNode<Algorithm>();
        return (*this); // TODO does this delete?
    }

    void run(
        size_t iterations)
    {
        session.run(iterations, device, *state, *model, root);
    }

    double get_exploitability()
    {
        if (solved_state == nullptr)
        {
            return -1;
        }
        else
        {
            typename Types::MatrixValue matrix;
            solved_state->get_payoff_matrix(matrix);
            typename Types::VectorReal row_strategy, col_strategy;
            session.get_empirical_strategies(&root, row_strategy, col_strategy);
            double expl = exploitability<Types>(matrix, row_strategy, col_strategy);
            return expl;
        }
    };

    State get_new_state () {
        State state{};
        return state;
    }


};