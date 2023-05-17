




# Types
Surskit attempts to codify search using four types, each 'higher' than the previous: 

 > State
 > Model
 > Algorithm
 > Tree

- Each of these types is actually a generic template (e.g. `DefaultState<TypeList>`, `MonteCarloModel<State>`, `MatrixNode<Algorithm>`) that accepts a specialization of the type just below it.
 	- The motivation for templating is to design a polymorphic interface without incurring the runtime penalty of virtual table lookup.
For a basic example of this, see the `rollout` method in the implementation of`MonteCarloModel<State>` below:
		
			void rollout(State &state) {
				while (!state.is_terminal) {
					const int row_idx = this->device.random_int(state.row_actions.size());
					const int col_idx = this->device.random_int(state.col_actions.size());
					const typename Types::Action row_action = state.row_actions[row_idx];
					const typename Types::Action col_action = state.col_actions[col_idx];
					state.apply_actions(row_action, col_action);
					state.get_actions();
				}
			}

* Each class specialization is derived from another class specialization of the same generic type.

	- For example, `MonteCarloModel<MatrixGame<...>>` is derived from `DoubleOracle<MatrixGame<...>>`, which in turn is derived from `AbstractModel<MatrixGame<...>>`.

- And each type has a class which is base to all. These are `State<TypeList>`, `AbstractModel<State>`, `AbstractAlgorithm<Model>`, `AbstractTree<Algorithm>`.

- Every class has a nested type list called `Types`. For a super class, this struct always inherits from the `Types` of the subclass. For the base classes (e.g. `AbstractModel`), `Types` will inherit from the`Types` of the lower template parameter class, except `State<TypeList>`, whose `Types` inherits from the template parameter directly.


	Besides the aforementioned inheritance, the below also illustrates the pattern for supplying `Types` with more aliases within the scope of the class. 
	
		class Exp3p : public _TreeBandit<Model, Exp3p<Model, _TreeBandit>>
		{
			public:
			struct MatrixStats;
			struct ChanceStats;
			struct Types : _TreeBandit<Model, Exp3p<Model, _TreeBandit>>::Types
			{
				using MatrixStats = Exp3p::MatrixStats;
				using ChanceStats = Exp3p::ChanceStats;
			};
			struct MatrixStats : _TreeBandit<Model, Exp3p<Model, _TreeBandit>>::MatrixStats
			{
				//...
			}
		};
	This pattern improves readability by confining this boilerplate to the top of the class block.
	

##  Typelist
This is the template parameter to states that determines the types native to the state's implementation, as well as the types that are indispensable for calculation up the hierarchy.
More precisely, it is expected that a TypeList contain the following aliases:

* `Action` 
For the contrived games that are implemented with Surskit, this is always just `int`. However some games may have a string data type e.g. 'exd5' instead.
* `Observation` 
This represents the observation during a transition just after committing actions. This type is used by the tree structure to distinguish between different outcomes. 
* `Probability` 
The numeric type used to represent the probability of a given transition occurring. If it is known, both `double` and `Rational` are good candidates. However many games do not implement or reveal this number. In that case, `bool` may be a good choice to indicate this, where the probability of a given transition is always `true`.
* `Real` 
Aka `double`. More precision may be desired.
* `VectorAction`
* `VectorReal`
* `VectorInt` 
These vectors are only expected to range over the legal actions at a given matrix node. 
As such the primary state implementation uses `std::array<Action, MAX_ACTIONS>` etc.
 Some games may have a large upper bound on the number of actions, and storing them in standard array may not be feasible.  In this case, the use of `std::vector<Action>` is encouraged. This is especially true if using MatrixUCB or a variant, since then the use of arrays would require `MAX_ACTIONS * MAX_ACTIONS` elements.

* `MatrixInt` 
* `MatrixReal`
The 'math' header contains a simple implementation of matrices via standard arrays and standard vectors.

## State
	template <class _TypeList>
	class State {
	public:
	    struct Types : _TypeList {
	        using TypeList = _TypeList;
	    };
	    struct Transition {};
	    struct Actions {};
	};

In general, a state represents a partially-observed, stochastic, two-player matrix game. A state has an `Actions` type (not to be confused with `Action`) which stores information about the legal moves of *both* players, and a `Transition` type, which stores any information observed from the state after committing joint actions. This terminology mirrors that of a Markov Decision Process.
Note that all these structs are empty. Indeed the abstract classes only inform the user what must be implemented in the derived classes. 

### `DefaultState<TypeList> : State<TypeList>`

