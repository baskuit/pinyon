This folder contains dynamic polymorphism wrappers for the state, model, and algorithm families.

For disparate algorithm types like `using A1 = TreeBanditThreaded<Exp3<MonteCarloModel<RandomTree>>>` and `using A2 = TreeBandit<MatrixUCB<MonteCarloModel<RandomTree>>>`, we can wrap these as `Search<A1>` and `Search<A2>`, which both inherit from the virtual, non-templated class `Search`.

A simple and clear application of this can be found in the `Arena` class, which treats different algorithms as 'actions' of an abstract game and uses search functions to efficiently determine which algorithm results in the strongest play.

The `run` method of each of the search algorithms is wrapped. This means that the searches still enjoy the performance benefit of static polymorphism, while the abstraction layer above them can exploit the power of dynamic polymorphism where there are no performance penalties.
