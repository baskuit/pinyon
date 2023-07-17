#include <surskit.hh>

void random_matrix(prng device, const size_t rows, const size_t cols, const int discrete = 20)
{
    const size_t entries = rows * cols;

    std::vector<mpq_class> rpd, cpd;

    Matrix<PairReal<RealType<mpq_class>>> matrix{rows, cols};

    for (int i = 0; i < entries; ++i)
    {
        const int den = device.random_int(discrete) + 1;
        const int row_num = device.random_int(den + 1);
        const int col_num = den - row_num;

        rpd.emplace_back(mpz_class{row_num}, mpz_class{den});
        cpd.emplace_back(mpz_class{col_num}, mpz_class{den});

        matrix[i].row_value = RealType<mpq_class>{rpd[i]};
        matrix[i].col_value = RealType<mpq_class>{cpd[i]};
    }

    Vector<RealType<mpq_class>> row_strategy, col_strategy;
    LRSNash::solve(matrix, row_strategy, col_strategy);

    auto expl = math::exploitability(matrix, row_strategy, col_strategy);
    double expl_ = static_cast<decltype(expl)::type>(expl).get_d();

    if (expl_ > 0)
    {
        exit(1);
    }
}

int main()
{
    prng device{0};

    const int discrete = 20;
    const size_t trials = 100;
    for (int rows = 2; rows <= 9; ++rows)
    {
        for (int cols = 2; cols <= 9; ++cols)
        {
            for (size_t trial = 0; trial < trials; ++trial)
            {
                random_matrix(device, rows, cols, discrete);
            }
        }
    }
};