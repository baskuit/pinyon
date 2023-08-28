
# State
### Markov Decision Process
Loosely speaking a state is a Markov Decision Process. There is some ambiguity that should be cleared up.
The **type** of a state corresponds to a game or a MDP, while an **instance/object** corresponds to a MDP 'state'
### Perfect Information and Inheritance
The assumption of perfect information means that the actions for both players is public knowledge.


# Perfect Info

In practice, search on general imperfect information games is hard and expensive. Therefore we codify the perfect information requirement and several other niceties with the `PerfectInfoState` class, and we use this class as the basis of all current models, algorithms, and tree structures.

The goal of Surskit is to aid development in an imperfect info game, so eventually we will expand our methods and classes as perfect info milestones are reached.


# Concepts/Interface
`IsStateTypes`:
```cpp
{
    const_state.is_terminal()
} -> std::same_as<bool>;
```
Self explanatory.
```cpp
{
    const_state.get_payoff()
} -> std::same_as<typename Types::Value>;
```
This function can always be called but the result is only valid when the state is terminal. Otherwise, it is likely to be uninitialized.
```cpp
{
    state.apply_actions(action, action)
} -> std::same_as<void>;
```
Without the assumption of perfect info, we cannot assume that an identifying observation is returned or that the probability of the transition is known.
```cpp
{
    const_state.get_actions(actions, actions)
} -> std::same_as<void>;
```
It is assumed that calculating actions 
```cpp
{
    state.randomize_transition(device)
} -> std::same_as<void>;
```
`IsPerfectInfoStateTypes`:
```cpp
{
    state.obs
} -> std::same_as<typename Types::Obs &>;
{
    state.prob
} -> std::same_as<typename Types::Prob &>;
```
The assumption of perfect info requires 
```cpp
{
    state.terminal
} -> std::same_as<bool &>;
```
The terminality of the state is stored as a bool and the method `is_terminal` is implemented as a getter.
```cpp
{
    state.payoff
} -> std::same_as<typename Types::Value &>;
{
    state.get_actions()
} -> std::same_as<void>;
{
    state.row_actions
} -> std::same_as<typename Types::VectorAction &>;
{
    state.col_actions
} -> std::same_as<typename Types::VectorAction &>;
```
These are essentially the same methods and attributes from the IsState concept, except now we assume that the information which passed as arguments to those methods is stored as a member. This is largely a convenience, to spare the use from instantiating action vectors and values all the time. There should be very little overhead to this attachment. 

* `is_terminal`
Simple boolean that is flipped in the `apply_actions` method as needed. Initialized to `false` since there is no reason not to assume a state isn't live upon its initialization.

* `row_actions`
* `col_actions`

> Not that much slower that using `get_actions(typename Types::VectorAction &, Types::VectorAction &)`. General case of expanding a node in tree-bandit justifies allocating the space for every  state.

* `payoff`
Following the convention of many other games, a reward is only received at a terminal state. This reward is of `Types::Value` type. Its value could be set inside the `apply_actions` method, for example. Its value is garbage until the state is terminal.

* `obs`
* `probs`
The `Observation` after transitioning and its associated `Probability` are assumed to mutated after every `apply_actions` call.

* `get_actions` 
Updates the `row_actions`, `col_actions` members of the state. The number of actions for either player can be retrieved by `row_actions.size()` etc, as they are of the `VectorAction` type. Calculation of all actions for both players is assumed to be expensive (as is often the case), so the search does it only when necessary

Thus the actions vectors are not assumed to be initialized on creation of a state. The `get_actions` method *must* be called directly to assure there is valid information there. The tree-bandit search is organized so that get_actions is always called prior to accessing that data.

* `apply_actions`
Transitions the state, and updates the `obs`, `prob` members. 

* `reseed`
**States are deterministic**. Copying a state does not alter the behavior either. In order to transition stochastically, the `reseed` method must be applied first. In tree bandit search, this is done just after copying the state object provided in the  arguments. With deterministic states, this function is just a no-op. In the case of battles, we use the PRNG device to update the battle's seed.

# Test States

### MoldState

This is basically the simplest possible state in implementation. It's main purpose is testing and benchmarking, as a control. It's only member is the `depth` parameter, which determines how many transitions until the state is terminal. It always has the same number of actions for both players, and so the `row_actions`, `col_actions` members are initialized in the `MoldState` constructor and never changed. Thus `get_actions` is a no-op and `apply_actions` merely decrements `depth` and checks if `depth == 0`.


### RandomTree

TODO This class is a powerful and expressive way to create a random game.

The biggest advantage of them is probably the fact that they are determined by their `PRNG` member.

Arbitrarily large games!

Fast!

Customizable with function pointers and mini-library (TODO) for simulating

The default behavior of a tree is that of a P-game.

# ChanceState

# SolvedState

