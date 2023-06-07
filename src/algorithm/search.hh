#pragma once

#include <surskit.hh>

struct S // TODO Rename
{
    virtual void run(
        size_t iterations) = 0;

    virtual void get_strategies (
        std::vector<float> &row_strategy,
        std::vector<float> &col_strategy,
        float row_value, 
        float col_value
    ) {

    }

    virtual void forward () {
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
    class _BanditAlgorithm,

    template <class M, class BA,
              template <class _M> class _Outcome, template <class A> class _MNode, template <class A> class _CNode>
    class TreeBandit>
struct Search<State, _Model, _BanditAlgorithm, TreeBandit> : S
{

    using Model = _Model<State>;
    using Algorithm = _BanditAlgorithm<Model, TreeBandit, MatrixNode, ChanceNode>;

    typename State::Types::PRNG device;
    State *state;
    Model *model;
    Algorithm session;
    MatrixNode<Algorithm> root;

    Search() : state{}, model{}
    {
    }

    Search(State *state, Model *model) : state{state}, model{model}
    {
    }

    template <class... Args>

    Search(State* state, Model* model, Args... args) : state(state), model(model), session(args...)
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

temp,l.ate

int main()
{

    // TODO make it easier for battle wrappers to use the same engine battle

    Search<Battle<64>, MonteCarloModel, Exp3, TreeBandit> x(nullptr, nullptr, .01);
    Search<Battle<128>, MonteCarloModel, Exp3, TreeBandit> y{};
    S *arr[2] = {&x, &y};

    for (S *search : arr)
    {
        search->run(1000);
    }

    // SearchList<
    // Search<Battle<128>, MonteCarloModel, Exp3, TreeBandit>,
    // Search<Battle<128>, MonteCarloModel, Exp3, TreeBandit>,
    // Search<Battle<128>, MonteCarloModel, Exp3, TreeBandit>,
    // Search<Battle<128>, MonteCarloModel, Exp3, TreeBandit>
    // > pool {
    //     {.01}, {.05}, {.1}, {.5}
    // };

    // SearchList<

    return 0;

    
}

