# What is This

Surskit is a highly extensible and heavily abstracted library for applying search and solving algorithms for perfect-info, simultaneous move, stochastic games.

## Intended Use

This library is really attempt at reconciling two disparate realities

* Pokemon is the highest grossing intellectual property of all time with over 20 years of competitve play in officially sanctioned and community organized formats.

* There are no agents that have been shown to be as strong as the best human players. In fact, most of the milestone developments of machine learning in other domains: (situational) tactical superiority, sound analysis, and computationally feasible search, have yet to be replicated here.

# Scope

As there is scarce publicly available work, this library attempts to attack machine learning in pokemon incrementally. By far the most important consequence of this attitude is information.

## Imperfect Info

The species of pokemon on a team, and each pokemon's moves, item, ability, etc are not known at the start of a game in any popular format. The management of this info is essential to high level play.

However my survey of methods in imperfect info games suggests that domain is significanltly more difficult and expensive to explore. Thus we make the following assumption that has natural justification

## Perfect Info

The hidden information of either player is only ever *revealed* during the course of a game, never renewed or augmented.

This means that games are always approaching a condition of perfect information. In practice, it is somewhat fre

## Simulator

TODO we use engine because nothing else is fast enough. If a sim for any other format other than RBY were to come out, the library is designed to wrap it all the same. 

This library is *format independent*

This also extends to other aspects that are explained in deeper docs :ghost: :scream:

# Installation

## Compiler

This project makes heavy use of the `Concepts` feature of C++20. Furthermore, it also uses `cartiesian_product` from C++23. Support for the latter is spotty, but GCC-13 will work

## GMP

```
    sudo apt install libgmp3-dev
```

## Clone and Install

```

```

# Example

The following code snippet is the execution of monte-carlo style tree search using a C++ Pytorch model

```cpp
using Battle = PkmnRBY<SmallTypes>;
using Model = Libtorch<Battle, Raw>;
using Search = OffPolicy<Exp3<Model>>;

typename Battle::Types::Seed seed = 42;
prng device{seed};

Battle battle{PkmnRBY::RandomTeams};

Model model{};

const double gamma = .1;
Search session{gamma};
MatrixNode<Search> root{};

const size_t playouts = 1000;
session.run(playouts, device, battle, model, root);

typename Battle::Types::VectorReal row_strategy, col_strategy;
session.get_empirical_strategies(root.stats, row_strategy, col_strategy);
```


and the following is the execution of simultaneous move AlphaBeta with depth=2.

```
using Battle = PkmnRBY<SmallTypes>;
using Model = Libtorch<Battle, Raw>;
using Search = OffPolicy<Exp3<Model>>;

typename Battle::Types::Seed seed = 42;
prng device{seed};

Battle battle{PkmnRBY::RandomTeams};

Model model{};

const int depth = 2;
Search session{depth};
MatrixNode<Search> root{};

session.run(battle, model, root);

typename Battle::Types::VectorReal row_strategy, col_strategy;
session.get_empirical_strategies(root.stats, row_strategy, col_strategy);
```

This level of abstraction makes it straightforward for a user to implement a process like below:
```
TODO PSEUDO CODE
```

## For More Info

The interface relies on a view of the search process via the following families of types:
* Types
* State
* Model
* Algorithm
* Node

The directory /src has its own readme regarding how these families relate to each other.
Also each family has its own directory in /src with its own readme.

The former doc is an overview while the latter will go more into implementation details. 

# Contributing

The best way to help is to use the library to test your own ideas. The interface is designed so that creating a custom search algorithm
