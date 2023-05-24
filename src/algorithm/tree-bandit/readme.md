The algorithms in the folder are variations of MCTS (Called "TreeBandit" herein) that follow this pattern

```
BanditAlgorithm<Model, BaseAlgorithm>
```

where `BanditAlgorithm` refers to the selection process that we apply at each node of the forward phase, and `Base` is some modifier that changes the search process. This is essentially a distinction of scope between the states/nodes and tree.

The Exp3 and MatrixUCB algorithms are provided. Not all bandits (i.e. stochastic bandit algorithms) are sound choices. Refer to "Analysis of Hannan Consistent Selection for Monte Carlo Tree Search in Simultaneous Move Games".

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