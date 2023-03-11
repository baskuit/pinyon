
# Types
Surskit attempts to codify search using four types, each 'higher' than the previous: 
 > state
model
algorithm
tree
- Each of these types is actually a generic template, e.g. `DefaultState<TypeList>`, `MonteCarloModel<State>`, `MatrixNode<Algorithm>` that accepts a specialization of the lower type.
	- The motivation for templating is to have a polymorphic interface without incurring the performance penalty of virtual table lookup.
For a basic example of this, see the 'rollout' method in the implementation of the `MonteCarloModel<State>` below.
		
			void  rollout(State &state)
			{
				while (!state.is_terminal)
				{
					const  int  row_idx = this->device.random_int(state.actions.rows);
					const  int  col_idx = this->device.random_int(state.actions.cols);
					const  typename  Types::Action  row_action = state.actions.row_actions[row_idx];
					const  typename  Types::Action  col_action = state.actions.col_actions[col_idx];
					state.apply_actions(row_action, col_action);
					state.get_actions();
				}
			}

* Each specialization is derived from another of the same generic type, all the way down to a base class for each generic type: `AbstractState<TypeList>`, `AbstractModel<State>`, `AbstractAlgorithm<Model>`, `AbstractTree<Algorithm>`.

	- For example, `MonteCarloModel<MatrixGame<...>>` is derived from `DoubleOracle<MatrixGame<...>>` which is derived from `AbstractModel<MatrixGame<...>>`.
- Each class has a nested type list called `Types`. This struct inherits from the same type list that belongs the base class. See below:

		class  Exp3p : public  _TreeBandit<Model, Exp3p<Model, _TreeBandit>>
		{
			public:
			struct  MatrixStats;
			struct  ChanceStats;
			struct  Types : _TreeBandit<Model, Exp3p<Model, _TreeBandit>>::Types
			{
				using  MatrixStats = Exp3p::MatrixStats;
				using  ChanceStats = Exp3p::ChanceStats;
			};
			struct  MatrixStats : _TreeBandit<Model, Exp3p<Model, _TreeBandit>>::MatrixStats
			{
			...
	Here we can see the pattern. We derive the `Types` struct before anything else, with the exception of forward declarations of the types that will be included in the type list from then on. Then just after the `Types` declaration we finish the definition of `MatrixStats`, `ChanceStats`.
	This convention gives us access to any type that was defined at a lower level via the idiom
						
		typename Types::T ...;
	Indeed, our definition of `MatrixStats` is not concerned with what primitive data types we chose at the typelist level to represent the gains and emperical strategies.
	
			struct  MatrixStats : _TreeBandit<Model, Exp3p<Model, _TreeBandit>>::MatrixStats
			{
				int  time = 0;
				typename  Types::VectorReal  row_gains = {0};
				typename  Types::VectorReal  col_gains = {0};
				typename  Types::VectorInt  row_visits = {0};
				typename  Types::VectorInt  col_visits = {0};
			};
	Finally, the `Types` at a base class will derive from the `Types` of the template parameter
	
		template <class  _State>
		class  AbstractMode
		{
			public:
			struct  Inference;
			struct  Types : _State::Types
			{
				using  State = _State;
				using  Inference = AbstractModel::Inference;
			};
			struct  Inference
			{
			}
		};
##  Typelist
This is the template parameter to states that determines the 'atomic' data types that are native to it's implementation and the types that are essential in what follows. It is expected that a TypeList contain the following type aliases:
* `Action` - For the contrived games that are implemented with Surskit, this is usually just `int`. However, I imagine some games may have a string data type e.g. 'exd5'.
* `Observation` - This represents our observation during a transition after committing actions. Usually equates the the type of the chance players action. This could be a string entry from a log, for example.
* `Probability` - The numeric type used to represent to known probability of a certain transition occurring. If it is known, both a `double` and a `Rational` are good candidates. If the probability is unknown, then `bool` may be a good choice, where the probability of a given transition is always `true`.
* `Real` - aka `double`. Higher precision may be desired.
* `VectorAction` - Container for list of actions.
* `VectorReal`
* `VectorInt` - These are both only expected to range over actions, for the purpose of algorithm statistics and such.
* `MatrixInt` - This should have a multiplication overload and a `get(i, j)` reference accessor.
## State
A **matrix game** is a situation where two players ( "row" and "col" ) each have a set of finite actions and each receive a payoff after simultaneously committing their actions. This can be extended to a **tree** where each node in the tree is itself a matrix game, committing causes the state to transition to the next node, and players only receive payoffs after transitioning to a terminal node.
**Stochastic** means that the transition depends not just on the joint player actions, but also on the action of a "chance" player. 
**Partially-observed** means two things: the identity and probability of aforementioned chance action may be unknown to the program. Additionally, the ex.. TODO
Despite the generality of games that can be represented by `State`, the algorithms that come implemented with Surskit are intended for the perfect-info case.

The `DefaultState` is the basest implemented State class. It comes with special `Actions` and `Transition` members that store information about the legal actions and the last transition, respectively. It is designed to represent every perfect-information. In particular, this means the the `Observation` is unique to the transition and both players actions are known.
The abstract class `SolvedState` derives from the former, and it also has members that contain a Nash equilibrium strategy for that point. Since the random tree game from the examples directory is solved recursively, it is derived from this class so that `MonteCarloPolicy` will work on it with no modification necessary.

## Model
A model provides knowledge on a state that is contained in the `Inference` struct. It's method modifies this struct in place. 
## Algorithm
## Node
