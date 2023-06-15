# What is a State

A state is a two-player game with simultaneous moves and Markovian transitions.

That's it. The `AbstractState` class does not make any more assumptions.

# Perfect Info

In practice, search on general imperfect information games is hard and expensive. Therefore we codify the perfect information requirement and several other niceties with the `PerfectInfoState` class, and we use this class as the basis of all current models, algorithms, and tree structures.

The goal of Surksit is to aid development in an imperfect info game, so eventually we will expand our methods and classes as perfect info milestones are reached.
```cpp
PerfectInfoState::obs;
PerfectInfoState row_actions, col_actions;

```

Following the convention of many other games, a reward is only received at a terminal state. This reward is of `Types::Value` type, stored in the `payoff` member. It's value is only set inside the `apply_actions` method, and is garbage until the state is terminal.

The `Observation` after transitioning and its associated `Probability` are also stored as members of the state, rather than returned by `apply_actions`.

```cpp
    bool is_terminal{false};
    typename Types::VectorAction row_actions{};
    typename Types::VectorAction col_actions{};
    typename Types::Value payoff{};
    typename Types::Observation obs{};
    typename Types::Probability prob{};

    void get_actions();

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action);

    void reseed(typename Types::PRNG &device){};

    void apply_action_indices(
        ActionIndex row_idx,
        ActionIndex col_idx);
```

The methods above are undefined and overridden by methods in derived classes of States. They are merely here to define the interface.

* `get_actions` 
updates the `row_actions`, `col_actions` members of the state. The number of actions for either player can be retrieved by `row_actions.size()` etc, as they are of the `VectorAction` type.
* `apply_actions`
transitions the state, and updates the `obs`, `prob` members.
* `reseed`
**States are deterministic**. Copying a state does not alter the behavior either. In order to transition stochastically, the `reseed` method must be applied first. In tree bandit search,  this is done just after copying the state object provided in the  arguments.

* `apply_action_indices`
TODO Remove?. Unfortunately this method can't be implemented here even though it has the same behavior for all states.

# Test States

### MoldState

This is basically the simplest possible state in implementation. It's main purpose is testing and benchmarking, as a control. It's only member is the `depth` parameter, which determines how many transitions until the state is terminal. It always has the same number of actions for both players, and so the `row_actions`, `col_actions` members are initialized in the `MoldState` constructor and never changed. Thus `get_actions` is a no-op and `apply_actions` merely decrements `depth` and checks if `depth == 0`.


### RandomTree

# ChanceState

# SolvedState

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

   typename Types::PRNG device;
   pkmn_gen1_battle battle_{};
   typename Types::Seed seed = 0;
   pkmn_psrng random = {};
   pkmn_result result = PKMN_RESULT_NONE;
   std::array<pkmn_choice, 9> options{0};

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
   }

   Battle(const Battle &t) : battle_(t.battle_), random(t.random), result(t.result) // TODO
   {
      this->row_actions = t.row_actions;
      this->col_actions = t.col_actions;
   }

   void reseed (typename Types::Seed seed) {
      engine::RBY::set_seed(battle_, seed);
   }

   void get_actions()
   {
      const size_t rows = pkmn_gen1_battle_choices(&battle_, PKMN_PLAYER_P1, pkmn_result_p1(result), this->row_actions.template data<pkmn_choice, A<9>::Array>(), PKMN_CHOICES_SIZE);
      const size_t cols = pkmn_gen1_battle_choices(&battle_, PKMN_PLAYER_P2, pkmn_result_p2(result), this->col_actions.template data<pkmn_choice, A<9>::Array>(), PKMN_CHOICES_SIZE);
      this->row_actions.fill(rows);
      this->col_actions.fill(cols);

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
         this->is_terminal = true;
         if (r == PKMN_RESULT_WIN)
         {
            this->payoff.row_value = typename::Types::Value{Rational(1)};
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
   }
};
```
