#include "state/state.hh"
#include "solvers/enummixed/enummixed.h"

/*
TreeInfo
*/

template <int _size>
class SeedState : public StateArray<_size, int, int, double>
{
public:
    struct Types : StateArray<_size, int, int, double>::Types
    {
        static const int size = _size;
    };
    int depth_bound = 0;
    int rows = _size;
    int cols = _size;

    prng &device;

    SeedState(prng &device, int depth_bound, int rows, int cols) : device(device), depth_bound(depth_bound), rows(rows), cols(cols)
    {
        for (int i = 0; i < _size; ++i)
        {
            this->actions.row_actions[i] = i;
            this->actions.col_actions[i] = i;
        }
        this->transition.prob = 1;
        this->transition.obs = 0;
    }

    void get_actions()
    {
        this->actions.rows = rows;
        this->actions.cols = cols;
    }

    void apply_actions(int row_action, int col_action)
    {
        depth_bound -= 1; //+ this->device.random_int(2);
        depth_bound *= depth_bound >= 0;
        if (depth_bound == 0)
        {
            this->row_payoff = this->device.random_int(2);
            this->col_payoff = 1.0 - this->row_payoff;
            rows = 0;
            cols = 0;
            this->is_terminal = true; // TODO where?
        }
        else
        {
            // More random stuff
        }
    }

private:
    // Functions to improve randomness
};

/*
Recursive Grow algorithm
*/

template <typename Model>
class Grow : public AbstractAlgorithm<Model>
{
    static_assert(std::derived_from<typename Model::Types::State, SeedState<Model::Types::size>>);
    // Model::State is based on SeedState

public:
    struct MatrixStats;
    struct Types : AbstractAlgorithm<Model>::Types
    {
        using MatrixStats = Grow::MatrixStats;
    };
    struct MatrixStats : AbstractAlgorithm<Model>::MatrixStats
    {
        bool grown = false;
        typename Types::Real payoff = 0;
        typename Types::MatrixReal expected_value;
        typename Types::VectorReal row_strategy = {0};
        typename Types::VectorReal col_strategy = {0};
    };

    using Solver = Gambit::Nash::EnumMixedStrategySolver<double>;
    using Solution = Gambit::List<Gambit::List<Gambit::MixedStrategyProfile<double>>>;

    prng &device;
    Solver solver;
    // bool require_interior = false; TODO unused cus no Bandit::

    Grow(prng &device) : device(device)
    {
    }

    void grow(
        typename Types::State &state,
        MatrixNode<Grow> *matrix_node)
    {
        if (matrix_node->stats.grown)
        {
            return;
        }

        state.get_actions();
        const int rows = state.actions.rows;
        const int cols = state.actions.cols;
        matrix_node->stats.expected_value.rows = rows;
        matrix_node->stats.expected_value.cols = cols;
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                ChanceNode<Grow> *chance_node = matrix_node->access(i, j);
                for (int c = 0; c < 1; ++c)
                {
                    typename Types::State state_ = state;
                    state_.apply_actions(i, j);
                    MatrixNode<Grow> *matrix_node_next = chance_node->access(state.transition);
                    grow(state_, matrix_node_next);
                    matrix_node->stats.expected_value.data[i][j] = matrix_node_next->stats.payoff;
                }
            }
        }

        // TODO check that all the tree stats are being updated properly
        if (rows * cols == 0)
        {
            matrix_node->stats.payoff = state.row_payoff;
            matrix_node->is_terminal = true;
        }
        else
        {
            solve_matrix(
                matrix_node->stats.expected_value,
                matrix_node->stats.row_strategy,
                matrix_node->stats.col_strategy);
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    matrix_node->stats.payoff +=
                        matrix_node->stats.row_strategy[i] *
                        matrix_node->stats.col_strategy[j] *
                        matrix_node->stats.expected_value.data[i][j];
                }
            }
        }
        matrix_node->stats.grown = true;
    }

private:
    void solve_matrix(
        typename Types::MatrixReal &matrix,
        typename Types::VectorReal &row_strategy,
        typename Types::VectorReal &col_strategy)
    {
        Gambit::Game game = build_nfg(matrix);
        Gambit::shared_ptr<Gambit::Nash::EnumMixedStrategySolution<double>> solution = solver.SolveDetailed(game); // No exceptino handling 8)
        Solution cliques = solution->GetCliques();
        Gambit::MixedStrategyProfile<double> joint_strategy = cliques[1][1];
        double is_interior = 1.0;
        for (int i = 0; i < matrix.rows; ++i)
        {
            row_strategy[i] = joint_strategy[i + 1];
            is_interior *= 1 - row_strategy[i];
        }
        for (int j = matrix.rows; j < matrix.rows + matrix.cols; ++j)
        {
            col_strategy[j - matrix.rows] = joint_strategy[j + 1];
            is_interior *= 1 - col_strategy[j];
        }

        // if (is_interior == 0 && this->require_interior)
        // {
        //     Bandit::SolveBimatrix<double, Grow::state_t::_size>(
        //         this->device,
        //         10000,
        //         matrix,
        //         row_strategy,
        //         col_strategy);
        // }
        delete game;
    }

    Gambit::Game build_nfg(
        typename Types::MatrixReal &matrix)
    {
        Gambit::Array<int> dim(2);
        dim[1] = matrix.rows;
        dim[2] = matrix.cols;
        Gambit::GameRep *nfg = NewTable(dim);
        Gambit::Game game = nfg;
        Gambit::StrategyProfileIterator iter(Gambit::StrategySupportProfile(static_cast<Gambit::GameRep *>(nfg)));
        for (int j = 0; j < matrix.cols; ++j)
        {
            for (int i = 0; i < matrix.rows; ++i)
            {
                const typename Types::Real x = matrix.data[i][j];
                (*iter)->GetOutcome()->SetPayoff(1, std::to_string(x));
                (*iter)->GetOutcome()->SetPayoff(2, std::to_string(1 - x));
                iter++;
            }
        }
        return game;
    }
};

/*
Random Tree
*/

template <int size>
class TreeState : public SolvedStateArray<size, int, int, double>
{
public:

    using SeedStateNode = MatrixNode<Grow<MonteCarloModel<SeedState<size>>>>;

    struct Types : SolvedStateArray<size, int, int, double>::Types
    {
    };

    SeedState<size> seed;
    std::shared_ptr<SeedStateNode> root;
    SeedStateNode *current;
    // Trying out make shared to keep copy assignment but also have automatic memory management

    TreeState(prng &device, int depth_bound, int rows, int cols) : seed(SeedState<size>(device, depth_bound, rows, cols))
    {
        this->root = std::make_shared<SeedStateNode>();
        this->current = &*root;
        Grow<MonteCarloModel<SeedState<size>>> session(device);
        session.grow(seed, current);
        this->row_strategy = current->stats.row_strategy;
        this->col_strategy = current->stats.col_strategy;
    }
};