A state that is solved does not necessarily have to have the same interface as `ChanceState`. However being a chance state is required for the solvers to work, and it is common to 'upgrade' a `ChanceState` to a `SolvedState` using the `FullTraversal` class. In practice, by this process all solved states are also chance states, and so deriving `SolvedState : public ChanceState` is justified.



# Wrapping a State

As an example, below is a wrapper for the `battle` object of the @pkmn/engine library for use with Surskit


```cpp
template <size_t MaxLog>
class Battle : public PerfectInfoState<BattleTypes<MaxLog>>
{
public:
   struct Types : PerfectInfoState<BattleTypes<MaxLog>>::Types
   {
   };

   // row_actions, obs, prob, etc are all inherited and accessed via this->

   pkmn_gen1_battle battle_{}; 
   // This is around 370 bytes, so only 6 cache lines on most cpu? Either way very fast to copy.
   // typename Types::Seed seed = 0;
   // because we use the interface this->reseed(PRNG), we don't have to store a seed member besides the defacto seed in the battle
   pkmn_psrng random = {};
   pkmn_result result = PKMN_RESULT_NONE;
   std::array<pkmn_choice, 9> options{0};
   // things I just copied from the C++ wrapper for engine.

   Battle(const engine::RBY::Side<engine::Gen::RBY> &side1, const engine::RBY::Side<engine::Gen::RBY> &side2)
   {
      auto it_s1 = side1.cbegin();
      auto it_e1 = side1.cend();
      auto it_s2 = side2.cbegin();
      auto it_b = std::begin(battle_.bytes);
      for (; it_s1 != it_e1; ++it_s1, ++it_s2, ++it_b)
      {
         *it_b = *it_s1;
         *(it_b + 184) = *it_s2;
      }
      battle_.bytes[368] = 0;
      battle_.bytes[369] = 0;
      battle_.bytes[370] = 0;
      battle_.bytes[371] = 0;
      battle_.bytes[372] = 0;
      battle_.bytes[373] = 0;
      battle_.bytes[374] = 0;
      battle_.bytes[375] = 0;
      for (int i = 0; i < 8; ++i)
      {
         battle_.bytes[376 + i] = seed >> 8 * i;
      }
      pkmn_psrng_init(&random, seed);
      this->prob = true;
      // we set prob to 1 in the constructor since we currently do not have probs from engine. pre is working hard on this
   }

   Battle(const Battle &t) : battle_(t.battle_), random(t.random), result(t.result) // TODO
   {
      this->row_actions = t.row_actions;
      this->col_actions = t.col_actions;
      // the base class `PerfectInfoState` is not called, so we don't automatically copy over terminal, etc
      // however the only base data of any use is the actions, and we really only copy it because it cased a bug. Its not needed because in all searches you only copy a state to advance it, and then call apply_actions() and get_actions(), which overwrites everything else 
   }

   void reseed (typename Types::PRNG &device) {
      engine::RBY::set_seed(battle_, device.uniform_64());
      // we can check BattleTypes<> if we really want to confirm that the PRNG type is just prng
   }

   void get_actions()
   {
      const size_t rows = pkmn_gen1_battle_choices(&battle_, PKMN_PLAYER_P1, pkmn_result_p1(result), this->row_actions.template data<pkmn_choice, A<9>::Array>(), PKMN_CHOICES_SIZE);
      const size_t cols = pkmn_gen1_battle_choices(&battle_, PKMN_PLAYER_P2, pkmn_result_p2(result), this->col_actions.template data<pkmn_choice, A<9>::Array>(), PKMN_CHOICES_SIZE);
      this->row_actions.resize(rows);
      this->col_actions.resize(cols);
      // similarly to last function we know the `VectorActions` type is array based, so these last two methods just set the size parameter
   }

   void apply_actions(
       typename Types::Action row_action,
       typename Types::Action col_action)
   {
      result = pkmn_gen1_battle_update(
         &battle_, 
         static_cast<pkmn_choice>(row_action), 
         static_cast<pkmn_choice>(col_action), 
         this->obs.template data<uint8_t, MaxLog>(), 
         MaxLog);
         
      const pkmn_result_kind r = pkmn_result_type(result);
      if (r)
      {
         // set is_terminal and payoff, as indicated
         this->terminal = true;
         if (r == PKMN_RESULT_WIN)
         {
            this->payoff.row_value = typename::Types::Value{Rational(1)};
            // Rational<> has implicit conversions to double, float, and mpq_class, so its a 'universal' initializing type for all Types::Real and Types::Probability values.
            // Value struct in Types assumes constant sum, so we only provide the row payoff.
         }
         else if (r == PKMN_RESULT_LOSE)
         {
            this->payoff = typename::Types::Value{Rational(0)};
         }
         else
         {
            this->payoff.row_value = typename::Types::Value{Rational(1, 2)};
         }
      }
   };
}
```
