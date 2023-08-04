
> **Note:** In what follows, 'MatrixNode' and 'ChanceNode' are used to denote *any* implementation of the tree, even though there is a specific implementation with the exact same name for the nodes. 

# Tree Structure

Because the tree structure is intended to represent *stochastic* games, we use two different kinds of nodes: `MatrixNodes` and `ChanceNodes`.
There are multiple implementations of the tree structure, mostly for performance considerations. They are all compatible with any state, model, and algorithm. Both the solver and tree-bandit style algorithms can use the same node implementations.

### Ownership

A node owns its children. If it is deleted, then so are they.

## Relation Between State and Tree

# MatrixNode

This represents a decision point for both players. It contains enough information that it can act as a stand-in for the state during the forward phase of tree-bandit.

The tree search mutates the state as it progresses deeper into the tree.

It is expected that when a state is mutated as the search travels along a path in the tree, its condition is unique. That is, the state is exactly the same as any other time the search reached that node.

### Interface

```cpp
	MatrixNode();
	// a matrix node must have a default constructor, for creating a new root
	void expand(const typename Types::State&);
	// marks node as expanded and sets the row and column player actions
	bool is_terminal() const;
	bool is_expanded() const;
	void apply_actions(typename Types::State&, const ActionIndex, const ActionIndex) const;
	// retrieves the actions and applies them to the state
	typename Types::Action get_row_action(const ActionIndex) const;
	typename Types::Action get_col_action(const ActionIndex) const;
	void set_terminal ();
	void set_terminal (const bool);
	ChanceNode<Algorithm> *access(const ActionIndex, const ActionIndex);
```

All but the last method are self explanatory. 

# ChanceNode

A chance node corresponds to a state just after the joint player actions have been committed, but before the state has actually transitioned. It represents the 'decision point' for the 'chance player'.

The `Types::Observation` object of the state is what identifies the transition. The chance node's main purpose is to provide use with the unique matrix node corresponding to a given transition.  

### Interface

```cpp
	bool is_explored () const;
	// returns true if a matrix node child exists for all of the chance player's actions 
	MatrixNode<Algorithm> *access(const  typename Types::Observation&);
```