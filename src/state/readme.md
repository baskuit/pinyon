# Perfect Info

The default state represents a perfect info game

IS_CONSTANT_SUM is used mostly in the algorithm level in order to specialize to memory effecient implementations (not using col value matrices etc.)

MAX, MIN_PAYOFF are used by the Alpha-Beta algorithm to set its initial bounds. They also determine PAYOFF_SUM which is needed to infer the column players value.

a Value struct means that the model level can ignore whether the state is constant sum.

# Imperfect Info

This may *eventually* be supported, after the perfect info setting has been explored. Doing it in a pricipled way would likely require counterfactual regret.

Otherwise, I do have one idea for lookead in one-sided imperfect info setting that is less principled but much easier to implement.