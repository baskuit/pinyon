#include <surskit.hh>

int main()
{

    auto types = type_list_cart_prod<RandomTree>(type_list<RandomTreeFloatTypes>{});
    auto bandit_types = std::make_tuple(Exp3<MonteCarloModel<MoldState<2>>>{});
    auto bandit_init = std::make_tuple(Exp3<MonteCarloModel<MoldState<2>>>::BanditAlgorithm{});
    auto node_pack = NodePack<DefaultNodes>{};
    auto tuple = algorithm_generator<TreeBandit>(bandit_types, bandit_init, node_pack);
}