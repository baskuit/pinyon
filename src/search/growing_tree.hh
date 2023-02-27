#pragma once

#include "algorithm.hh"
#include "tree/node.hh"

template <typename Model>
class GrowingTree : public Algorithm<Model>
{
public:
    struct MatrixStats : Algorithm<Model>::MatrixStats
    {
    };

    struct ChanceStats : Algorithm<Model>::ChanceStats
    {
    };


    void run() {}

    void run(
        int playouts,
        typename GrowingTree::State &state,
        typename GrowingTree::Model &model,
        MatrixNode<GrowingTree> &root)
    {
        for (int playout = 0; playout < playouts; ++playout)
        {
            auto state_copy = state;
            this->playout(state_copy, model, &root);
        }

private:
    MatrixNode<GrowingTree> *playout(
        typename GrowingTree::State &state,
        Model &model,
        MatrixNode<GrowingTree> *matrix_node)
    {
        if (matrix_node->is_terminal == true)
        {
            return matrix_node;
        }
        else
        {
            if (matrix_node->is_expanded == true)
            {
this->forecast(matrix_node);
double row_inverse_prob = 1 / this->row_forecast[row_idx];
double col_inverse_prob = 1 / this->col_forecast[col_idx];
// get forecast (exp3p, solve with gambit for MatrixUCB)

                int row_idx = device.sample_pdf<typename GrowingTree::VectorDouble>(this->row_forecast, matrix_node->pair_actions.rows);
                int col_idx = device.sample_pdf<typename GrowingTree::VectorDouble>(this->col_forecast, matrix_node->pair_actions.cols);
                typename GrowingTree::PlayerAction row_action = matrix_node->pair_actions.row_actions[row_idx];
                typename GrowingTree::PlayerAction col_action = matrix_node->pair_actions.col_actions[col_idx];
                state.apply_actions(row_action, col_action);

                ChanceNode<GrowingTree> *chance_node = matrix_node->access(row_idx, col_idx);
                MatrixNode<GrowingTree> *matrix_node_next = chance_node->access(state.transition_data);
                MatrixNode<GrowingTree> *matrix_node_leaf = this->playout(state, model, matrix_node_next);

// Update node
double row_payoff = matrix_node_leaf->inference_data.row_value;
double col_payoff = matrix_node_leaf->inference_data.col_value;

this->update_matrix_node(
    matrix_node,
    row_payoff, col_payoff,
    row_idx, col_idx,
    row_inverse_prob, col_inverse_prob);
this->update_chance_node(chance_node, row_payoff, col_payoff);

                return matrix_node_leaf;
            }
            else
            {
                this->expand(state, model, matrix_node);
                return matrix_node;
            }
        }
    };

    inline void expand(
        typename GrowingTree::State &state,
        Model &model,
        MatrixNode<GrowingTree> *matrix_node)
    {
        matrix_node->is_expanded = true;
        state.get_player_actions();
        matrix_node->is_terminal = state.is_terminal;
        matrix_node->pair_actions = state.pair_actions;

        if (matrix_node->is_terminal)
        {
            matrix_node->inference_data.row_value = state.row_payoff;
            matrix_node->inference_data.col_value = state.col_payoff;
        }
        else
        {
            model.inference(state);
            matrix_node->inference_data = model.inference_data;
        }

// Do more stuff (time stuff for Exp3p, TODO what does MatrixUCB do)
    }

    inline void forecast(MatrixNode<GrowingTree> *matrix_node)
    {
    }


    inline void update_matrix_node(
        MatrixNode<GrowingTree> *matrix_node,
        double row_payoff, double col_payoff,
        int row_idx, int col_idx,
        double row_inverse_prob, double col_inverse_prob)
    {
        matrix_node->stats.row_value_total += row_payoff;
        matrix_node->stats.col_value_total += col_payoff;
        matrix_node->stats.visits += 1;
        matrix_node->stats.row_visits[row_idx] += 1;
        matrix_node->stats.col_visits[col_idx] += 1;
        const double rows = matrix_node->pair_actions.rows;
        const double cols = matrix_node->pair_actions.cols;
        const double time = matrix_node->stats.t;
        const double row_beta = std::sqrt(std::log(rows) / (double)(time * rows));
        const double col_beta = std::sqrt(std::log(cols) / (double)(time * cols));
        matrix_node->stats.row_gains[row_idx] += row_beta * row_inverse_prob;
        matrix_node->stats.row_gains[row_idx] += col_beta * row_inverse_prob;
    }

    inline void update_chance_node(ChanceNode<GrowingTree> *chance_node, double row_payoff, double col_payoff)
    {
        chance_node->stats.row_value_total += row_payoff;
        chance_node->stats.col_value_total += col_payoff;
        chance_node->stats.visits += 1;
    }
};