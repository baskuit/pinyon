#pragma once

#include <state/state.hh>

/*
 Large uniform tree for testing etc. So called because it grows until it can't.
*/

template <size_t size>
struct MoldState : SimpleTypes
{

    class State : public PerfectInfoState<SimpleTypes>
    {
    public:
        size_t max_depth = 1;

        State(size_t max_depth) : max_depth{max_depth}
        {
            this->terminal = (this->max_depth == 0);
            this->init_range_actions(size);
            this->prob = SimpleTypes::Prob{1};
            this->obs = SimpleTypes::Obs{};
        }

        void randomize_transition(
            SimpleTypes::PRNG &device)
        {
        }

        void get_actions()
        {
        }

        void get_actions(
            SimpleTypes::VectorAction &row_actions,
            SimpleTypes::VectorAction &col_actions) const
        {
            row_actions = this->row_actions;
            col_actions = this->col_actions;
        }

        void apply_actions(
            SimpleTypes::Action,
            SimpleTypes::Action)
        {
            --this->max_depth;
            this->terminal = (this->max_depth == 0);
        }

        void get_chance_actions(
            SimpleTypes::Action,
            SimpleTypes::Action,
            std::vector<SimpleTypes::Obs> &chance_actions) const
        {
            chance_actions.resize(1);
        }

        void apply_actions(
            SimpleTypes::Action,
            SimpleTypes::Action,
            SimpleTypes::Obs)
        {
            --this->max_depth;
            this->terminal = (this->max_depth == 0);
        }
    };

    // static_assert(IsStateTypes<MoldState>);
};

struct OneSumMatrixGame : SimpleTypes
{
    class State : public PerfectInfoState<SimpleTypes>
    {
    public:
        const static int den = 10; 
        SimpleTypes::MatrixValue payoff_matrix;

        State(SimpleTypes::PRNG &device, size_t rows, size_t cols)
        {
            this->init_range_actions(rows, cols);
            payoff_matrix.fill(rows, cols);
            for (int i = 0; i < rows * cols; ++i) {
                int x = device.random_int(den) + 1;
                SimpleTypes::Q row_payoff{x, den};
                SimpleTypes::Q col_payoff{den - x, den};
                payoff_matrix[i] = SimpleTypes::Value{row_payoff, col_payoff};
            }

            std::cout << "constr" << std::endl;
            payoff_matrix.print();

        }

        void randomize_transition(
            SimpleTypes::PRNG &device) const
        {
        }

        void get_actions() const
        {
        }

        void get_actions(
            SimpleTypes::VectorAction &row_actions,
            SimpleTypes::VectorAction &col_actions) const
        {
            row_actions = this->row_actions;
            col_actions = this->col_actions;
        }

        void apply_actions(
            SimpleTypes::Action row_action,
            SimpleTypes::Action col_action)
        {
            this->terminal = true;
            this->payoff = payoff_matrix.get(static_cast<int>(row_action), static_cast<int>(col_action));
        }

        void get_chance_actions(
            SimpleTypes::Action,
            SimpleTypes::Action,
            std::vector<SimpleTypes::Obs> &chance_actions) const
        {
            chance_actions.resize(1);
        }

        void apply_actions(
            SimpleTypes::Action row_action,
            SimpleTypes::Action col_action,
            SimpleTypes::Obs)
        {
            this->terminal = true;
            this->payoff = payoff_matrix.get(static_cast<int>(row_action), static_cast<int>(col_action));
        }
    };
};
