#pragma once

#include <string>

#include "libsurskit/random.hh"
#include "libsurskit/math.hh"
#include "algorithm.hh"
#include "tree/node.hh"

#include "gambit.h"
#include "solvers/enummixed/enummixed.h"

using namespace Gambit;
using namespace Gambit::Nash;

Game build_nfg(); // TODO see if need forward decs
template <class T>
void PrintCliques(const List<List<MixedStrategyProfile<T>>> &p_cliques,
                  shared_ptr<StrategyProfileRenderer<T>> p_renderer);

template <typename Model>
class MatrixUCB : public Algorithm<Model>
{
public:
    struct MatrixStats : Algorithm<Model>::MatrixStats
    {
        int t = 0;
        Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> cumulative_payoffs;
        Linear::Matrix2D<int, MatrixUCB::state_t::_size> visits;
        std::array<double, MatrixUCB::state_t::_size> strategy0;
        std::array<double, MatrixUCB::state_t::_size> strategy1;
        // When we calculate NE, it's likely that that strategy
        // will be just as effective as the one calculated at the next round.
        // So we store it here.
    };

    struct ChanceStats : Algorithm<Model>::ChanceStats
    {
        int visits = 0;
        double cumulative_value0 = 0;
        double cumulative_value1 = 0;
        double get_expected_value0() { return visits > 0 ? cumulative_value0 / visits : .5; }
        double get_expected_value1() { return visits > 0 ? cumulative_value1 / visits : .5; }
    };

    prng &device;
    EnumMixedStrategySolver<double> solver;
    shared_ptr<StrategyProfileRenderer<double>> renderer;

    double c_uct = 2;
    // solutions to ucb matrix are recalculated if the exploitability is above the threshold.
    double expl_threshold = .05;
    // number of times the above happens.
    int expl_hits = 0;
    // number of times we use bandit solver
    int gambit_hits = 0;


    MatrixUCB(prng &device) : device(device)
    {
        int numDecimals = 10;
        renderer = new MixedStrategyCSVRenderer<double>(std::cout, numDecimals);
    }

