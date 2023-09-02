
# State
### Markov Decision Process
A state is a two player Markov game when we are referring to the type. An instance of that type is also called a state and it corresponds to a 'state' of the Markov game. Usually we mean an instance when using this term.
The transition function `void apply_action(Action, Action)` takes two joint player actions and mutates the state into its successor. The 'Markov' descriptor implies that these transitions are stochastic. In the interface and docs we often formulate the stochasticity using a third 'chance player' who plays an action after the players. A transition after joint actions are committed can then be thought of as an 'action', and the fixed distribution of transitions mandated by the Markov definition is then the 'strategy' of the chance player.
Payoffs for both players are only received once the state has reached terminality.
### Perfect Information and Inheritance
The assumption of perfect information means that the legal actions for both players at any state is public knowledge. We also require that the chance action is observed by both players after the transition. The full set of possible transitions after a given pair of joint actions is not known. However, players can always distinguish transition observations from one another. 
We drop the assumption of full knowledge of transitions because there may be many possibilities and calculating them non-trivial. This is certainly the case for Pokemon.
The `IsState<>` concept formalizes the general former notion of a state, while `IsPerfectInfoState<>` is the one actually required by the majority of library code.
There is template class called `PerfectInfoState<TypeList>` that has data members pertaining to the related concept. We assume that all state types derive from this class, since then they will automatically inherit members like `obs`, `prob`, and methods like `get_payoff()`, `is_terminal()`.
### Row and Column


# Concepts/Interface
### IsStateTypes
```cpp
{
    const_state.is_terminal()
} -> std::same_as<bool>;
```
Self-explanatory.
```cpp
{
    const_state.get_payoff()
} -> std::same_as<typename Types::Value>;
```
The result is only valid when the state is terminal. Otherwise, it is likely to be uninitialized.
```cpp
{
    state.apply_actions(action, action)
} -> std::same_as<void>;
```
Without the assumption of perfect info, we cannot assume that an identifying observation is returned or that the probability of the transition is known.
```cpp
{
    const_state.get_actions(actions, actions)
} -> std::same_as<void>;
```
It is assumed that calculating actions 
```cpp
{
    state.randomize_transition(device)
} -> std::same_as<void>;
```
### IsPerfectInfoStateTypes
```cpp
{
    state.obs
} -> std::same_as<typename Types::Obs &>;
{
    state.prob
} -> std::same_as<typename Types::Prob &>;
```
These members are updated after calling `apply_actions`.
The assumption of perfect info means that an observation is available to distinguish transitions. There is no academic consensus whether perfect info means that the probability of the transitions (a.k.a. the strategy of the chance player) is also known. The constraint means that it must be provided by the state classes, but this is not restrictive for two reasons. Firstly, the `prob` value is not used in the Tree Bandit algorithms since they are sample based. Secondly, user implemented classes can set this type to `bool` TODO
```cpp
{
    state.terminal
} -> std::same_as<bool &>;
{
    state.payoff
} -> std::same_as<typename Types::Value &>;
```
The terminality of the state is stored as a bool and the method `is_terminal()` is implemented as a getter; Same for `get_payoff()`.
```cpp
{
    state.get_actions()
} -> std::same_as<void>;
{
    state.row_actions
} -> std::same_as<typename Types::VectorAction &>;
{
    state.col_actions
} -> std::same_as<typename Types::VectorAction &>;
```
These are essentially the same methods and attributes from the `IsState<>` concept, except now we assume that the information which was passed as arguments to those methods is now stored as a member. This is largely a convenience, to spare the user from instantiating action vectors and values all the time. There should be very little overhead to this attachment. 
```cpp
{
    state.randomize_transition(device)
} -> std::same_as<void>;
```
**State transitions are deterministic unless this function is called**. That is, calling `state.apply_actions(row_action, col_action)` on the same state will always mutate the state in the same manner as long as the actions are the same.
This function is a no-op on deterministic states and is thus optimized away.
It is invoked by default on `state_copy` at the start of the forward phase of Tree Bandit search.
## IsChanceStateTypes
```cpp
{
    state.get_chance_actions(action, action, chance_actions)
} -> std::same_as<void>;
{
    state.apply_actions(action, action, obs)
} -> std::same_as<void>;
```
This is a state where calculating the chance actions for a given pair of player actions is possible. It is not needed for tree bandit search since that is sample based, but it is required for solver algorithms. We also assume that the probability that is observed is accurate. This implies that the sum of associated probabilities of a set of chance actions is 1 if and only if the action set encompasses all possible transitions (there is some subtlety here when the type representing probability is floating point.)

