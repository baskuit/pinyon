
# Types

Surskit was designed to test search algorithms ... This requires interchangeability 

* `State`
Development on Surskit began before there were any fast Pokemon simulators, so already there is no concrete game that we can assume as a base. Additionally, within pokemon there are many different generation and formats of interest, and the techniques and algorithms we want to investigate are not limited to any specific game.
* `Model`
We borrow this term from machine learning, where it typically refers to a trained neural network.
Search is simply looking ahead, we still need a way to estimate the game value of the future states our search algorithm visits. This is left to the 'Model'
There are three major approaches in computer learning. One is a handcrafted heuristic. Another is a large neural network, a la Alpha Zero. Finally we have a smaller neural network like the NNUE, which powers the recent versions of stockfish. It was not clear from the onset approach would yield a strong agent.
* `Algorithm`

Ancillary types:
* `TypeList`

* `Node`

## Hierarchy

The families of types above can be arranged in 

## Advantages of Modularity

### Testing

### Uncertainty

### Tuning

# Implementing

## A naive approach

The above is a useful framework for thinking about

## Templates to the Rescue

# Organization

Every object in surskit is given a subobject called Types which inherits

`typename Types::VectorAction`

`TypeList`
`AbstractState<TypeList>`
`PerfectInfoState<TypeList>`


# Idioms


```cpp
template <class Lower>
class UpperDerived : public UpperBase<Lower> {
    struct SomeStruct;
    struct Types :: UpperBase<Lower>::Types {
        using SomeStruct = UpperDerived::SomeStruct;
    };
};
```
If abstraction is not your preference, we can give a specific example

```cpp
```
In `AbstractUpper` we derive the Types struct
