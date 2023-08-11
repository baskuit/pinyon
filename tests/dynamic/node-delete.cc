#include <surskit.hh>

using Types = TreeBandit<Exp3<MonteCarloModel<MoldState<2>>>>;

/*

Currently the memory usage increases
when root's are created in the loop. Idk why
*/

int main()
{

    auto state  = W::make_state <Types>(size_t{10});
    auto model  = W::make_model <Types>();
    auto search = W::make_search<Types>();
        W::Types::PRNG device{};

    const size_t n = 1 << 30;
    for (size_t i = 0; i < n; ++i)
    {
        auto root = W::make_root<Types>();
        search.run_for_iterations(1024, device, state, model, root);
        auto x = dynamic_cast<W::Dynamic::MatrixNodeT<TypeListNormalizer::MSearchTypes<Types>> *>(root.ptr.get());
        auto y = x->data;
        // std::cout << y.count_matrix_nodes() << std::endl;
        // std::cout << '!' << std::endl;
        // delete &y;
    }
}