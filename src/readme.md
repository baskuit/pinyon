

# Types
Surskit attempts to codify search using four types, each 'higher' than the previous: 

 > **state**
 > **model**
 > **algorithm**
 > **tree**

- Each of these types is actually a generic template (e.g. `DefaultState<TypeList>`, `MonteCarloModel<State>`, `MatrixNode<Algorithm>`) that accepts a specialization of the type just below it.
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

* Each class specialization is derived from another of the same generic type.

	- For example, `MonteCarloModel<MatrixGame<...>>` is derived from `DoubleOracle<MatrixGame<...>>`, which in turn is derived from `AbstractModel<MatrixGame<...>>`.

- And each type has a class which is base to all:  `AbstractState<TypeList>`, `AbstractModel<State>`, `AbstractAlgorithm<Model>`, `AbstractTree<Algorithm>`.

- Every class has a nested type list called `Types`. For a derived class, this struct always inherits from the `Types` struct of the base class. For the most basic classes (e.g. `AbstractModel`), `Types` will inherit from the the `Types` struct of the template class.

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
	Here we can see the pattern by which each `Types` struct is defined. This definition occurs before anything else in the class block, with the exception of forward declarations of the types that will be included `Types` from then on.
	Just after this we finish defining the foward declarations.
	
	Here we see the `Types` of a base class being derived from the `Types` of the template parameter
	
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
This is the template parameter to states that determines the 'atomic' data types that are native to the state's implementation, as well as the types that are elementary to any algorithm.
More precisely, it is expected that a TypeList contain the following aliases:
* `Action` - For the contrived games that are implemented with Surskit, this is usually just `int`. However, I imagine some games may have a string data type e.g. 'exd5'.
* `Observation` - This represents our observation during a transition after committing actions. Usually equates the the type of the chance players action. This could be a string entry from a log, for example.
* `Probability` - The numeric type used to represent to known probability of a certain transition occurring. If it is known, both a `double` and a `Rational` are good candidates. If the probability is unknown, then `bool` may be a good choice, where the probability of a given transition is always `true`.
* `Real` - aka `double`. Higher precision may be desired.
* `VectorAction` - Container for list of actions.
* `VectorReal`
* `VectorInt` - These are both only expected to range over actions, for the purpose of algorithm statistics and such.
* `MatrixInt` 
* `MatrixReal`
	 This should have a multiplication overload and a `get(i, j)` reference accessor.
## State
	AbstractState

In general, a state represents a partially-observed stochastic matrix game. The information concerning the legal actions for a both players is contained within the `Actions` struct. The `Transition` struct contains information about the transition of a state after joint actions have been committed.

	DefaultState : AbstractState
This is the most basic implemented class, and all are other states intended to be used with the provided algorithms are intended to be derived from this class. In this regard, `AbstractState` only exists because the functionality of Surskit may be eventually extended to more general games.  The design of this state is outline below.

	class DefaultState : public AbstractState<TypeList>
	{
	    struct Transition : AbstractState<TypeList>::Transition
	    {
	        typename Types::Observation obs;
	        typename Types::Probability prob;
	    };

	    struct Actions : AbstractState<TypeList>::Actions
	    {
	        typename Types::VectorAction row_actions;
	        typename Types::VectorAction col_actions;
	        int rows;
	        int cols;
	    };

	    bool is_terminal = false;
	    typename Types::Real row_payoff, col_payoff;
	    Transition transition;
	    Actions actions;

	    void get_actions();
	    void apply_actions(
	        typename Types::Action row_action,
	        typename Types::Action col_action);
	};
In particular, we have *members* of type `Actions`, `Transition` that are modified *in-place* by the methods ` get_actions()` and `apply_actions()`. 
The actions data is straightforward, but note that we also store the number of actions as `int`.
The transitions data consists of `obs`, which is essentially the chance action. The default tree implementation will use this member to identify the correct branch of a chance node.
**Important**: We don't assume that states are constructed  with valid data, which may cause errors. You will likely have to call `get_actions()` on a state before using it.

	SolvedState<TypeList> : DefaultState<TypeList>
