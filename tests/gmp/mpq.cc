#include <surskit.hh>

void random_matrix(prng device, const int rows, const int cols, const int discrete = 20)
{
    const int entries = rows * cols;

    // mpq_t *row_payoff_data = new mpq_t[entries];
    // mpq_t *col_payoff_data = new mpq_t[entries];

    std::vector<mpq_class> rpd, cpd;
    // std::vector<mpq_class> rpd;


    Matrix<PairRat> matrix{rows, cols};

    for (int i = 0; i < entries; ++i)
    {

        const int den = device.random_int(discrete) + 1;
        const int row_num = device.random_int(den + 1);
        const int col_num = den - row_num;
        // mpq_init(row_payoff_data[i]);
        // mpq_init(col_payoff_data[i]);
        rpd.emplace_back(mpz_class{row_num}, mpz_class{den});
        cpd.emplace_back(mpz_class{col_num}, mpz_class{den});
        // mpq_set_ui(row_payoff_data[i], row_num, den);
        // mpq_set_ui(col_payoff_data[i], col_num, den);
        // std::cout << mpq_get_d(row_payoff_data[i]) << ' ' << mpq_get_d(col_payoff_data[i])  << std::endl;
        matrix[i].row_value = mpq_class{rpd[i]};
        matrix[i].col_value = mpq_class{cpd[i]};
    }

    Vector<RealType<mpq_class>> row_strategy, col_strategy;
    LRSNash::solve(matrix, row_strategy, col_strategy);

    auto expl = math::EXPL(matrix, row_strategy, col_strategy);
    double expl_ = expl.unwrap().get_d();

    if (expl_ > 0)
    {
        // matrix.print();
        std::cout << "expl: " << expl.unwrap().get_str() << std::endl;

        // for (int row_idx = 0; row_idx < rows + 2; ++row_idx)
        // {
        //     std::cout << mpz_get_ui(row_solution_data[row_idx]) << ' ';
        // }
        // std::cout << std::endl;
        // for (int col_idx = 0; col_idx < cols + 2; ++col_idx)
        // {
        //     std::cout << mpz_get_ui(col_solution_data[col_idx]) << ' ';
        // }
        // std::cout << std::endl;
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