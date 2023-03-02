
Surskit is designed to enable simple implementation and testing of search algorithms on a wide class of simultaneous move games. The generality of these terms is detailed below.

### `State` represents a partially-observed, stochastic matrix-tree game.
A **matrix game** is a situation where two players ( "row" and "col" ) each have a set of finite actions and each receive a payoff after simultaneously committing their actions. This can be extended to a **tree** where each node in the tree is itself a matrix game, committing causes the state to transition to the next node, and players only receive payoffs after transitioning to a terminal node.
**Stochastic** means that the transition depends not just on the joint player actions, but also on the action of a "chance" player. 
**Partially-observed** means two things: the identity and probability of aforementioned chance action may be unknown to the program. Additionally, the ex.. TODO
Despite the generality of games that can be represented by `State`, the algorithms that come implemented with Surskit are intended for the perfect-info case.

### Templates

We avoid dynamic polymorphism for performance reasons, and our solution is to use templates and a particular design pattern that gives each higher category of classes e.g. `State<TypeList>`, `Model<State>`, `Algorithm<Model>` access to the 'canonical' namespace of the lower e.g. every `Model` type has an `Inference` type.
Additionally the patern allows us to make derivation assertions about the template/lower class without resorting to making it a template template parameter.
The patter is generically depicted in a minimal example below, with comments that provide as specific example with Surskit


    #include <concepts>

    class FromLowerBase{};
    class FromLowerDerived : public FromLowerBase {};
    class LowerDerived : public FromLowerDerived {};

    class FromUpperBase {};
    template <class LowerDerived>
    class UpperBase {};

    /*
    Let LowerBase = SolvedState, LowerDerived = A particular toy game implementation
    and UpperBase = ValuePolicyModel.

    We are defining MonteCarloSolved, the Monte Carlo model but specialized so that it takes inference on SolveStates,
    by taking their known solutions and processesing them to simulate the policy output of an expert.
    (This would be used to test algorithms that use policy, Exp3p and MatrixUCB don't)
    */

    class _UpperDerived : public _UpperBase {};
    // Derive generic MonteCarloPolicy from generic ValuePolicyModel so that...
    template <class LowerDerived>
    class UpperDerived : public _UpperDerived {
    // specified MonteCarloPolicy<T> is then derived from generic ValuePolicyModel. 
    // This lets us type check the MonteCarloPolicy<T> when it itself is a template parameter at a higher level...
    static_assert(std::derived_from<LowerDerived, _LowerBase> == true);
    // with a static assert just like this. 
    // At the level of MODEL, the above asserts that MonteCarloPolicy<State> being passed a State that is really a SolvedState.
        struct SpecializationOfUpperType;
        struct NewTypeForUpperDerived;
        struct Types : UpperBase<LowerDerived>::Types {
                    // Notice Types inherit from specified Types, which lets it cross levels: TypeList -> State -> Model -> Algo
            using UpperType = SpecializationOfUpperType;
            // This type (e.g. Inference) was nested in ValuePolicyModel but maybe we are adding new members/methods
            using NewTypeForUpperDerived = UpperDerived::NewTypeForUpperDerived;
        };
        // The forward declaration of structs lets us put the Types inheritance boilerplate always at the top

        struct SpecializationOfUpperType {
            // Now any type of interest is accessible, if you know it's 'global' name, with
            // typename Types::Name ... ;
        };
        struct NewTypeForUpperDerived {
            // ...
        };
        // Impl...

    };

    int main () {
        UpperDerived<LowerDerived> okay;
        // compiles

        UpperDerived<int> error;
        /*
    [build] /home/user/Desktop/surskit/test/test.cc:26:20: error: template argument 2 is invalid
    [build]    26 | static_assert(std::derived_from<LowerDerived, _LowerBase> == true);
    [build]       |                    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        */

        return 0;
    }