Currently, all implemented higher classes assume that the state is derived from `DefaultState`. The design of this state is outlined below.

	class DefaultState : public State<TypeList>
	{
	    struct Transition : State<TypeList>::Transition {
	        typename Types::Observation obs;
	        typename Types::Probability prob;
	    };

	    struct Actions : State<TypeList>::Actions {
	        typename Types::VectorAction row_actions;
	        typename Types::VectorAction col_actions;
	        int rows;
	        int cols;
	    };
	    typename Types::Real row_payoff, col_payoff;
	    bool is_terminal = false;
	    Transition transition;
	    Actions actions;

	    void get_actions();
	    void apply_actions(
	        typename Types::Action row_action,
	        typename Types::Action col_action);
	};
The `Actions` and `Transitions` structs are implemented and included as *members*. We populate these members with methods `get_actions` and `apply_actions` that modify them in place.
The payoffs for both row and column players are members. These payoffs are assumed only to be  initialized in the `apply_actions` method, after the state has transitioned to terminal. No reward is received for a non-terminal transition.
Lastly there is a boolean indicator of whether the state is terminal. This is also updated in `apply_actions`.
It is not assumed that states are constructed with valid actions, payoff, or transition data in place. This is because this information may be costly to calculate. The user may have to call `get_actions()` on a state before using it. 
A state must also be copyable. This is because the state will be 'rolled out' (mutated) during search, so these operations must be performed on a copy to keep the original state unmodified. 

`DefaultState` has a smaller scope of representation than `State`. Its purpose is to codify a *perfect information* game. 
Thus the search tree assumes that the `Observation` uniquely identifies different transitions and has no handling of any 'mix-up' that may result from incomplete observations.
Imperfect information games are beyond the scope of vanilla Surskit, and in fact convergence guarantees on such games would require something like Counterfactual Regret. Nevertheless, `DefaultState` is not base because the user may find that the look-ahead functionality of Surskit is useful in some imperfect info contexts.

###	`SolvedState<TypeList> : DefaultState<TypeList>`
This class is used to represent a state whose Nash equilibrium strategies and payoffs are known *a priori*.
It has additional  members  `row_strategy, col_strategy`, and the `row_payoff, col_payoff` members inherited from `DefaultState` are assumed to be initialized and valid even when the state is not terminal.



### `StateChance<TypeList> : DefaultState<TypeList>`

This class is for states that provide a means for controlling the actions of the 'chance player', meaning that the user can dictate how a state transitions. This is codified with the undefined  overload method

	    void apply_actions(
	        typename Types::Action row_action,
	        typename Types::Action col_action,
	        typename Types::Observation chance_action);
	        
There are currently no provided algorithms that exploit this functionality, but the potential benefits are clear.

## Model

	template <class _State>
	class AbstractModel {
	public:
	    struct Inference;
	    struct Types : _State::Types {
	        using State = _State;
	        using Inference = AbstractModel::Inference;
	    };
	    struct Inference {};
	    void get_inference(
	        typename Types::State &state,
	        typename Types::Inference &inference);
	};

A model provides knowledge about a state which is contained in the `Inference` struct.

### `DoubleOracleModel<State> : AbstractModel<State>`

Conventional models like heuristic-based value estimation, monte carlo, and neural networks all sit under the umbrella of the double oracle model, which provides a value and policy estimate for both players.

### `MonteCarloModel<State> : DoubleOracleModel<State>`

A universal model that is able to provide unbiased estimates for any perfect-information state. Its value estimates are the payoff of a state after a random rollout and its policy estimates are the uniform distribution over legal actions.

### `SolvedMonteCarloModel<State> : MonteCarloModel<State>`

The state template class must be derived from `SolvedState`. That is because this model works just like a normal Monte Carlo model, only that it simulates a strong policy estimate by modifying the de facto solutions via the Lp norm. This model is intended to test the use of policy priors in search algorithms.

## Algorithm

	template <class _Model>
	class AbstractAlgorithm {
	public:
	    struct MatrixStats {};
	    struct ChanceStats {};
	    struct Types : _Model::Types {
	        using Model = _Model;
	        using MatrixStats = AbstractAlgorithm::MatrixStats;
	        using ChanceStats = AbstractAlgorithm::ChanceStats;
	    };
	};

An algorithm determines how a tree is expanded and produces value/policy solutions for a state.
Besides any methods that an algorithm may use in its operation, it also provides `MatrixStats` and `ChanceStats` structs while will be stored inside each matrix node and chance node, respectively.  These structs encapsulate the information the algorithm needs in its tree expansion and calculations.

### MCTS

The MatrixUCB and Exp3p algorithms are both instances of a [Monte Carlo Tree Search](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search), where a tree is iteratively expanded via iterations. In the following, it is assumed that the reader is familiar with the fundamentals of this process.

