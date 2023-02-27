
Surskit is designed to enable simple implementation and testing of search algorithms on a wide class of simultaneous move games. The generality of these terms is detailed below.

### `State` represents a partially-observed, stochastic matrix-tree game.
A **matrix game** is a situation where two players ( "row" and "col" ) each have a set of finite actions and each receive a payoff after simultaneously committing their actions. This can be extended to a **tree** where each node in the tree is itself a matrix game, committing causes the state to transition to the next node, and players only receive payoffs after transitioning to a terminal node.
**Stochastic** means that the transition depends not just on the joint player actions, but also on the action of a "chance" player. 
**Partially-observed** means two things: the identity and probability of aforementioned chance action may be unknown to the program. Additionally, the ex.. TODO
Despite the generality of games that can be represented by `State`, the algorithms that come implemented with Surskit are intended for the perfect-info case.

### Templates