    void search(
        int playouts,
        typename MatrixUCB::state_t &state,
        typename MatrixUCB::model_t &model,
        MatrixNode<MatrixUCB> &root)
    {
        root.stats.t = playouts;
        for (int playout = 0; playout < playouts; ++playout)
        {
            auto state_ = state;
            runPlayout(state_, model, &root);
        }
        std::cout << "begin search output" << std::endl;
        Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> bimatrix =
            get_matrix(
                root.stats.cumulative_payoffs,
                root.stats.visits);
        std::cout << "expected value matrix:" << std::endl;
        bimatrix.print();
        std::cout << "visit matrix" << std::endl;
        root.stats.visits.print();
    }

private:
    MatrixNode<MatrixUCB> *runPlayout(
        typename MatrixUCB::state_t &state,
        typename MatrixUCB::model_t &model,
        MatrixNode<MatrixUCB> *matrix_node)
    {

        if (matrix_node->is_terminal == true)
        {
            return matrix_node;
        }
        else
        {

            if (matrix_node->is_expanded == true)
            {
                Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> bimatrix(matrix_node->legal_actions.rows, matrix_node->legal_actions.cols);

                get_ucb_matrix(
                    bimatrix,
                    matrix_node->stats.cumulative_payoffs,
                    matrix_node->stats.visits,
                    matrix_node->stats.t);
                double exploitability = Bandit::exploitability<double, MatrixUCB::state_t::_size>(
                    bimatrix,
                    matrix_node->stats.strategy0,
                    matrix_node->stats.strategy1);
                if (exploitability > this->expl_threshold || false)
                {
                    ++this->expl_hits;
                    solve_bimatrix(bimatrix, matrix_node->stats.strategy0, matrix_node->stats.strategy1);
                    double is_interior = 1.0;
                    for (int i = 0; i < bimatrix.rows; ++i) {
                        if (matrix_node->stats.strategy0[i] == 0){
                            is_interior *= 1 - matrix_node->stats.strategy0[i];
                        }
                    }
                    for (int j = 0; j < bimatrix.cols; ++j) {
                        if (matrix_node->stats.strategy0[j] == 0){
                            is_interior *= 1 - matrix_node->stats.strategy1[j];
                        }
                    }
                    if (is_interior == 0) {
                        ++this->gambit_hits;
                        Bandit::SolveBimatrix<double, MatrixUCB::state_t::_size>(
                            device,
                            10000,
                            bimatrix,
                            matrix_node->stats.strategy0,
                            matrix_node->stats.strategy1
                        );
                    }
                }

                int row_idx = device.sample_pdf<double, MatrixUCB::state_t::_size>(matrix_node->stats.strategy0, matrix_node->legal_actions.rows);
                int col_idx = device.sample_pdf<double, MatrixUCB::state_t::_size>(matrix_node->stats.strategy1, matrix_node->legal_actions.cols);
                typename MatrixUCB::action_t action0 = matrix_node->legal_actions.actions0[row_idx];
                typename MatrixUCB::action_t action1 = matrix_node->legal_actions.actions1[col_idx];
                typename MatrixUCB::transition_data_t transition_data = state.apply_actions(action0, action1);

                ChanceNode<MatrixUCB> *chance_node = matrix_node->access(row_idx, col_idx);
                MatrixNode<MatrixUCB> *matrix_node_next = chance_node->access(transition_data);
                MatrixNode<MatrixUCB> *matrix_node_leaf = runPlayout(state, model, matrix_node_next);

                double u0 = matrix_node_leaf->inference.value0;
                double u1 = matrix_node_leaf->inference.value1;
                update(matrix_node, u0, u1, row_idx, col_idx);
                return matrix_node_leaf;
            }
            else
            {
                expand(state, model, matrix_node);
                return matrix_node;
            }
        }
    }

    void expand(
        typename MatrixUCB::state_t &state,
        Model model,
        MatrixNode<MatrixUCB> *matrix_node)
    {
        matrix_node->is_expanded = true;
        state.get_legal_actions(matrix_node->legal_actions);
        matrix_node->is_terminal = (matrix_node->legal_actions.rows * matrix_node->legal_actions.cols == 0);

        if (matrix_node->is_terminal)
        {
            matrix_node->inference.value0 = state.payoff0;
            matrix_node->inference.value1 = state.payoff1;
        }
        else
        {
            model.inference(state, matrix_node->legal_actions);
            matrix_node->inference = model.last_inference;
        }
        // Initializing matrix node stats
        matrix_node->stats.cumulative_payoffs.rows = matrix_node->legal_actions.rows;
        matrix_node->stats.cumulative_payoffs.cols = matrix_node->legal_actions.cols;
        matrix_node->stats.visits.rows = matrix_node->legal_actions.rows;
        matrix_node->stats.visits.cols = matrix_node->legal_actions.cols;
        for (int row_idx = 0; row_idx < matrix_node->legal_actions.rows; ++row_idx)
        {
            for (int col_idx = 0; col_idx < matrix_node->legal_actions.cols; ++col_idx)
            {
                matrix_node->stats.cumulative_payoffs.set0(row_idx, col_idx, 0);
                matrix_node->stats.cumulative_payoffs.set1(row_idx, col_idx, 0);
                matrix_node->stats.visits.set(row_idx, col_idx, 0);
            }
        }

        // Uniform initialization of stats.strategies
        for (int row_idx = 0; row_idx < matrix_node->legal_actions.rows; ++row_idx)
        {
            matrix_node->stats.strategy0[row_idx] = 1 / (float)matrix_node->legal_actions.rows;
        }
        for (int col_idx = 0; col_idx < matrix_node->legal_actions.cols; ++col_idx)
        {
            matrix_node->stats.strategy1[col_idx] = 1 / (float)matrix_node->legal_actions.cols;
        }

        // estimating t
        ChanceNode<MatrixUCB> *parent = matrix_node->parent;
        if (parent != nullptr)
        {
            MatrixNode<MatrixUCB> *matrix_parent = parent->parent;
            int row_idx = parent->row_idx;
            int col_idx = parent->col_idx;
            // transition prob from matrix_parent to matrix child, factoring all players', including chance, strategies.
            double joint_p = matrix_parent->inference.strategy_prior0[row_idx] * matrix_parent->inference.strategy_prior1[col_idx] * ((double)matrix_node->transition_data.probability);
            int t_estimate = matrix_node->parent->parent->stats.t * joint_p;
            t_estimate = t_estimate < 1 ? 1 : t_estimate;
            matrix_node->stats.t = t_estimate;
        }
    }

