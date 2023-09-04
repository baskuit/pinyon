#include <pinyon.hh>

int main()
{
    auto types = type_list_cart_prod<RandomTree>(TypePack<RandomTreeFloatTypes>{});
    auto bandit_types = std::make_tuple(Exp3<MonteCarloModel<MoldState<>>>{});
    auto bandit_init = std::make_tuple(Exp3<MonteCarloModel<MoldState<>>>::BanditAlgorithm{});
    auto node_template_pack = NodeTemplatePack<DefaultNodes>{};

    auto search_tuple = algorithm_generator<TreeBandit>(bandit_types, bandit_init, node_template_pack);
    auto t = std::get<0>(bandit_types);
    decltype(t)::State mold_state{2, 2};
}