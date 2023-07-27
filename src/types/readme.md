
# Types Struct
As alluded earlier, a TypeList object is just a struct with some alias declarations.  All the `using` declarations found in the `DefaultTypes<...>` template basically specify a minimal 'standard' that any user-defined TypeList must satisfy.

### Required
* Each of the names must be defined at some point in the derivation of the `::Types` helper in order for all features of the library to compile.
* Each of the type and template aliases must have a certain, usually minimal, interface. These are described in turn further below.

### Not Required

* In `DefaultTypes<...>`, some of the basic aliases like `Action` are not defined to be exactly what is provided by the user in the template parameter list. Instead they are wrapped e.g. `using Action = ActionType<_Action>;`
* The benefits of using strong typing for certain primitives are not absolutely necessary to write working search code. More to the case against wrapping, I don't take it for granted that these abstracts are at no cost to run-time performance.
For this reason the library functions will work just fine if raw types are used instead.
* Also, strictly speaking the required types do not have to be defined all at once at this stage. But the user will have to include the declarations elsewhere if they choose to omit them in their TypeList.

# `DefaultTypes`

Below is the entire definition of the `DefaultTypes` TypeList.
The template parameters are demarcated from the actual type aliases with an underscore prefix.
The template parameters with the `template <typename...>` prefix are not simply types but are themselves templates.

```cpp
template <
    typename _Real,
    typename _Action,
    typename _Observation,
    typename _Probability,

    template <typename...> typename _Value = PairReal,
    template <typename...> typename _Vector = Vector,
    template <typename...> typename _Matrix = Matrix,

    typename _Mutex = std::mutex,
    typename _Seed = uint64_t,
    typename _PRNG = prng,
    typename _Rational = Rational<int>>
struct DefaultTypes
{
    using Real = RealType<_Real>;
    using Action = ActionType<_Action>;
    using Observation = ObservationType<_Observation>;
    using Probability = ProbabilityType<_Probability>;
    
    using Value = _Value<Real>;
    using VectorReal = _Vector<Real>;
    using VectorAction = _Vector<Action>;
    using VectorInt = _Vector<int>;
    using MatrixReal = _Matrix<Real>;
    using MatrixInt = _Matrix<int>;
    using MatrixValue = _Matrix<Value>;
    template <typename... Args>
    using Vector = _Vector<Args...>;
    template <typename... Args>
    using Matrix = _Matrix<Args...>;
    
    using ObservationHash = ObservationHashType<_Observation>;
    using Mutex = std::mutex;
    using Seed = _Seed;
    using PRNG = _PRNG;
    using Rational = _Rational;
};
```

## Wrapped Primitives

The `Real`, `Probability`, `Action`, and `Observation` type aliases that are defined in the `DefaultTypes` struct are not the same as the types which are provided in the template parameter list. Since these types are usually C++ primitives, they are instead wrapped with template classes which store the same data but also provide some extra functionality and make the search code more consistent when using library types like `mpq_class`. 

### `Wrapper<T>`

All the types in this group are derived from `Wrapper<T>`. 
* This purpose of this class is to simply hold a value of type `T`, and so any class which is derived from `Wrapper<T>` will automatically store this data by inheriting that member.
*  This class also defines a conversion to type `T`, which is basically an 'unwrapping' operation. This operator is marked as explicit, otherwise an implicit unwrapping might occur without the user's knowledge, which would circumvent the strong-typing conventions. The conversion operator is best invoked via a static cast, e.g. `double raw_data = static_cast<double>(some_real);`
* The underlying type `T` is accessible via an alias declaration `using type = T;`. This allows the user to make assertions about the underlying type easily, e.g. 
	```cpp
	if constexpr (std::is_same_v<typename Types::Real::type, mpq_class>) {
		mpq_canonicalize(some_real._value);
	}
	```
* The user should never construct or interface with this class directly. Its constructor is called by the constructors of its derived classes automatically. 

### `ArithmeticType<T>`

**This section is important to understand otherwise the user may find that expressions which look valid may cause a compilation error.**
```cpp
typename Types::Real x = 0; // valid
// ...
x += 1; // invalid, no match for operator += with...
```

The use of strong type wrappers is the root of this. The `Types::Real` and `Types::Probability` wrappers need to have the same arithmetic functionality as their underlying types. However, these operations would only be available a priori if the wrappers were *derived* from their underlying types. The class which serves as the default implementation of `Types::Vector` has this luxury, but C++ does not allow for a class to be derived from *primitive* types like `double`.

Thus we are forced to define each of these operations manually. We could define the common operations (`+`, `==`, etc) for `Real` and `Probability` separately, and we would then also have to define 'mixed' operations too (where we multiply a `Real` typed player payoff by a `Probability`).  However, the reduction of boiler-plate code (take a look at "types/arithmetic.hh" to see this) is a core design principle.

Thus we define the operations on a new class `ArithmeticType<T>` and make `RealType<T>`, `ProbabilityType<T>` derived from this class, so that they inherit these operations. This approach introduces its own kinks which have to be smoothed over.