    void solve_bimatrix(
        Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> bimatrix,
        std::array<double, MatrixUCB::state_t::_size> &strategy0,
        std::array<double, MatrixUCB::state_t::_size> &strategy1)
    {
        Game game = build_nfg(bimatrix);
        shared_ptr<EnumMixedStrategySolution<double>> solution = solver.SolveDetailed(game); // No exceptino handling 8)
        List<List<MixedStrategyProfile<double>>> cliques = solution->GetCliques();
        MixedStrategyProfile<double> joint_strategy = cliques[1][1];
        for (int i = 0; i < bimatrix.rows; ++i)
        {
            strategy0[i] = joint_strategy[i + 1];
        }
        for (int j = bimatrix.rows; j < bimatrix.rows + bimatrix.cols; ++j)
        {
            strategy1[j - bimatrix.rows] = joint_strategy[j + 1];
        }
        delete game;
    }

    Game build_nfg(
        Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> bimatrix)
    {
        Array<int> dim(2);
        dim[1] = bimatrix.rows;
        dim[2] = bimatrix.cols;

        GameRep *nfg = NewTable(dim);
        // Assigning this to the container assures that, if something goes
        // wrong, the class will automatically be cleaned up.. idk tho
        Game game = nfg;

        StrategyProfileIterator iter(StrategySupportProfile(static_cast<GameRep *>(nfg)));

        for (int i = 0; i < bimatrix.rows; ++i)
        {
            for (int j = 0; j < bimatrix.cols; ++j)
            {
                (*iter)->GetOutcome()->SetPayoff(1, std::to_string(bimatrix.get0(i, j)));
                (*iter)->GetOutcome()->SetPayoff(2, std::to_string(bimatrix.get1(i, j)));
                iter++;
            }
        }

        // game->Write(std::cout);
        return game;
    }

