
Surskit is designed to enable simple implementation and testing of search algorithms on a wide class of simultaneous move games. The generality of these terms is detailed below.

### `State` represents a partially-observed, stochastic matrix-tree game.
A **matrix game** is a situation where two players ( "row" and "col" ) each have a set of finite actions and each receive a payoff after simultaneously committing their actions. This can be extended to a **tree** where each node in the tree is itself a matrix game, committing causes the state to transition to the next node, and players only receive payoffs after transitioning to a terminal node.
**Stochastic** means that the transition depends not just on the joint player actions, but also on the action of a "chance" player. 
**Partially-observed** means two things: the identity and probability of aforementioned chance action may be unknown to the program. Additionally, the ex.. TODO
Despite the generality of games that can be represented by `State`, the algorithms that come implemented with Surskit are intended for the perfect-info case.

### Templates

We avoid dynamic polymorphism for performance reasons, and our solution is to use templates and a particular design pattern that gives each higher category of classes e.g. `State<TypeList>`, `Model<State>`, `Algorithm<Model>` access to the 'canonical' namespace of the lower e.g. every `Model` type has an `Inference` type.
Additionally the patern allows us to make derivation assertions about the template/lower class without resorting to making it a template template parameter.
The pattern is generically depicted in a minimal example below, with comments that provide a specific example within Surskit


    #include <concepts>

    class LowerBase
    {
    };
    class LowerDerived : public LowerBase
    {
    };
    // These each have a Types struct which derive covariantly

    template <class _Lower>
    class AbstractUpper
    {
    public:
        struct Types : _Lower::Types
        {
            using Lower = _Lower;
        };
        // All Upper objects' Types will derive from AbstactUpper's Types, propogating the Type list higher.
    };
    template <class LowerDerived>
    class UpperBase : public AbstractUpper<LowerDerived>
    {
    };

    /*
    Let LowerBase = SolvedState, LowerDerived = A particular toy game implementation
    and UpperBase = ValuePolicyModel.

    We are defining MonteCarloSolved, the Monte Carlo model but specialized so that it takes inference on SolveStates,
    by taking their known solutions and processesing them to simulate the policy output of an expert.
    This would be used to test algorithms that use policy, of which Exp3p and MatrixUCB don't, for instance.
    */

    template <class LowerDerived>
    class UpperDerived : public UpperBase<LowerDerived>
    {
        // This outer derivation is the most important aspect because it provides the shadowing static polymorphism that gives Surskit a compact and extensible API

        // static_assert(std::derived_from<LowerDerived, LowerBase<typename LowerDerived::Types::LowerLower>>);
        // This static assertion would check that "LowerDerived" parameter is really is derived from LowerBase

        static_assert(std::derived_from<LowerDerived, LowerBase>);
        // This static assert emulates the real one above without needing too many dummy classes...

        struct SpecializationOfUpperType;
        struct NewTypeForUpperDerived;
        struct Types : UpperBase<LowerDerived>::Types
        {
            using UpperType = SpecializationOfUpperType;
            // This type (e.g. Inference) was nested in ValuePolicyModel but maybe we are adding new members/methods
            using NewTypeForUpperDerived = UpperDerived::NewTypeForUpperDerived;
            // If we want lower or derived classes to have access to the type
        };
        // The forward declaration of structs lets us put the Types inheritance boilerplate always at the top

        struct SpecializationOfUpperType
        {
            // ...
        };
        struct NewTypeForUpperDerived
        {
            // ...
        };
        // Now any type of interest is accessible, if you know it's 'global' name, with
        // typename Types::Name ... ;
    };
    int main()
    {
        UpperDerived<LowerDerived> okay;
        // compiles

        UpperDerived<int> error;
        /*
    [build] /home/user/Desktop/surskit/src/main.cc:43:24: error: static assertion failed
    [build]    43 |     static_assert(std::derived_from<LowerDerived, LowerBase>);
    [build]       |                   ~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        */

        return 0;
    }