



# Types
Surskit attempts to codify search using four types, each 'higher' than the previous: 

 > **state**
 > **model**
 > **algorithm**
 > **tree**

- Each of these types is actually a generic template (e.g. `DefaultState<TypeList>`, `MonteCarloModel<State>`, `MatrixNode<Algorithm>`) that accepts a specialization of the type just below it.
 	- The motivation for templating is to have a polymorphic interface without incurring the runtime penalty of virtual table lookup.
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

* Each class specialization is derived from another class specialization of the same generic type.

	- For example, `MonteCarloModel<MatrixGame<...>>` is derived from `DoubleOracle<MatrixGame<...>>`, which in turn is derived from `AbstractModel<MatrixGame<...>>`.

- And each type has a class which is base to all. These are  `AbstractState<TypeList>`, `AbstractModel<State>`, `AbstractAlgorithm<Model>`, `AbstractTree<Algorithm>`.

- Every class has a nested type list called `Types`. For a super class, this struct always inherits from the `Types` of the subclass. For the base classes (e.g. `AbstractModel`), `Types` will inherit from the`Types` struct of the lower template parameter class.

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
	Besides the aforementioned inheritance patter, we can also how we use Types in the scope of the class. 
	`Types` definition occurs before anything else in the class block, with the exception of forward declarations of the types that will be included therein.
	Just after `Types` closing brace we finish the definitions of the new types.

	
		template <class  _State> // underscore so using decl does not shadow
		class  AbstractModel
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
	
	Here we see the `Types` of a base class being derived from the `Types` of the template parameter
##  Typelist
This is the template parameter to states that determines the types native to the state's implementation, as well as the types that are indispensable for calculation.
More precisely, it is expected that a TypeList contain the following aliases:
* `Action` 
	For the contrived games that are implemented with Surskit, this is always just `int`. However, I imagine some games may have a string data type e.g. 'exd5'.
* `Observation` 
This represents our observation during a transition just after committing actions. This type is used by the tree structure to distinguish between 
* `Probability` 
The numeric type used to represent to known probability of a certain transition occurring. If it is known, both a `double` and a `Rational` are good candidates. If the probability is unknown, then `bool` may be a good choice, where the probability of a given transition is always `true`.
* `Real` 
Aka `double`. Higher precision may be desired.
* `VectorAction`
* `VectorReal`
* `VectorInt` 
These vectors are only expected to range over the legal actions at a given matrix node.
* `MatrixInt` 
* `MatrixReal`
The 'math' header contains a simple implementation of matrices as two dimensional std::arrays. It is really only necesssary that this type hold search statistics and assist in computation with a multiplication overload.

## State
	template <class _TypeList>
	class AbstractState {
	public:
	    struct Types : _TypeList {
	        using TypeList = _TypeList;
	    };
	    struct Transition {};
	    struct Actions {};
	};

In general, a state represents a partially-observed stochastic matrix game. A state has an `Actions` type which stores information about the legal moves of *both* players, and a `Transition` type, which stores any information observed about committing joint actions.
Note that all these structs are empty. Indeed all the abstract classes only inform what must be implemented in the derived classes 

	DefaultState<TypeList> : AbstractState<TypeList>
