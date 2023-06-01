# Arithmetic

Strong typing wrappers for Probability vs Real, e.g.

# Matrix

Matrix entries include floats, ints, and pairs of floats (values)
1-D vector implementation. In fact, derives from std::vector<T>.

# Random

Simple Mersenne Twister wrapper. Once this and several other things that were in /libsurskit were made modular I moved them into /types

# Rational

Simple rational type for exact prob, value calculation. Lsrlib needs `mpz_t` to work under the hood so we make those rationals available as well.

# Strategy

Smaller representation for strategies. E.g. `uint8_t` as base represents fractions `x / 256`. Use `n - 1` dimensions, since the last is implied `\sigma = 1`.
Mostly desired for smaller matrix nodes, hence only bandits that store strategies (e.g. p_uct) will need this.

# Value

Way to accomodate faster constant-sum and more general sum states with the least amount of refactor

# Vector

Use std vector or array!