# Flexibility

# Layout

> Types State Model Algorithm

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

```cpp
```
In `AbstractUpper` we derive the Types struct