```cpp
typename Types::Real x{1}, y{1};
x + y; // ArithmeticType<T>;
```
Both the operands are staticly cast to `ArithmeticType<T>`, which is also the return type of the operation. Thus the result has to be cast into either a `Real<T>` or a `Probability<T>`. We make this constructor explicit, again so that implicit conversions do not foil the strong typing.
```cpp
typename Types::Probability w {x * y + 1};
typename Types::Real z = x + y;
// note: the second line makes use of a specially defined assignment operator 
// RealType<T>::RealType<T> operator=(ArithmeticType<T>)
```

* Like `Wrapper<T>`, the user never needs to explicitly construct `ArithmeticType<T>`. It is always constructed explicity by invoking some arithmetic operator.

* Currently only right-handed operators are defined. That is, the expression `1 + x;` has no match but `x + 1` does. Thus we express `1 - x` (e.g. getting the column players payoff from the row players in a 1-sum game) as `x * -1 + 1` 

* Applying a non-elementary operation like `std::exp` is done by a static cast conversion, e.g. 
`const  typename  Types::Real  y{std::exp(static_cast<double>(gains[i] *  eta))};` 

### `RealType<T>` & `ProbabilityType<T>`

Since the essential mathematical operators are defined on their base class, there is currently only one task that these two are relied upon to perform.
The default implementation for multiple precision arithmetic is provided by `mpq_class`, which is the C++ front-end for GMP, or the [GNU Multiple Precision Library](https://gmplib.org/).
Unfortunately, there is one quirk of this class that makes writing polymorphic code (or more specifically code that works the same for float and `mpq_class` without template specialization or `if constexpr` everywhere) difficult.
In order for the equality and comparison (`>=` etc) operators to work correctly, the `mpq_class` objects must be *canonicalized*, or reduced to a proper fraction. Unless the user naively initializes an `mpq_class` to be un-canonical (e.g. `typename Types::Rational {2, 4}`), the only way rationals end up improper is after an arithmetic operation. The GMP library is designed for performance and thus does not perform this reduction automatically.
Our method of handling this is to automatically canonicalize the underlying value when an `ArithmeticType<mpq_class>` is cast to a `RealType<mpq_class>` or `ProbabilityType<mpq_class>`, since that typically means we are finished operating on that value.
```cpp
RealType &operator=(const ArithmeticType<T> &val) {
    this->_value = val._value;
    if constexpr (std::is_same_v<T, mpq_class>) {
        mpq_canonicalize(this->value.get_mpq_t());
    }
    return *this;
}
```
Regrettably, this means that some unnecessary reductions may be performed. However, I feel that `mpq_class` is restricted to theoretical or academic contexts, rather than in the performance critical context of a user created engine, where they will instead surely use `double` or `float` as the underlying `Real` and `Probability` types.


### `ObservationType<T>`
A wrapper for the observation type. Only the equality operator `==` is used on this, to check whether a particular state transition has occurred before. This is how the search functions match the development of a state accurately in the corresponding tree structure.

### `ActionType<T>`
A wrapper for the action type, and the argument type of the `apply_actions` method. No operators are required for this type, not even equality `==` (actions are identified by their *index* in the `row_actions`, `col_actions` containers.)

## Template Aliases

## Other Types


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
    Work on Surskit began before there was a working simulator, and so the required primitive types were unknown. Additionally, different formats and generations of Pokemon may use a different simulator.

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

This is to prevent `int row_idx, col_idx` being accidentally misused e.g. `apply_actions(row_idx, col_idx)` when the `Action` type is simply `int`.

Implicit conversions from `T` to `Wrapped<T>` are allowed, but not the other way. The idea is that `row_actions[row_idx] = 0` is permissible (when `Action` is just `int`) is fine but accidental type mixing due to an implicit `Action` -> `int` -> `Observation` conversion is not possible.

The `Action` type is merely the type of the inputs to the transition function `apply_actions(Action, Action)`. 

* `Wrapped<T>`
* `ArithmeticType`
* ``

### Arithmetic

The following types are similarly wrapped but we want to keep their arithmetic operations

* Real
* Probability

The `Real` type has the same purpose of `double` or `float`. It is the type that is used in the floating point arithmetic of the various bandit algorithms, e.g. soft-maxing, UCB scores, etc.

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

Surskit does not make the assumption that games are constant sum. Although all games of interest have this property, it is not always satisfied by every ancillary game.

In each matrix node, MatrixUCB must store a cumulative score for each of the `rows * cols` joint actions. Without the constant sum assumption, this requirement doubles.

Thus we use a special struct to payoffs/value estimates for a game. This struct stores a `Real` type member for the row player, and if the game is not constant sum then stores an additional `Real` for the column player.

```cpp
Real Value::get_row_value() const;
Real Value::get_col_value() const;
static Real Value::PAYOFF_SUM;  
```


> Warning: Although the constant sum version of the struct is smaller, it usually result in slightly less performance than its more general counterpart. The reason for this lies in the backward phase of tree bandit, where we update the matrix and chance nodes using `get_col_value()`. In the non constant sum version, the column players value estimate at the leaf node is simply retrieved. In the constant sum version, it must be recalculated at each visited node via `PAYOFF_SUM - get_row_value()`.
