# Tree Structure
Because the tree structure is intended to represent *stochastic* games, we require two different kinds of nodes to faithfully represent the game tree: `MatrixNodes` and `ChanceNodes`.
The node implementations differ in how much information they store (debug stores bidirectional pointers) and how they store and access the pointers of their children.
### Ownership
A node owns its children in the sense that a nodes destructor will always delete all of the nodes children.
### Matrix Node Primacy
Conceptually a matrix node corresponds to a state. A chance node is transitory, and represents the decision point of the 'chance player' for a particular pair of actions for the row and column player.
Once consequence of this is that chance nodes are virtually never constructed by the user, instead they are as a consequence of search operations.
### Pointer vs Reference
The interface tends to pass references to (matrix) nodes while implementation functions use pointers. This is related to the point regarding ownership. The root node for a search is usually created in block scope instead of a heap allocated, and the ownership scheme means that the tree will be cleaned up when the root is destroyed at the end of scope. 
This means it is natural to pass a reference at the top level interface. Implementation functions tend to use pointers since nodes store their children and siblings as pointers.
### Identification
obs vs row, col idx TODO

# Concepts/Interface

## MatrixNode
```cpp
{
    matrix_node.stats
} -> std::same_as<typename Types::MatrixStats &>;
```
The search data member is accessed directly instead of by method. The data is not initialized when the node is constructed; That is expected to happen in the `expand`method  (or `expand_using_output` for "off-policy" search.)
```cpp
{
    matrix_node.access(0, 0)
} -> std::same_as<typename Types::ChanceNode *>;
```
This is a state agnostic way of accessing the child node corresponding to joint actions, assuming that action order is unique. It will return a pointer to the node, and if the node does not already exist then it will heap allocate it.
```cpp
{
    const_matrix_node.access(0, 0)
} -> std::same_as<const typename Types::ChanceNode *>;
```
This behaves the same except it will return `nullptr` rather than allocate. It is used in the `TraversedState::State` class so that the matrix node that identifies the state in the tree can be const.
```cpp
{
    matrix_node.expand(state)
} -> std::same_as<void>;
```
Ex
```cpp
{
    const_matrix_node.is_expanded()
} -> std::same_as<bool>;
{
    const_matrix_node.is_terminal()
} -> std::same_as<bool>;
{
    matrix_node.set_terminal(false)
} -> std::same_as<void>;
{
    matrix_node.set_terminal()
} -> std::same_as<void>;
```
In all implementations, these are mapped to bool members `terminal`, `expanded`. It may be profitable to use e.g. a single byte to store this information, so we use getter and setter methods instead of direct access. Note: the multi-threaded search assumes that `is_expanded()` is atomic, which is not always true. It currently is on x86 since read/write of cache aligned primitives is atomic on that architecture.
Also, `is_terminal()` is not used in single or multi-threaded search, since those currently check if the attached state is terminal. It is necessary for off policy search since the state is no longer attached at that point in the program (although it could be stored at some penalty if wanted.) The getter does allow for the possibility of marking nodes as terminal during run-time. TODO unclear
```cpp
{
    Types::MatrixNode::STORES_VALUE
} -> std::same_as<const bool &>;
```
* `stats`
The search data of any tree bandit or solver class is stored here.
* `access(int, int)`
This is a state agnostic way of accessing the child node corresponding to joint actions, assuming that action order is unique. It will return a pointer to the node, and if the node does not already exist then it will heap allocate it.
* `access(int, int) const`
This behaves the same except it will return `nullptr` rather than allocate. TODO where used lol?
* `expand(state&)`

* `is_terminal() const`
## ChanceNode

```cpp
{
    chance_node.stats
} -> std::same_as<typename Types::ChanceStats &>;
{
    chance_node.access(obs)
} -> std::same_as<typename Types::MatrixNode *>;
{
    const_chance_node.access(obs)
} -> std::same_as<const typename Types::MatrixNode *>;
```
See their mirrors for matrix nodes.


