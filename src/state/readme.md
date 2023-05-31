# Perfect Info

The default state represents a perfect info game

IS_CONSTANT_SUM is used mostly in the algorithm level in order to specialize to memory effecient implementations (not using extra matrices etc.)

MAX, MIN_PAYOFF are used by the Alpha-Beta algorithm to set its bound. They also determine PAYOFF_SUM which is needed to infer the column players value.

a Value struct means that the model level can ignore whether the state is constant sum.
