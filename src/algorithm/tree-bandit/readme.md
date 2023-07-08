# Tree Bandit

The algorithms in the folder are generalizations of MCTS, which we call 'tree bandit'.

> In mathematics, "Monte Carlo" refers to estimation via random process. MCTS gets its name sake from the random rollout method of value estimation.

 The tree bandit algorithms are defined in two parts. 

* First there is the bandit algorithm, which takes the model type is its sole template parameter. This component decides how the algorithm chooses the next node in the tree to visit.

* Secondly, there is the tree algorithm. This component wraps the bandit algorithm by accepting it as its first template parameter (The State and basic types are deduced from that). It also accepts template parameters for the types/implementation of the matrix and chance nodes, as well as a boolean that determines whether the algorithms stops exploring the tree when it reaches an unexpanded node.

## `BanditAlgorithm`

Bandit algorithms get their name from 'multi-armed bandits'. To quote Wikipedia:

> The multi-armed bandit problem models an agent that simultaneously attempts to acquire new knowledge (called "exploration") and optimize their decisions based on existing knowledge (called "exploitation"). The agent attempts to balance these competing tasks in order to maximize their total value over the period of time considered.

### A Terse Theoretical Overview

There is an entire field of mathematics devoted to describing and analyzing algorithms for multi-armed bandits. This field is itself split depending on the assumptions that are made about the nature of the problem. The two most studied of these sub-fields are *stochastic bandits* and *adversarial bandits*.
 
 The standard formula in Monte Carlo tree search

$$ \frac{w_i}{n_i} + c \sqrt{\frac{\ln N_i}{n_i}} $$

is often called UCB (Upper Confidence Bounds). It is a solution to the stochastic bandits problem. By this, we mean that the rewards that are received 

The Exp3 and MatrixUCB algorithms are already provided. Not all bandits algorithms (i.e. stochastic bandit algorithms) are sound choices. Refer to "Analysis of Hannan Consistent Selection for Monte Carlo Tree Search in Simultaneous Move Games".

### Interface

To work with all the default tree algorithms, there is a lightweight interface that is expected from bandit algorithms. This interface can be exampled by the implementation of `Rand`, which is basically a bench-marking algorithm that chooses randomly and maintains basically no statistics.

The entire class is defined below.

```cpp
template <class Model>
class Rand : public AbstractAlgorithm<Model>
{
public:
    struct MatrixStats;
    struct ChanceStats;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using MatrixStats = Rand::MatrixStats;
        using ChanceStats = Rand::ChanceStats;
    };
    struct MatrixStats
    {
        int rows, cols;
    };
    struct ChanceStats {};
    struct Outcome {};
    friend std::ostream &operator<<(std::ostream &os, const Rand &session)
    {
        os << "Rand";
        return os;
    }
    void get_empirical_strategies(
        MatrixStats &stats,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy) {}
    void get_empirical_value(
        MatrixStats &stats,
        typename Types::Value &value) {}
    void get_refined_strategies(
        MatrixStats &stats,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy) {}
    void get_refined_value(
        MatrixStats &stats,
        typename Types::Value &value) {}

protected:
    void initialize_stats(
        int iterations,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixStats &stats) {}
    void expand(
        typename Types::State &state,
        MatrixStats& stats,
        typename Types::ModelOutput &inference)
    {
        stats.rows = state.row_actions.size();
        stats.cols = state.col_actions.size();
    }
    void select(
        typename Types::PRNG &device,
        MatrixStats &stats,
        typename Types::Outcome &outcome)
    {
        const int row_idx = device.random_int(stats.rows);
        const int col_idx = device.random_int(stats.cols);
        outcome.row_idx = row_idx;
        outcome.col_idx = col_idx;
    }
    void update_matrix_stats(
        MatrixStats &stats,
        typename Types::Outcome &outcome) {}
    void update_chance_stats(
        ChanceStats &stats,
        typename Types::Outcome &outcome) {}
    void update_matrix_stats(
        MatrixStats &stats,
        typename Types::Outcome &outcome,
        typename Types::Real learning_rate) {}
    void update_chance_stats(
        ChanceStats &stats,
        typename Types::Outcome &outcome,
        typename Types::Real learning_rate) {}
    void get_policy(
        MatrixStats &stats,
        typename Types::VectorReal &row_policy,
        typename Types::VectorReal &col_policy) {}
};

```
#### Explanation of the Above

