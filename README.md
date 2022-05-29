# Surskit

A general matrix tree game program.


## Class Hierarchy

It is probably useful to understand the classes defined in this project and how the relate to each other.


### prng
A pseudo random generator class for use in all other classes, for reproducibility.

* `float uniform ()`
	*	Uniform float between 0 and 1.
* `std::mt19937::result_type  get_seed ()`
	* Used to instantiate an identical probability outcome. Thread safety remains a problem.

## State
The game in question, in the language of a markov decision process

* `PairActions Actions ()`
	* The action sets of both players returned as *PairActions* struct which is just two C style arrays of the *Action* type.
	* If your game implementation needs expensive logic to compute the actions then you may want the interface to be lazy.
	* Currently *Action* is just an int. The storage benefits of this are appreciated since we store actions in the tree, as means of identifying nodes since their positions in the linked list may change after a node is pruned.
	
a State is terminal if either Action set is empty, i.e. *rows* or *cols* is zero.
* `StateTransitionData Transition ()`
	*	*StateTransitionData* is a tuple of *Hash* (currently an alias for int) and a *Rational* transition probability. The transition of a state is assumed to be stochastic and thus is identified by its hash and occurs with known rational probability. 
	*	It is not assumed that you have any control over the transition outcomes, but there may or may not be an override defined that accepts the hash key as input.


* `StateInfo info`
	*  This is where we store most of the salient information about the state. By default there are no members to this struct but there is a definition of the *SolvedStateInfo* type which at contains the Nash strategies and payoff, as well as helper members: rows, cols, terminal.

## Model
This is the next abstraction. Before we think about search algorithms, we need a way to evaluate states. This evaluation usually takes the form of a value estimate and a action selection policy for both players. There may also be other information, such as WLD estimates, so we define a base *InferenceData* class.
Models only have one method, although there will likely be multi threaded tree search and batch GPU inference methods.
* `InferenceData inference ()*`

Monte carlo evaluation is encapsulated here, and a default implementation with rollout and uniform policy is provided.
Note there are two value estimates. In certain algorithms we need the constant sum assumption, but our implementation sacrifices an extra float per node in order to allow for dual model tree search and 'double-loss' outcomes to avoid loops.

## Node 