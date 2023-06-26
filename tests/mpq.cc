#include <surskit.hh>

int main () {

    Matrix<PairRational> x{2, 2};

    std::vector<mpq_class> row_strategy, col_strategy;

    x[0] = mpq_class{2, 3};
    x[1] =  mpq_class{4, 5};
    x[2] =  mpq_class{3, 2};
    x[3] =  mpq_class{1, 7};

    LRSNash::solve_matrix(x, row_strategy, col_strategy);

    for (int row_idx = 0; row_idx < 2; ++row_idx) {
        std::cout << row_strategy[row_idx] << std::endl;
    }

};