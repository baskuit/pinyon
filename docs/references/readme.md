# Papers

This directory for collecting papers that are particularly important to Pinyon.

# Abstracts

* Analysis of Hannan Consistent Selection for Monte
Carlo Tree Search in Simultaneous Move Games

>  Monte Carlo Tree Search (MCTS) has recently been successfully used to create strategies for playing imperfect-information games. Despite its popularity, there are no theoretic results that guarantee its convergence to a well-defined solution, such as Nash equilibrium, in these games. We partially fill this gap by analysing MCTS in the class of zero-sum extensive-form games with simultaneous moves but otherwise perfect information. The lack of information about the opponent's concurrent moves already causes that optimal strategies may require randomization. We present theoretic as well as empirical investigation of the speed and quality of convergence of these algorithms to the Nash equilibria. Primarily, we show that after minor technical modifications, MCTS based on any (approximately) Hannan consistent selection function always converges to an (approximate) subgame perfect Nash equilibrium. Without these modifications, Hannan consistency is not sufficient to ensure such convergence and the selection function must satisfy additional properties, which empirically hold for the most common Hannan consistent algorithms.

* Using Double-Oracle Method and Serialized Alpha-Beta Search
for Pruning in Simultaneous Move Games

> We focus on solving two-player zero-sum
extensive-form games with perfect information and
simultaneous moves. In these games, both players
fully observe the current state of the game where
they simultaneously make a move determining the
next state of the game. We solve these games by a
novel algorithm that relies on two components: (1)
it iteratively solves the games that correspond to
a single simultaneous move using a double-oracle
method, and (2) it prunes the states of the game us-
ing bounds on the sub-game values obtained by the
classical Alpha-Beta search on a serialized variant
of the game. We experimentally evaluate our
algorithm on the Goofspiel card game, a pursuit-
evasion game, and randomly generated games.
The results show that our novel algorithm typically
provides significant running-time improvements
and reduction in the number of evaluated nodes
compared to the full search algorithm.