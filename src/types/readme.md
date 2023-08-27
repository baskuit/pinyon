
# Types Struct

As alluded earlier, a type list object is just a struct with some type declarations for a certain set of names (`Real`, `Mutex`, `VectorReal`) etc. The `DefaultTypes` struct in the next section essentially lists the minimal collection of declaration that are used in the library. There are also some additional constraints on the individual types that defines an expected interface.
The use of concepts will mean this interface is auto-suggested when dealing with constrained template parameters.

### Strong Typing

In the case of some aliases, the final type that is accessible via the type list (e.g. `Types::Real`) is **not** simply `float` or `mpq_class` as the user intended.
Instead, some classes are actually wrapper classes around the underlying type that mainly provide strong typing and also normalize the Surskit interface.

To the first point, let's consider a motivating example: If the underlying primitive type for an `Action` is `int`, then the following erroneous code would compile without warning
```cpp
	int row_idx = device.sample_pdf(row_strategy);
	int col_idx = device.sample_pdf(col_strategy);
	typename Types::Action row_action, col_action;
	row_action = row_actions[row_idx];
	col_action = col_actions[col_idx];
	state.apply_actions(row_idx, col_idx); // error
```

To the second point, most of the interface trouble comes from `mpq_class`. For example, conversion from a `mpq_class` member to a double is done via its `get_d()` method, rather than a conversion operator `static_cast<double>()`.

### Nearly Optional

When strong typing was introduced I had planned to design the library so that they could be eschewed for the raw types if the user preferred. However, the support for `mpq_class` means that's not going to happen until I disentangle `GMP`from Surskit.

This should not be an issue because of C++'s *zero cost abstractions*. If a program is compiled with `-O1` or higher (e.g. release mode in VSCode/Cmake) then these abstractions will be optimized away.

# `DefaultTypes`

Below is the entire definition of the `DefaultTypes` TypeList.
The template parameters are demarcated from the actual type aliases with an underscore prefix.
The template parameters with the `template <typename...>` prefix are not simply types but are themselves templates.

```cpp
template <
    typename _Real,
    typename _Action,
    typename _Obs,
    typename _Prob,

    template <typename...> typename _Value = PairReal,
    template <typename...> typename _Vector = std::vector,
    template <typename...> typename _Matrix = Matrix,

    typename _Mutex = std::mutex,
    typename _Seed = uint64_t,
    typename _PRNG = prng,
    typename _Rational = Rational<int>>
struct DefaultTypes
{
    using TypeList = DefaultTypes;
    using Q = _Rational;
    using Real = RealType<_Real>;

    using Action = ActionType<_Action>;
    using Obs = ObsType<_Obs>;
    using Prob = ProbType<_Prob>;

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

    using Mutex = std::mutex;
    using PRNG = _PRNG;
    using Seed = _Seed;
};
```

## Wrapped Primitives

The `Real`, `Prob`, `Action`, and `Obs` type aliases that are defined in the `DefaultTypes` struct are not the same as the types which are provided in the template parameter list. Since these types are usually C++ primitives, they are instead wrapped with template classes which store the same data but also provide some extra functionality and make the search code more consistent when using library types like `mpq_class`. 

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

The use of strong type wrappers is the root of this. The `Types::Real` and `Types::Prob` wrappers need to have the same arithmetic functionality as their underlying types. However, these operations would only be available a priori if the wrappers were *derived* from their underlying types. The class which serves as the default implementation of `Types::Vector` has this luxury, but C++ does not allow for a class to be derived from *primitive* types like `double`.

Thus we are forced to define each of these operations manually. We could define the common operations (`+`, `==`, etc) for `Real` and `Prob` separately, and we would then also have to define 'mixed' operations too (where we multiply a `Real` typed player payoff by a `Prob`).  However, the reduction of boiler-plate code (take a look at "types/arithmetic.hh" to see this) is a core design principle.

Thus we define the operations on a new class `ArithmeticType<T>` and make `RealType<T>`, `ProbType<T>` derived from this class, so that they inherit these operations. This approach introduces its own kinks which have to be smoothed over.

