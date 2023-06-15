# Why Use Types Struct

All search functions and higher level families of classes will make use of a variety of primitive and object types. The State family needs a small type to represent the actions the players, and a type to act as the observation or signature of a transition, so that it can be identified. The search algorithms need mathematical types like vectors and matrices, and so on.

Rather than fixing these types, we make the most basic family of classes to be the `Types` struct. This struct uses `using` declarations to determine the basic types, and all the higher classes borrow from this struct like so:

```cpp
typename Types::Real half {Rational(1, 2)};
```
which is equivalent to 
```cpp
double half = 0.5;
```

There are several reasons for this modularity.

* Performance
Using different data types for representation can make a significant impact on performance. For example using `float` instead of `double` significantly reduces the number of bytes needed to represent a matrix node of the tree. Smaller nodes means better cache usage, which is one of the most important considerations regarding performance of a search engine.

* Generality
    Work on Surskit began before there was a working simulator, and so the required primitive types were unknown. Additionally, different formats and generations of pokemon may use a different simulator.

* Strong Typing
Organizing types in this manner makes it simpler to use strong typing. There are many reasons for doing this, but principal among them is the elimination of certain kinds of silents bugs.
```cpp
state.apply_actions (row_idx, col_idx); // wrong, but legal since Action = int
```

# Required Aliases

A `Types` struct is expected to have the following aliases defined

Arithmetic:

* `Rational`
* `Real`
* `Float`

State:

* `Action`
* `Observation`
* `Probability`
* `Seed`
* `PRNG`
* `Value`
* `Strategy`

Data:

* `VectorReal`
* `VectorAction`
* `VectorInt`
* `MatrixReal`
* `MatrixInt`
* `MatrixValue`


The motivation and requirements of these type aliases are discussed below.

## Wrapped Types

The following types are given wrappers for type correctness, but otherwise gain little to no functionality.

* Action
* Observation

This is to prevent `int row_idx, col_idx` being accidently misused e.g. `apply_actions(row_idx, col_idx)` when the `Action` type is simply `int`.

Implicit coversions from `T` to `Wrapped<T>` are allowed, but not the other way. The idea is that `row_actions[row_idx] = 0` is permissible (when `Action` is just `int`) is fine but accidental type mixing due to an implicit `Action` -> `int` -> `Observation` conversion is not possible.

The `Action` type is merely the type of the inputs to the transition function `apply_actions(Action, Action)`. 

### Arithmetic

The following types are similarly wrapped but we want to keep their arithmetic operations

* Real
* Probability

The `Real` type has the same purpose of `double` or `float`. It is the type that is used in the floating point arithmetic of the various bandit algorithms, e.g. softmaxing, UCB scores, etc.

The `Probability` type simply represents the probability of a transition occurring. This quantity is stored in each state as `PerfectInfoState::prob`.

This quantity is not needed for the tree bandit algorithms, since they are sample based and hence the probabilities of transitions are 'baked into' the observations. On the other hand, this quantity is needed for solving methods, since otherwise we cannot calculate the expected payoff of chance nodes.

## Vectors

In most cases, `std::vector` is an adequate container for storing floats, actions, etc.

However, vectors are heavier than `std::array`, which does not reallocate or allocate unneeded storage, and its size is known at compile-time. This means standard arrays are preferable in certain cases. For example, if a game always has the same number of actions for both players, why not use an `std::array` to store that information?

Thus we use `VectorReal`, `VectorInt`, `VectorAction` to specify the most common uses for contiguous containers.

> The following requirements apply to all three vector types

`Vector::fill(int n)`

Ensures that the container can hold at least `n` elements. If the underlying type is a `std::vector`, this method calls `std::vector<T>::resize()`. On a `std::array` based Vector, this is a no-op. The program assumes the underlying size of the array is at least `n`, and there are no static or run-time checks to ensure this. 

`Vector::fill(int n, T value)`

Same as above, but also writes `value` into the first `n` elements of the container.
Both the above methods are used mainly for initializing containers stored in the tree, during the expansion of a matrix node.