    void update(MatrixNode<MatrixUCB> *matrix_node, double u0, double u1, int row_idx, int col_idx)
    {
        const double v0 = matrix_node->stats.cumulative_payoffs.get0(row_idx, col_idx);
        const double v1 = matrix_node->stats.cumulative_payoffs.get1(row_idx, col_idx);
        const int n = matrix_node->stats.visits.get(row_idx, col_idx);
        matrix_node->stats.cumulative_payoffs.set0(row_idx, col_idx, v0 + u0);
        matrix_node->stats.cumulative_payoffs.set1(row_idx, col_idx, v1 + u1);
        matrix_node->stats.visits.set(row_idx, col_idx, n + 1);
        // TODO add increment overloads
    }
    // Bimatrix of UCB Scores (EV + Exploration)
    void get_ucb_matrix(
        Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> &bimatrix,
        Linear::Bimatrix<double, MatrixUCB::state_t::_size> &cumulative_payoffs,
        Linear::Matrix<int, MatrixUCB::state_t::_size> &visits,
        int t)
    {
        const int rows = bimatrix.rows;
        const int cols = bimatrix.cols;
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                double u = cumulative_payoffs.get0(row_idx, col_idx);
                double v = cumulative_payoffs.get1(row_idx, col_idx);
                int n = visits.get(row_idx, col_idx);
                if (n == 0)
                {
                    n = 1;
                }
                double a = u / n;
                double b = v / n;
                double const eta = this->c_uct * std::sqrt((2 * std::log(t) + std::log(2 * rows * cols)) / n);
                const double x = a + eta;
                const double y = b + eta;
                bimatrix.set0(row_idx, col_idx, x);
                bimatrix.set1(row_idx, col_idx, y);
            }
        }
    }
    Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> get_ucb_matrix(
        Linear::Bimatrix<double, MatrixUCB::state_t::_size> &cumulative_payoffs,
        Linear::Matrix<int, MatrixUCB::state_t::_size> &visits,
        int t)
    {
        Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> bimatrix(visits.rows, visits.cols);
        const int rows = bimatrix.rows;
        const int cols = bimatrix.cols;
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                double u = cumulative_payoffs.get0(row_idx, col_idx);
                double v = cumulative_payoffs.get1(row_idx, col_idx);
                int n = visits.get(row_idx, col_idx);
                if (n == 0)
                {
                    n = 1;
                }
                double a = u / n;
                double b = v / n;
                double const eta = this->c_uct * std::sqrt((2 * std::log(t) + std::log(2 * rows * cols)) / n);
                const double x = a + eta;
                const double y = b + eta;
                bimatrix.set0(row_idx, col_idx, x);
                bimatrix.set1(row_idx, col_idx, y);
            }
        }
        return bimatrix;
    }
    // Bimatrix of EV
    Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> get_matrix(
        Linear::Bimatrix<double, MatrixUCB::state_t::_size> &cumulative_payoffs,
        Linear::Matrix<int, MatrixUCB::state_t::_size> &visits)
    {
        Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> bimatrix(visits.rows, visits.cols);
        const int rows = bimatrix.rows;
        const int cols = bimatrix.cols;
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                double u = cumulative_payoffs.get0(row_idx, col_idx);
                double v = cumulative_payoffs.get1(row_idx, col_idx);
                int n = visits.get(row_idx, col_idx);
                n += 1;
                double a = n > 0 ? u / n : .5;
                double b = n > 0 ? v / n : .5;
                bimatrix.set0(row_idx, col_idx, a);
                bimatrix.set1(row_idx, col_idx, b);
            }
        }
        return bimatrix;
    }
    void get_matrix(
        Linear::Bimatrix<double, MatrixUCB::state_t::_size> &cumulative_payoffs,
        Linear::Matrix<int, MatrixUCB::state_t::_size> &visits,
        Linear::Bimatrix2D<double, MatrixUCB::state_t::_size> &bimatrix)
    {
        const int rows = bimatrix.rows;
        const int cols = bimatrix.cols;
        for (int row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (int col_idx = 0; col_idx < cols; ++col_idx)
            {
                double u = cumulative_payoffs.get0(row_idx, col_idx);
                double v = cumulative_payoffs.get1(row_idx, col_idx);
                int n = visits.get(row_idx, col_idx);
                n += 1;
                double a = n > 0 ? u / n : .5;
                double b = n > 0 ? v / n : .5;
                bimatrix.set0(row_idx, col_idx, a);
                bimatrix.set1(row_idx, col_idx, b);
            }
        }
    }
};

// enummixed.cc helper
template <class T>
void PrintCliques(const List<List<MixedStrategyProfile<T>>> &p_cliques,
                  shared_ptr<StrategyProfileRenderer<T>> p_renderer)
{
    for (int cl = 1; cl <= p_cliques.size(); cl++)
    {

        for (int i = 1; i <= p_cliques[cl].size(); i++)
        {
            p_renderer->Render(p_cliques[cl][i],
                               "convex-" + lexical_cast<std::string>(cl));
                                std::cout << cl << ' ' << i << std::endl;
        }
    }
    std::cout << std::endl;
}