```cpp
typename Types::Real x{1}, y{1};
x + y; // ArithmeticType<T>;
```
Both the operands are staticly cast to `ArithmeticType<T>`, which is also the return type of the operation. Thus the result has to be cast into either a `Real<T>` or a `Prob<T>`. We make this constructor explicit, again so that implicit conversions do not foil the strong typing.
```cpp
typename Types::Prob w {x * y + 1};
typename Types::Real z = x + y;
// note: the second line makes use of a specially defined assignment operator 
// RealType<T>::RealType<T> operator=(ArithmeticType<T>)
```

* Like `Wrapper<T>`, the user never needs to explicitly construct `ArithmeticType<T>`. It is always constructed explicity by invoking some arithmetic operator.

* Currently only right-handed operators are defined. That is, the expression `1 + x;` has no match but `x + 1` does. Thus we express `1 - x` (e.g. getting the column players payoff from the row players in a 1-sum game) as `x * -1 + 1` 

* Applying a non-elementary operation like `std::exp` is done by a static cast conversion, e.g. 
`const  typename  Types::Real  y{std::exp(static_cast<double>(gains[i] *  eta))};` 

### `RealType<T>` & `ProbType<T>`

Since the essential mathematical operators are defined on their base class, there is currently only one task that these two are relied upon to perform.
The default implementation for multiple precision arithmetic is provided by `mpq_class`, which is the C++ front-end for GMP, or the [GNU Multiple Precision Library](https://gmplib.org/).
Unfortunately, there is one quirk of this class that makes writing polymorphic code (or more specifically code that works the same for float and `mpq_class` without template specialization or `if constexpr` everywhere) difficult.
In order for the equality and comparison (`>=` etc) operators to work correctly, the `mpq_class` objects must be *canonicalized*, or reduced to a proper fraction. Unless the user naively initializes an `mpq_class` to be un-canonical (e.g. `typename Types::Q {2, 4}`), the only way rationals end up improper is after an arithmetic operation. The GMP library is designed for performance and thus does not perform this reduction automatically.
Our method of handling this is to automatically canonicalize the underlying value when an `ArithmeticType<mpq_class>` is cast to a `RealType<mpq_class>` or `ProbType<mpq_class>`, since that typically means we are finished operating on that value.
```cpp
RealType &operator=(const ArithmeticType<T> &val) {
    this->_value = val._value;
    if constexpr (std::is_same_v<T, mpq_class>) {
        mpq_canonicalize(this->value.get_mpq_t());
    }
    return *this;
}
```
Regrettably, this means that some unnecessary reductions may be performed. However, I feel that `mpq_class` is restricted to theoretical or academic contexts, rather than in the performance critical context of a user created engine, where they will instead surely use `double` or `float` as the underlying `Real` and `Prob` types.


### `ObsType<T>`
A wrapper for the Obs type. Only the equality operator `==` is used on this, to check whether a particular state transition has occurred before. This is how the search functions match the development of a state accurately in the corresponding tree structure.

### `ActionType<T>`
A wrapper for the action type, and the argument type of the `apply_actions` method. No operators are required for this type, not even equality `==` (actions are identified by their *index* in the `row_actions`, `col_actions` containers.)


## Vectors

In most cases, `std::vector` is an adequate container for storing floats, actions, etc.

However, vectors are heavier than `std::array`, which does not reallocate or allocate unneeded storage and which size is known at compile-time. This means standard arrays are preferable in certain cases, for example if a game always has the same number of actions.

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

Simple rational type for exact prob, value calculation. Rationals are the preferred way of initializing `Real`, `Prob` and other arithmetic values since may also be a precise representation!
The library provides a global accessible type `Rational<T>`, where `T` is the underlying integral type for the numerator and denominator. 
In the named Type structs, `Rational<int>` is the default `Rational` type.

```cpp
	Rational(T p);
	Rational(T p, T q);
```

* Strategy

TODO
This type is currently unused. For bandit algorithms that use the model's policy inference (MatrixPUCB), the policy for both players is stored in the matrix node stats. Using even a vector of doubles here would explode the size of the matrix nodes, resulting in a significant performance drop. 

Since Prob distributions satisfy `0 < p_i < 1` for all entries `p_i` and precision is not terribly important, we can quantize the distribution using unsigned integers.

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