`T& Vector::operator[]()`

Besides the fill methods, a Vector need only have the standard bracket operator. TODO I'm pretty sure we also use std::transform and range based iteration.


## Matrices

Obviously a library for matrix tree games should have a matrix type. In practice though its mostly a data class, used to store visit and reward information for the MatrixUCB algorithm and its variants. There is currently no need for matrix multiplication. The closest to that use-case is the calculation of exploitability, where the calculation of expected rewards can be done using matrix multiplication. However, in practice I've found it's faster to use another approach.

Much like Vectors, we have specially denominated types for common data types: `MatrixReal`, `MatrixInt`, `MatrixValue`

> The following requirements apply to all Matrix types

```cpp
	void fill(size_t rows, size_t cols);
```
This also serves the purpose of initializing a matrix, ensuring it has at least `rows * cols` entries. This method also sets the `rows` and `cols` dimensions.

```cpp
	void fill(size_t rows, size_t cols, T value);
```
Same as above, but also sets all entries to `value`.

```cpp
	T& get(size_t row_idx, size_t col_idx)
```
2-D accessor, similar to `operator[]`


``` cpp
    Matrix operator*(T t) const
    Matrix operator+(T t) const
    Matrix operator+(const Matrix &t) const
```
Only scalar multiplication/addition and matrix addition are needed.

```cpp
	T max() const
	T min() const
```
The gambit solver normalizes the entries in the payoff matrix. These methods are used to assist that.

## Remaining Types

* `PRNG`
* `Seed`

The core search functions are designed to be deterministic.

State transitions are determined by their `seed` member, which is randomized at the start of the forward phase of tree bandit search.

```cpp
	PRNG();
	PRNG(Seed seed);
	Seed new_seed ();
```

The provided pseudo-random number generator  is `prng`, which is just a simple wrapper around `std::mt19937`. Users may want to replace this with something faster.

* `Rational`

Simple rational type for exact prob, value calculation. Rationals are the preferred way of initializing `Real`, `Probability` and other arithmetic values since may also be a precise representation!
The library provides a global accessible type `Rational<T>`, where `T` is the underlying integral type for the numerator and denominator. 
In the named Type structs, `Rational<int>` is the default `Rational` type.

```cpp
	Rational(T p);
	Rational(T p, T q);
```

* Strategy

TODO
This type is currently unused. For bandit algorithms that use the model's policy inference (MatrixPUCB), the policy for both players is stored in the matrix node stats. Using even a vector of doubles here would explode the size of the matrix nodes, resulting in a significant performance drop. 

Since probability distributions satisfy `0 < p_i < 1` for all entries `p_i` and precision is not terribly important, we can quantize the distribution using unsigned integers.

For example, `uint8_t` has 256 possible values. Thus  the distro `{0.3, 0.3, 0.4}` could be represented as
```cpp
uint8_t strategy[3] = {76, 76, 104};
```

This is a 4x reduction in size from `float` already and we could even omit the last entry, since the probabilities must sum to 1.

* `Value`

Surskit does not make the assumption that games are constant sum. Although all games of interest have this property, it is not always satisfied by every ansillary game.

In each matrix node, MatrixUCB must store a cumulative score for each of the `rows * cols` joint actions. Without the constant sum assumption, this requirement doubles.

Thus we use a special struct to payoffs/value estimates for a game. This struct stores a `Real` type member for the row player, and if the game is not constant sum then stores an additional `Real` for the column player.

```cpp
Real Value::get_row_value() const;
Real Value::get_col_value() const;
static Real Value::PAYOFF_SUM;  
```


> Warning: Although the constant sum version of the struct is smaller, it usually result in slightly less performance than its more general counterpart. The reason for this lies in the backward phase of tree bandit, where we update the matrix and chance nodes using `get_col_value()`. In the non constant sum version, the column players value estimate at the leaf node is simply retrieved. In the constant sum version, it must be recalculated at each visited node via `PAYOFF_SUM - get_row_value()`.