This class is used to represent a state whose Nash equilibrium strategies and payoffs are know a priori.
It has additional members `typename Types::VectorReal row_strategy, col_strategy;`, and the `row_payoff, col_payoff` members from `DefaultState` are assumed to be valid even when the state is not terminal.

	StateChance<TypeList> : DefaultState<TypeList>
This class is for states that provide a means for controlling the behaviour of the chance player, meaning that we can force the state to transition in any (valid) way. This is codified with the undefined  overload method

	    void apply_actions(
	        typename Types::Action row_action,
	        typename Types::Action col_action,
	        typename Types::Observation chance_action);
There are currently no provided algorithms that exploit this functionality.

## Model

	AbstractModel<State>

A model provides knowledge on a state that is contained in the `Inference` struct.

	DoubleOracleModel<State> : AbstractModel<State>

Conventional models like heuristic-based value estimation, monte carlo, and neural networks all sit under the umbrella of the DoubleOracle model, which simply provides a value and strategy/policy estimate for both players.

	MonteCarloModel : DoubleOracleModel
A universal model that is able to provide unbiased estimates for any perfect-information state. Its policy estimate is the uniform distribution over legal actions.

	SolvedMonteCarloModel<State> : DoubleOracleModel<State>

The state template class must be derived from `SolvedState`. That is because this model works just like a normal Monte Carlo model, only it provides the *p normalized* de factor solutions as an approximation of expert policy inference. This model is intended to test the use of policy priors in search algorithms.
Indeed, just as MatrixUCB is a natural generalization of the more well-known UCT algorithm, we can also naturally generalize the PUCT algorithm to use priors in the SM setting. 

## Algorithm

	AbstractAlgorithm<Model>

An algorithm is a way to use a model's inference to produce solutions for a state. 
Besides any methods that an algorithm may use in its operation, there must be `MatrixNode` and `ChanceNode` structs which encapsulate any information the algorithm will need to store at matrix and chance nodes, respectively.

	BanditAlgorithm<Model, TreeBanditAlgorithm> : AbstractAlgorithm<Model>

The aforementioned MatrixUCB and Exp3p algorithms are examples of *bandit algorithms* applied to sequential decision making. They grow the tree in the same way, the only thing that differs is how they select which nodes to expand next.

Additionally, the operation of growing the tree has been made multithreaded via two different (naive) schemes.
So we can perform a single threaded Exp3p search, or a multi threaded MatrixUCB search, etc. The tree growing schemes and bandit algorithms are independent. 
Thus why the also accepts a template algorithm class called `TreeBanditAlgorithm`
TODO
The algorithm stuff uses CRTP to add single-threaded/multi-threaded functionality to the bandit algorithms with just a simple template parameter swap. Hard to explain though.



## Tree

	AbstractTree<Algorithm>
There is currently only one implementation of a search tree, via `MatrixNode` and `ChanceNode`.

	MatrixNode<Algorithm> : AbstractTree<Algorithm>

sdfsdf
	
	ChanceNode<Algorithm> : AbstractTree<Algorithm>
TODO: Ownership, the nodes own their children. Matirix node carry enough info about the state 'at that point' to make an decisions.
We have an `Actions` object stored there so we can transition properly.
We have a `Transition` stored there so the chance node parent can tell what led to that matrix node
We have a `Inference` object that contains the model's inference when we expanded the node.
We have a  
	
	class MatrixNode : public AbstractNode<Algorithm>
	{
	public:
	    struct Types : AbstractNode<Algorithm>::Types
	    {
	    };

	    ChanceNode<Algorithm> *parent = nullptr;
	    ChanceNode<Algorithm> *child = nullptr;
	    MatrixNode<Algorithm> *prev = nullptr;
	    MatrixNode<Algorithm> *next = nullptr;

	    bool is_terminal = false;
	    bool is_expanded = false;

	    typename Types::Actions actions;
	    typename Types::Transition transition;
	    typename Types::Inference inference;
	    typename Types::MatrixStats stats;
