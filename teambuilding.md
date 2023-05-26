Sorry I got home and sat down for like an hour thinking about this 
The selection of a team in a battle is most simply thought of as the first matrix node in the game. Both players have the same actions: one action for every possible team.
Much like the rest of SM games, the solution concept here is an equilibrium. There is no such thing as an optimal team but there is such a thing as an optimal prob distro over teams.
Thus one approach for 'team generation' (which tbf could mean MANY things) is to simply find the right distro over the right roster of teams.
I am assuming that the number of teams required for an epsilon nash equilibrium is 
Obviously solving this very large matrix directly is impossible, even if we had the value of its entries.
But the serialized alpha beta paper happened to have an algorithm for solving matrix games by iterative adding to the pool of worthwhile actions. You can even remove an action from the pool as its probability drops below a threashold.
This means that we can maintain a static 'optimal team strategy' over the training run of a strictly playing (no teambuilding) model. This static strategy is the team building model, essentially.

In the paper, candidate actions are added to the pool by iterating through all outsider actions and picking the one thats the best response to the current NE strategy.
We clearly can't just iterate over every possible team, theres too many. So we will exploit the fact that (partially completed) teams have a tree structure. The partial teams are the nodes and an action between them is the determination of some piece of info (zapdos is next poke, spore is breloom's next move)
Now we can use tree search to select a good new team to add to our static pool. Indeed, the completed teams are just terminal nodes and so choosing one is just rolling out a trajectory on this game from initial node to a terminal node.
All games have the payoffs at the terminal nodes, and here the payoff for a completed team is its expected score vs the current NE over the team pool.
Unfortunately we can't know this value exactly, but we can estimate it by having the new team play training games vs the old teams.
We can actually treat this like alpha zero. At each node, we perform a *totally standard* MCTS and then to move to the next node we just pick the team addition that our search most liked.
The only thing to give us pause is what value do we assign leaf nodes when we expand them. They are in fact just a partially completed team, so no eval battles can be performed.
Well like A0 we just can use a NN, and we can update this NN using the terminal value all the same. Alternatively we can just use monte carlo as our model: When we have a leaf node we just fill its team out randomly and have that play our 'teambuilder'. Its all sound because of tree search
[Note: astute reader might notice that this is a single player game, while alpha zero is for 2. The reality is that games like chess can be thought of as single player, and rather than trying to maximize the reward of white/black, they are trying to minimize the exploitability of their play. A0 is really a single player algo!]

Now we have a sound way to generate new candidate teams for our teambuilder. This method has the nice property that `1 terminal node value get` = `1 training game for the battle model`. The team builder leverages model training  for its search and the model is training on better and better teams.

We don't have to stop there though.There has been research re using Directed Acyclic Graphs in game search as a way to strengthen play. This is because DAGs have less nodes than the equivalent tree (no more duplicates for transpositions), so less wasted exploration. In order for this to work though, we we are backpropogating values during tree search we cant just update along the path of selected nodes, we also have to do some additional updating of nodes and edge statistics to account for equivalent paths.

This is where things get speculative but very exciting. I believe we can leverage this extra updating, in addition tothe normal benefits of DAG search, to great effect in team generation.
Firstly, while every tree is technically a DAG, and most real games have transpositions, the Teambuilding "tree" is truly truly a GRAPH. Each node has a number of parents nearly equal to its depth!
This means that node reduction by using the correct data structure is huge. In principle this should result in disproportionate gain compared to the improvements that chess has seen regarding DAG search.
Secondly, the extra updating that is required in a tree search may be a benefit if when it comes to training a NN for teambuilding.
When we are updating the net the samples in the minibatch correspond to nodes that were visited along a trajectory. This is analogous to updating node search stats in vanilla MCTS.
`{} -> {X} -> {X, Y} -> {X, Y, Z}`
