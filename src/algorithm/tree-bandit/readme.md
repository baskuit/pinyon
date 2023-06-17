# Tree Bandit

The algorithms in the folder are generalizations of MCTS, which we call 'tree bandit'.

> In mathematics, "Monte Carlo" refers to estimation via random process. MCTS gets its name sake from the random rollout method of value estimation.

 The tree bandit algorithms are defined in two parts. 

* First there is the bandit algorithm, which takes the model type is its sole template parameter. The tree component takes the bandit type as its first template parameter, as well as the types of the nodes and ...

* The tree component wraps the bandit component. It derives its own `MatrixStats`, `ChanceStats` from those in the former, to add any necessary info.

## BanditAlgorithm

The Exp3 and MatrixUCB algorithms are already provided. Not all bandits algorithms (i.e. stochastic bandit algorithms) are sound choices. Refer to "Analysis of Hannan Consistent Selection for Monte Carlo Tree Search in Simultaneous Move Games".

The interface that a bandit algorithm must have is best explained by looking at Exp3:

```cpp
template <class Model>
class Exp3 : AbstractAlgorithm<Model>
{
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Outcome;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using MatrixStats = Exp3::MatrixStats;
        using ChanceStats = Exp3::ChanceStats;
        using Outcome = Exp3::Outcome;
    };
    struct MatrixStats
    {
        typename Types::VectorReal row_gains;
        typename Types::VectorReal col_gains;
        typename Types::VectorInt row_visits;
        typename Types::VectorInt col_visits;
        int visits = 0;
        typename Types::Value value_total;
    };
    struct ChanceStats {};
    struct Outcome
    {
        ActionIndex row_idx, col_idx;
        typename Model::Types::Value value;
        typename Types::Real row_mu, col_mu;
    };
 //...
};
```

There are three structs that all bandit algorithms must define:

* `MatrixStats`
* `ChanceStats`
* `Outcome`

The following methods are expected.

## TreeAlgorithm

The base algorithms are:

* `TreeBandit`
Essentially vanilla MCTS

* `TreeBanditThreaded`
The CRTP is used here to add a mutex member to the matrix stats of the bandit algorithm. This mutex is locked before accessing chance stats for selection and updating.

* `TreeBanditThreadPool`
To save on memory compared to the above, the instances of the algorithms maintain a pool of mutexes, and the index of a matrix node is stored in its stats instead.

* `OffPolicy`
The name might be misleading. Its basically intended for use with batched GPU inference.

The reason for the name is the way it handles selecting and updating nodes. Many other implementations of parallel search use 'virtual loss' as a way of diversifying the threads' leaf node selection. Additionally with this method we update the matrix node stats like normal during the backward phase.

This method relies on certain properties of UCB, the standard bandit algorithm in games like Chess. Instead we use artificial sampling (like tossing repeated leaf nodes) and instead treat the process of updating the stats in the backward phase as an instance of off policy learning in RL. This base maintains the 'actor policies' (`row_mu, col_mu`) of the forward phase and uses those in conjunction with the selection probabilities in the backward phase (the "learner" policy) to calculate the ratio `pi / mu`. This coefficient to adjust the 'learning rate' of the node is a common trick in order to un-bias the samples in the context of off-policy RL.
