#include <concepts>
template <typename State>
concept IsPerfectInfoState = requires(
    State obj,
    typename State::Types::VectorAction &vec,
    typename State::Types::PRNG &device) {
    {
        obj.terminal
    } -> std::same_as<bool &>;
    {
        obj.apply_actions(
            typename State::Types::Action{},
            typename State::Types::Action{})
    } -> std::same_as<void>;
    {
        State::IS_CONSTANT_SUM
    } -> std::same_as<bool>;
    {
        typename State::Types::Real {}
    } -> std::same_as<typename State::Types::Real>;

    // assert the existence of dependent type
    typename State::Types;
    typename State::Types::ExistingType;
};

    template <IsPerfectInfoState State>
    void test () {
        State state;
        state.terminal; // OK, shows bool type
        state.apply_actions(0, 0); //OK, shows `typename State::Types::Action` param type
        state.IS_CONSTANT_SUM; // Ok, shows static bool type
        // State:: will autocomplete all the above as well
        State::IS_CONSTANT_SUM;

        
        // Not OK, does not autocomplete Types nor Types::ExistingType
        //typename State::
    }