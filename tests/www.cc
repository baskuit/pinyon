#include <surskit.hh>

using Types = TreeBandit<Exp3<MonteCarloModel<MoldState<2>>>>;

int main() {
    W::Types::State state{Types{}, typename Types::State{10}};
    W::Types::Model model{Types{}, uint64_t{}};
    W::Types::Search search{Types{}, typename Types::Search{}};

    W::Types::ModelOutput output;
    model.get_inference(state, output);

}