This is the most basic implemented class. Currently, all implemented higher classes assume that the state is derived from DefaultState. The design of this state is outline below.

	class DefaultState : public AbstractState<TypeList>
	{
	    struct Transition : AbstractState<TypeList>::Transition {
	        typename Types::Observation obs;
	        typename Types::Probability prob;
	    };

	    struct Actions : AbstractState<TypeList>::Actions {
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
The `Actions` and `Transitions` structs are defined and included as *members*. Also we have undefined methods that modify these members in place.
We also have the the payoffs for both row and column players as members. These are generally only initialized in the `apply_actions()` method, where we know were have transitioned to a terminal state. Lastly there is a boolean indicator of this fact (breaking from the defunct convention of simply letting either of the players number of actions by 0).
**Important**: We don't assume that states are constructed  with valid data, which may cause errors. You will likely have to call `get_actions()` on a state before using it.

Although it might not be obvious, the purpose of `DefaultState` is to codify a perfect information game. The search tree assumes that observations uniquely identify different transitions and it has no handling of any 'confusion' that may result from incomplete observations. 
Imperfect information games are beyond the scope of Surskit, and in fact convergence guarantees on such games would require something like Counterfactual Regret. Never the less, this class is not base because I would like to expand the functionality of Surksit to handle shakey observations and at least lookahead in some other imperfect info settings.

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

A model provides knowledge on a state that is contained in the `Inference` struct.

	DoubleOracleModel<State> : AbstractModel<State>

Conventional models like heuristic-based value estimation, monte carlo, and neural networks all sit under the umbrella of the DoubleOracle model, which simply provides a value and strategy/policy estimate for both players.

	MonteCarloModel<State> : DoubleOracleModel<State>
A universal model that is able to provide unbiased estimates for any perfect-information state. Its policy estimate is the uniform distribution over legal actions.

	SolvedMonteCarloModel<State> : DoubleOracleModel<State>

The state template class must be derived from `SolvedState`. That is because this model works just like a normal Monte Carlo model, only that it mimics a strong prior policy by modifying the de facto solutions via the Lp norm. This model is intended to test the use of policy priors in search algorithms.
Indeed, just as MatrixUCB is a natural generalization of the more well-known UCT algorithm, we can also naturally generalize the PUCT algorithm to use priors in the SM setting. 

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

An algorithm is a way to use a model's inference to produce solutions for a state. 
Besides any methods that an algorithm may use in its operation, there must be `MatrixNode` and `ChanceNode` structs which encapsulate any information the algorithm will need to store at matrix and chance nodes, respectively.

The MatrixUCB and Exp3p algorithms are both instances of a Multi-Armed-Bandit solution being applied to a sequential decision making problem. For this reason, it was natrual to make both of them derived from a common base. Take a look at the `playout ()` function that handles the classic MCTS style tree expansion:

	    MatrixNode<Algorithm> *_playout(
	        typename Types::State &state,
	        typename Types::Model &model,
	        MatrixNode<Algorithm> *matrix_node)
	    {
	        if (!matrix_node->is_terminal) {
	            if (matrix_node->is_expanded) {
	                typename Types::Outcome outcome;
	                this->select(matrix_node, outcome);

	                typename Types::Action row_action = matrix_node->actions.row_actions[outcome.row_idx];
	                typename Types::Action col_action = matrix_node->actions.col_actions[outcome.col_idx];
	                state.apply_actions(row_action, col_action);

	                ChanceNode<Algorithm> *chance_node = matrix_node->access(outcome.row_idx, outcome.col_idx);
	                MatrixNode<Algorithm> *matrix_node_next = chance_node->access(state.transition);

	                MatrixNode<Algorithm> *matrix_node_leaf = this->_playout(state, model, matrix_node_next);

	                outcome.row_value = matrix_node_leaf->inference.row_value;
	                outcome.col_value = matrix_node_leaf->inference.col_value;
	                this->update_matrix_node(matrix_node, outcome);
	                this->update_chance_node(chance_node, outcome);
	                return matrix_node_leaf;
	            }
	            else
	            {
	                this->expand(state, model, matrix_node);
	                return matrix_node;
	            }
	        }
	        else
	        {
	            return matrix_node;
	        }
	    }
The difference between Exp3p and MatrxUCB arises in how they select actions.
But there is another catch. We could very easily modify the above function to be multithreaded with just two `lock` and two `unlock` calls while a thread is accessing the stats need to traverse down the tree. In fact, I've implemented two different naive threading schemes: one where each matrix node contains a mutex, and one where each matrix has an index for a shared pool of mutexes.
Surskit allows the user to mix and match any bandit algorithm with any single or multi threaded traversal scheme. This is achieved using Curiously Recurring Template Pattern, where 

## Tree

	template <class _Algorithm>
	class AbstractNode {
	public:
	    struct Types : _Algorithm::Types {
	        using Algorithm = _Algorithm;
	    };
	};
There is currently only one implementation of a search tree, via `MatrixNode` and `ChanceNode`.

	MatrixNode<Algorithm> : AbstractTree<Algorithm>

A matrix node represents a decision point by both players. It contains all the information provided by the model and all the statistics necessary for it's base algorithm to select actions and traverse further down the tree.

	ChanceNode<Algorithm> : AbstractTree<Algorithm>

A chance node represents all the possible transition that are possible once a given pair of joint actions has been committed.

See below for a detailed look at these members and functions, starting with the `MatrixNode`.
	
	class MatrixNode : public AbstractNode<Algorithm>
	{
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

The tree structure is provided by the four pointer members. Rather than storing a vector of pointers that refer to the children of a node, we use a linked list implementation. Only the first child is stored in the parent, and all of the child's siblings are accessible via `next`. More precisely, the children of `matrix_node` would be `matrix_node->child`, `matrix_node->child->next`, `matrix_node->child->next->next` and so on.

Note that the parent and children of a `MatrixNode` are `ChanceNodes`, while the siblings prev and next are of the same type. The same is true for the pointer members of a `ChanceNode`.

As we are traversing the tree, we try to only use information that is stored in the nodes, querying the state only when necessary: to expand a new matrix node. To this end, the member `is_terminal` and `actions` are just copies of their state object counterparts.

We store a `Transition` object (recall this is just an `Observation` key and a `Probability` value) at the matrix node which is used by the chance node parent to navigate transitions.

Lastly, we store a stats object which is defined as a member of the template `Algorithm` parameter.

Now the `ChanceNode.

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

The chance node stores considerably less data since no decisions are made here. Chance node's function is to point us the the correct matrix node given a state's particular transition. Similarly to how a matrix node will store the transition object that 'led to it', a chance node will also store the indices of the actions of the row and column player that make up the joint action. The use of these identifiers is evident in the implemntations of both the access functions.

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
            if (current->transition.obs == transition.obs)
            {
                return current;
            }
            current = current->next;
        }
        MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>(this, previous, transition);
        previous->next = child;
        return child;
    };

The last thing to cover is resource management. A node owns it children but not it's siblings. Destructing a node will thus delete all its children  and remove it from it parents linked list of children.

	template <typename Algorithm>
	MatrixNode<Algorithm>::~MatrixNode() {
	    while (this->child != nullptr) {
	        ChanceNode<Algorithm> *victim = this->child;
	        this->child = this->child->next;
	        delete victim;
	    }
	    if (this->prev != nullptr) {
	        this->prev->next = this->next;
	    }
	    else if (this->parent != nullptr) {
	        this->parent->child = this->next;
	    }
	}

	template <typename Algorithm>
	ChanceNode<Algorithm>::~ChanceNode() {
	    while (this->child != nullptr) {
	        MatrixNode<Algorithm> *victim = this->child;
	        this->child = this->child->next;
	        delete victim;
	    }
	    if (this->prev != nullptr) {
	        this->prev->next = this->next;
	    }
	    else if (this->parent != nullptr) {
	        this->parent->child = this->next;
	    }
	};

Thus a simple way to ensure that everything is cleaned up is to instantiate the root node at the stack level, so that it and all of it's children will be destructed when it leaves scope.

	void scoped_search_example () {
		MatrixNode<Algorithm> root;
		session.search(10000, state, model, root);
		// tree will be created and destroyed within the execution of this function.
	}

The tree does not currently support more advanced memory management like pruning or hash tables.