There are three structs that all bandit algorithms must define:

* `MatrixStats`
Contains the stats the algorithm needs to use for the selection and update process. This minimal example only needs to store the number of actions for the players, but something like `exp3` would need to store the exponential weights and probably also some other hyper-parameters.
This struct should be as small as possible, as it has the most impact on cache-efficiency. The MatrixUCB algorithm is problematic in this regard, since it has to store $n^2$ statistics for the matrix instead of $2 n$  like most adversarial bandits, with respect to the number of actions $n$.
* `ChanceStats`
This storage is not needed by the provided algorithms, but it could be useful in the future. It's worth mentioning that if the struct is empty and its associated methods are no-ops, it should be entirely optimized away by the compiler.
* `Outcome`
A struct that is used for storage of any data or observations that need to be shared between the selection and update phases. For virtually all bandit algorithms, this will need to include the action (indices) that were selected. `Rand` does not store even these simply because its update is a no-op.

An `ostream` operator is useful but not essential.

The following methods are expected.

* `get_empirical_strategies`
* `get_empirical_value`
* `get_refined_strategies`
* `get_refined_value`

Used as the 'final answer' interface. All bandit methods can off empirical value and policies for that purpose, and in most RL schemes this is taken to be the training target. There are many cases where you may want to use other estimates instead, hence the 'refined' version.
e.g. alpha zero stuff using `q` over `z`, or a mix of the two.

* `initializes_stats`
This method may be removed in the future. The vanilla MatrixUCB algorithm assumes that we know the number of iterations in advance, which is only true for the root node. This method was used to set the total number of iterations in the `MatrixStats` of the root node. Any other node would use the expected iterations of its parent matrix node to estimate its own expected episodes. 

* `expand`
This method properly initializes the statistics of the matrix stats. In exp3 for example, it zero-initializes the gains and visit count vectors for both players.

* `select`
Chooses the joint actions for both players to commit and hence the associated chance node to visit.

* `update_matrix_stats`
* `update_chance_stats` 
Updates the stats in the matrix and selected chance node for the next episode. Uses the value observed at the end of the forward phase.

* `get_policy`
Off policy only


## TreeAlgorithm

The bandit algorithm decides how the tree is expanded and explored, which is outside the scope of the bandit algorithm.

**This class is derived from the bandit algorithm.**

Because of this, the tree algorithm inherits all of the methods of the bandit algorithm.

It will also inherit the three structs `MatrixStats`, `ChanceStats`, and `Outcome` from before. Inheriting the methods and data structures of the bandit algorithms is what allows the interchangeability of latter.

But the tree algorithm will not use the `MatrixStats` and `ChanceStats` just as inherited. Instead, it will derive its own classes to add extra, necessary data. For the multi-threaded tree algorithms, a mutex or mutex-id is added to protect the data in the stats from race conditions.

```cpp

```

The matrix and chance nodes are templates that accept algorithms as parameters. It is the *tree* algorithms that is passed as arguments for this, so that the augmented `TreeAlgorithm::MatrixStats` and `TreeAlgorithm::ChanceStats` are used in the tree structure, not their respective base classes.

### Tree Search and RL Analogy

The default `return_if_expand` template parameter is a good moment to introduce this design principle of Surskit.

### Defaults

Unlike the bandit algorithms, it is expected that the provided tree algorithms should accommodate most experiments.

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