We also have an overloaded `apply_actions` method where we can specify the exact transition we want to occur by providing the same observation that the transition will produce as an argument.
It is clear how this method is required for solving since we need to be able to traverse the abstract game tree at will.

## IsSolvedStateTypes

```cpp
{
    state.get_strategies(strategy, strategy)
} -> std::same_as<void>;
```
A solved state is one where at least one pair of Nash equilibrium strategies is known and so is the associated payoff.
The strategies are provided by method above, and the Nash payoff is just the normal `payoff` member. Thus the payoff is now updated after every transition to reflect its current value at the new state.

This concept assumes that the game is constant sum. Otherwise we do not have equality of uniqueness of Nash payoffs. Even the criteria for choosing a particular Nash equilibrium is not clear in that case. 

## Subsumption
Each of these concepts assumes that the concepts before it are also satisfied. It is theoretically not necessary for a 'solved state' to also be a 'chance state' but it is natural in practice. 


# Test States

### MoldState
This is basically the simplest possible state in implementation. It's main purpose is testing and benchmarking as a control. It's only member is the `depth` parameter, which determines how many transitions are made until the state is terminal. It always has the same number of actions for both players, and so the `row_actions`, `col_actions` members are initialized in the `MoldState` constructor and never changed. Thus `get_actions` is a no-op and `apply_actions` merely decrements `depth` and checks if `depth == 0`. The payoff member is not changed or even initialized.

### OneSumMatrixGame
A basic one shot matrix game where the payoff sum is 1. It is non-stochastic, meaning there is only one possible transition for a pair of joint actions. It can be constructed by directly passing such a matrix, or passing a PRNG device and the number of rows and columns; the entries are then generated using (a copy of) the device.


### RandomTree

This class is a powerful and expressive way to create a random games for testing.
The game is well-defined, in the sense that providing the same hyper-parameters, helper functions, and PRNG device to the constructor will produce the exact same state. One advantage of this property is that arbitrarily large games can be used to test models via the `Arena` utility. Previously, random trees had to be solved and the node tree was used as the state.

```cpp
State(
    const Types::PRNG &device,
    size_t depth_bound,
    size_t rows,
    size_t cols,
    size_t transitions,
    Types::Q chance_threshold,
    int (*depth_bound_func)(State *, int),
    int (*actions_func)(State *, int),
    int (*payoff_bias_func)(State *, int))
    : device{device},
    depth_bound{depth_bound},
    rows{rows},
    cols{cols},
    transitions{transitions},
    chance_threshold{chance_threshold},
    depth_bound_func{depth_bound_func},
    actions_func{actions_func},
    payoff_bias_func{payoff_bias_func}
{
    this->init_range_actions(rows, cols);
    get_chance_strategies();
}
```
Let's look at the full list of all constructor arguments:
* device
This device is copied and stored as a member. The member is used TODO
* depth_bound
This `int` member is an upper bound on the number of transitions before the state is terminal. The default behavior is to simply decrement this by one. This produces trees with uniform length.
* rows/cols
These member determine the number of actions each player has at a given state. It is updated during transition. The action type is simply `int` and the actions are always `0, 1, ..., rows - 1` for the row player etc.
* transitions
The maximum number of legal transitions at any given state. This number is const.
* chance_threshold
Upon initalization and each transition, a new strategy for the chance player is computed. If the probability of any transition is less than this threshold, it is zeroed and the resulting distribution is re-normalized.

Function pointers:
* depth_bound_func
* actions_fun
* payoff_bias_func

### SolvedState

Any game which satisfies the `IsChanceStateTypes` concept can have it entire game tree solved using the `FullTraversal` algorithm. As the name implies, this will traverse the entire game tree and thus produces a sub-game perfect solution.
The `FullTraversal` algorithm will represent the game tree on heap using nodes.

### Arena

This is a state that leverages the generality of Pinyon to evaluate the performance of models and search algorithms using preexisting algorithms.


# Wrapping a State