The usual formula for determining what actions to select during a playout is known as Upper Confidence Bounds. The application of this selection process to a tree structure is sometimes known as Upper Confidence Trees. 

It is well known that this formula, when used to select the actions of the row and column players independently, fails to produce Nash equilibrium strategies. 
The reason for this is best explained with some math terminology. UCB is a solution to a class of problems called [Multi Armed Bandits](https://en.wikipedia.org/wiki/Multi-armed_bandit). It is a principled way of selecting actions during iterations. However, UCB is a solution to the *stochastic* bandits problem, where the quality of an action is more or less fixed. This assumption does not hold in simultaneous move games because the quality of an action also depends on the selection of the opponent.

The two algorithms provided with Surskit amend this by using a more suitable bandit algorithm:

* Exp3p is a solution to the *adversarial* bandit problem. The selection process is robust to the fact that the quality of an action depends on the selection for the other player.

* MatrixUCB recovers the stochastic assumption by making the selection over *joint  actions* for both players.

### `TreeBandit : public AbstractAlgorithm<Model>`

The class encapsulates the iterative selection, expansion and back-propagation process that is common to vanilla MTCS, Exp3p, and MatrixUCB. It introduces a helper struct called `Outcome` which stores the outcome of the bandit selection process at each node that is visited in the playout. This information consists of: the actions selected, the sampled probability of selecting those actions, and the reward received (the reward being the model's value estimate for the leaf node at the end of the playout.)
The user interfaces with this class with its methods `run`, which asks for a specific number  of iterations to perform, `run_for_duration` which performs iterations until the time is up, and `get_strategies`, which gives the 'final answer' for the root node using the accumulated stats in the search tree.

This class is actually incomplete, and has three derivations. 

* `TreeBandit : public TreeBandit<Model, Algorithm>`
Single threaded

* `TreeBanditThreaded : public TreeBandit<Model, Algorithm>`
A multi-threaded version that stores a mutex in each matrix node. This has lower contention but higher memory cost.

* `TreeBanditPool : public TreeBandit<Model, Algorithm>`
A mult-threaded version that uses a pool of mutexes for the entire tree. Each matrix node is assigned an index that speficies a mutex. This has lower memory cost but higher contention.

These classes differ only by their implementation of the playout function and data they have to add to the matrix stats for that purpose.
Each of these templates accepts a model specialization, of course, but also an Algorithm. This is where we specify Exp3p, MatrixUCB, or any other bandit algorithm.

 See the single threaded playout function below:


    MatrixNode<Algorithm> *playout(
        prng &device,
        typename Types::State &state,
        typename Types::Model &model,
        MatrixNode<Algorithm> *matrix_node) 
	{
        if (!matrix_node->is_terminal) {
            if (matrix_node->is_expanded) {
                typename Types::Outcome outcome;

                this->_select(device, matrix_node, outcome);

                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

                MatrixNode<Algorithm> *matrix_node_leaf = this->playout(device, state, model, matrix_node_next);

                outcome.row_value = matrix_node_leaf->inference.row_value;
                outcome.col_value = matrix_node_leaf->inference.col_value;
                this->_update_matrix_node(matrix_node, outcome);
                this->_update_chance_node(chance_node, outcome);
                return matrix_node_leaf;
            }
            else {
                this->_expand(state, model, matrix_node);
                return matrix_node;
            }
        }
        else {
            return matrix_node;
        }
    }

The methods `expand`, `update_matrix_node`, `update_chance_node`, and `select` are members of  the `TreeBandit` class, but `TreeBandit` is actually a *CRTP base class*. The methods are implemented in the particular bandit algorithm class like so:

	// from TreeBandit
    void update_matrix_node(
        MatrixNode<Algorithm> *matrix_node,
        Outcome &outcome) {
        return static_cast<Algorithm *>(this)->_update_matrix_node(
            matrix_node,
            outcome);
    }

and 

	// from Exp3p
    void _update_chance_node(
        ChanceNode<Exp3p> *chance_node,
        typename Types::Outcome &outcome)
    {
        chance_node->stats.row_value_total += outcome.row_value;
        chance_node->stats.col_value_total += outcome.col_value;
        chance_node->stats.visits += 1;
    }

The result of this inheritance scheme is quite powerful. The user can write their own bandit algorithm by simply implementing `select`, `expand`, `update_matrix_node`, `update_chance_node`. The bandit algorithm is then compatible with the single and multi threaded search processes, and can be initialized with just:

	CustomBandit<Model, TreeBanditThreaded> session(device);
	session.run(iterations, state, model, root);



## Tree

	template <class _Algorithm>
	class AbstractNode {
	public:
	    struct Types : _Algorithm::Types {
	        using Algorithm = _Algorithm;
	    };
	};
There is currently only one implementation of a search tree, via `MatrixNode` and `ChanceNode`.

A matrix node represents a decision point by both players. It contains all the information provided by the model and all the statistics necessary for it's base algorithm to select actions and traverse further down the tree.

A chance node represents all the possible transition that are possible once a given pair of joint actions has been committed.

### `MatrixNode : public AbstractNode<Algorithm>`

	class MatrixNode : public AbstractNode<Algorithm> {
	public:
	    struct Types : AbstractNode<Algorithm>::Types {};

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
	    
	    ChanceNode<Algorithm> *access(int row_idx, int col_idx);
	};

The tree structure is provided by the four pointer members. Rather than storing a vector of pointers that refer to the children of a node, we use a linked list. Only the first child is stored in the parent, and all of the child's siblings are accessible via `next`. Explicitly, the children of `matrix_node` would be `matrix_node->child`, `matrix_node->child->next`, `matrix_node->child->next->next` and so on.

Note that the parent and children of a `MatrixNode` are `ChanceNodes`, while the siblings prev and next are of the same type. The analogous is true for the pointer members of a `ChanceNode`.

As the tree is being traversed,  the search uses only information that is stored in the nodes. It will query the state and model only when necessary, during the expansion of a new matrix node.
Thus we store a copy of `Actions`, `is_terminal`, and `inference` in each matrix node.

A `Transition` object (recall this is just an `Observation` key and a `Probability` value) is stored in each matrix node. This is used by the chance node parent to match the observed transition with the correct child.

Lastly, a `MatrixStats` object is stored, which is defined inside the template `Algorithm` parameter.

### `ChanceNode : public AbstractNode<Algorithm>`

	template <typename Algorithm>
	class ChanceNode : public AbstractNode<Algorithm>
	{
	public:
	    struct Types : AbstractNode<Algorithm>::Types {};

	    MatrixNode<Algorithm> *parent = nullptr;
	    MatrixNode<Algorithm> *child = nullptr;
	    ChanceNode<Algorithm> *prev = nullptr;
	    ChanceNode<Algorithm> *next = nullptr;

	    int row_idx;
	    int col_idx;

	    typename Types::ChanceStats stats;

The chance node stores considerably less data since no decisions are made here. Its primary purpose is to point us the the correct matrix node given a state's particular transition. 

Similarly to how a matrix node will store the transition object that 'led to it', a chance node will also store the indices of the actions of the row and column player that make up the joint action. The use of these identifiers is evident in the implementations of both the access functions.

    ChanceNode<Algorithm> *access(int row_idx, int col_idx)
    {
        if (this->child == nullptr)
        {
            this->child = new ChanceNode<Algorithm>(this, nullptr, row_idx, col_idx);
            return this->child;
        }
        ChanceNode<Algorithm> *current = this->child;
        ChanceNode<Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->row_idx == row_idx && current->col_idx == col_idx)
            {
                return current;
            }
            current = current->next;
        }
        ChanceNode<Algorithm> *child = new ChanceNode<Algorithm>(this, previous, row_idx, col_idx);
        previous->next = child;
        return child;
    };
and 

    MatrixNode<Algorithm> *access(typename Types::Transition &transition)
    {
        if (this->child == nullptr)
        {
            MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>(this, nullptr, transition);
            this->child = child;
            return child;
        }
        MatrixNode<Algorithm> *current = this->child;
        MatrixNode<Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->obs == transition.obs)
            {
                return current;
            }
            current = current->next;
        }
        MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>(this, previous, transition);
        previous->next = child;
        return child;
    };

The last thing to cover is resource management. 
A node owns its children but not its siblings. 
Destructing a node will thus delete all its children  and remove it from it parents linked list of children. See below:

	template <typename Algorithm>
	MatrixNode<Algorithm>::~MatrixNode()
	{
	    while (this->child != nullptr)
	    {
	        ChanceNode<Algorithm> *victim = this->child;
	        this->child = this->child->next;
	        delete victim;
	    }
	}

	template <typename Algorithm>
	ChanceNode<Algorithm>::~ChanceNode()
	{
	    while (this->child != nullptr)
	    {
	        MatrixNode<Algorithm> *victim = this->child;
	        this->child = this->child->next;
	        delete victim;
	    }
	};

Thus a simple way to ensure that everything is cleaned up is to instantiate the root node at the stack level, so that it and all of it's children will be destructed when it leaves scope.

	void scoped_search_example () {
		MatrixNode<Algorithm> root;
		session.search(10000, state, model, root);
		// tree will be created and destroyed within the execution of this function.
	}

The tree does not currently support more advanced memory management like pruning or hash tables.

