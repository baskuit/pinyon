#pragma once

#include <surskit.hh>

// #include <battle-surskit.hh>

struct S // TODO Rename
{
    virtual void run(
        size_t iterations) = 0;

    virtual void get_strategies(
        std::vector<double> &row_strategy,
        std::vector<double> &col_strategy)
    {
    }

    virtual void forward()
    {
        // search and advance
    }
};

template <

    class State,

    template <class S> class _Model,

    template <
        class __Model,
        template <class ___Model, class _BanditAlgorithm, template <class ____Model> class _Outcome, template <class A> class _MNode, template <class A> class>
        class _TreeBandit,
        template <class A> class _MNode,
        template <class A> class _CNode>
    class BanditAlgorithm,

    template <class M, class BA,
              template <class _M> class _Outcome, template <class A> class _MNode, template <class A> class _CNode>
    class TreeBandit>
struct Search : S
{

    using Model = _Model<State>;
    using Algorithm = BanditAlgorithm<Model, TreeBandit, MatrixNode, ChanceNode>;

    typename State::Types::PRNG device;
    State *state;
    Model *model;
    Algorithm session;
    MatrixNode<Algorithm> root;

    Search() : state{}, model{}
    {
    }

    Search(State &state, Model &model) : state{&state}, model{&model}
    {
    }

    Search(State *state, Model *model) : state{state}, model{model}
    {
    }

    template <class... Args>

    Search(State *state, Model *model, Args... args) : state(state), model(model), session(args...)
    {
    }

    // template <class... Args>
    // Search(Args... args) : state(), model()
    // {
    //     session = Algorithm(args...);
    // }

    Search &operator()(State *state, Model *model)
    {
        root = MatrixNode<Algorithm>();
        return (*this);
    }

    void run(
        size_t iterations)
    {
        session.run(iterations, device, *state, *model, root);
    }
};

template <
    class State,

    template <class S> class _Model,

    template <
        class __Model,
        template <class ___Model, class _BanditAlgorithm, template <class ____Model> class _Outcome, template <class A> class _MNode, template <class A> class>
        class _TreeBandit,
        template <class A> class _MNode,
        template <class A> class _CNode>
    class BanditAlgorithm,

    template <class M, class BA,
              template <class _M> class _Outcome, template <class A> class _MNode, template <class A> class _CNode>
    class TreeBandit>
Search<State, _Model, BanditAlgorithm, TreeBandit> foo(State &state, _Model<State> &model) {
    return Search<State, _Model, BanditAlgorithm, TreeBandit>(state, model);
}