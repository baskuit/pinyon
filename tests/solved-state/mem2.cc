
#include <surskit.hh>

// using UnsolvedStateTypes = MoldState<2>;
using UnsolvedStateTypes = RandomTree<RandomTreeRationalTypes>;
// using SolvedStateTypes = TraversedState<EmptyModel<UnsolvedStateTypes>>;

prng a_device{};

int main()
{
    for (size_t i = 0; i < (1 << 2); ++i)
    {
        UnsolvedStateTypes::MatrixValue matrix;
        matrix.fill(3, 3, {});
        UnsolvedStateTypes::VectorReal r, c;
        LRSNash::solve(matrix, r, c);
        
        // FT_::PRNG device{0};
        // solver.run_for_iterations(100, device, state, model, root);

    }

    return 0;
}
