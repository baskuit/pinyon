
# What is this?

This repo is a header-only library version of Gambit, with every feature except those necessary for the EnumMixed solver stipped away. It comes with a CMakeLists.txt and example program to illustrate how it might be used within a project.

A rudimentary speed test showed it is 6.5x faster than vanilla Gambit on matrices of size [1, 5] x [1, 5], and 2x as fast on matrices of size [6, 9] x [6, 9].

Further optimizations are likely.