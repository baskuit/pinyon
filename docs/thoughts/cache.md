# Speeding up tree search

The most only consideration I've put into tree performance so far has been limited to static polymorphism.

This avoids cost of vtable (which might not matter much since the correct function addresses probably get cached)

It probably helps with cache use in general though since the `MatrixStats`, `ChanceStats` structs are specified to each algorithm.

But my `Node` implementation is probably larger than it needs to be since we:
* Two bools for terminal and expanded.
* Store inference for all nodes when usually we don't care about the first inference value unless its a terminal node
* Store strategy as a float when it could easily be stored is a uint_8. 1/256 = .00390625.

Ironically I think all this is a case for templating, since now I can implement the uint8_t "float" as `Types::Strategies` etc. Just need to figure out what types to add. That can be deduced by going through the tree/algorithm code