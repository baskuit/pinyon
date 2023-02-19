#pragma once

#include "../libsurskit/math.hh"

template <typename Algorithm>
class ChanceNode;

// Matrix Node

template <typename Algorithm>
class MatrixNode
{
public:
    using state_t = typename Algorithm::state_t;
    using action_t = typename Algorithm::action_t;
    using pair_actions_t = typename Algorithm::pair_actions_t;
    using transition_data_t = typename Algorithm::transition_data_t;
    using model_t = typename Algorithm::model_t;
    using inference_t = typename Algorithm::inference_t;
    using stats_t = typename Algorithm::MatrixStats;

    ChanceNode<Algorithm> *parent = nullptr;
    ChanceNode<Algorithm> *child = nullptr;
    MatrixNode<Algorithm> *prev = nullptr;
    MatrixNode<Algorithm> *next = nullptr;

    transition_data_t transition_data;

    bool is_terminal = false;
    bool is_expanded = false;

    pair_actions_t legal_actions;
    inference_t inference;
    stats_t stats; // cumulative_value, vists now part of stats

    MatrixNode() : transition_data() {}
    MatrixNode(ChanceNode<Algorithm> *parent, MatrixNode<Algorithm> *prev, transition_data_t transition_data) : parent(parent), prev(prev), transition_data(transition_data) {}
    ~MatrixNode();

    ChanceNode<Algorithm> *access(int row_idx, int col_idx)
    {
        if (this->child == nullptr)
        {
            ChanceNode<Algorithm> *child = new ChanceNode<Algorithm>(this, nullptr, row_idx, col_idx);
            this->child = child;
            return child;
        }
        ChanceNode<Algorithm> *current = this->child;
        ChanceNode<Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->row_idx == row_idx && current->col_idx == col_idx)
            {
                return current;
            }
            current = current->next;
        }
        ChanceNode<Algorithm> *child = new ChanceNode<Algorithm>(this, previous, row_idx, col_idx);
        previous->next = child;
        return child;
    };

    void make_terminal()
    {
        while (child != nullptr)
        {
            delete child;
        }
        is_terminal = true;
    }

    int count()
    {
        int c = 1;
        ChanceNode<Algorithm> *current = child;
        while (current != nullptr)
        {
            c += current->count();
            current = current->next;
        }
        return c;
    }

    void matrix(Linear::Matrix<double, state_t::size_> &M)
    {
        auto child = this->child;
        while (child != nullptr)
        {
            M.set(child->row_idx, child->col_idx, child->stats.get_expected_value0());
            child = child->next;
        }
    }
};

// Chance Node

template <typename Algorithm>
class ChanceNode
{
public:
    using state_t = typename Algorithm::state_t;
    using action_t = typename Algorithm::action_t;
    using pair_actions_t = typename Algorithm::pair_actions_t;
    using transition_data_t = typename Algorithm::transition_data_t;
    using model_t = typename Algorithm::model_t;
    using inference_data_t = typename Algorithm::inference_t;
    using stats_t = typename Algorithm::ChanceStats;

    MatrixNode<Algorithm> *parent = nullptr;
    MatrixNode<Algorithm> *child = nullptr;
    ChanceNode<Algorithm> *prev = nullptr;
    ChanceNode<Algorithm> *next = nullptr;

    int row_idx;
    int col_idx;

    stats_t stats;

    ChanceNode<Algorithm>(MatrixNode<Algorithm> *parent, ChanceNode<Algorithm> *prev, int row_idx, int col_idx) : parent(parent), prev(prev), row_idx(row_idx), col_idx(col_idx) {}
    ~ChanceNode<Algorithm>();

    MatrixNode<Algorithm> *access(transition_data_t data)
    {
        if (this->child == nullptr)
        {
            MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>(this, nullptr, data);
            this->child = child;
            return child;
        }
        MatrixNode<Algorithm> *current = this->child;
        MatrixNode<Algorithm> *previous = this->child;
        while (current != nullptr)
        {
            previous = current;
            if (current->transition_data.key == data.key)
            {
                return current;
            }
            current = current->next;
        }
        MatrixNode<Algorithm> *child = new MatrixNode<Algorithm>(this, previous, data);
        previous->next = child;
        return child;
    };

    int count()
    {
        int c = 0;
        MatrixNode<Algorithm> *current = child;
        while (current != nullptr)
        {
            c += current->count();
            current = current->next;
        }
        return c;
    }

    Rational get_explored_total () {
           Rational total(0, 1);
           MatrixNode<Algorithm> cur = child;
           while (cur != nullptr) {
                total += cur.transition_data.probability;
                cur = cur->next;
           }
           return total;
    }
};

// We have to hold off on destructor definitions until here

template <typename Algorithm>
MatrixNode<Algorithm>::~MatrixNode<Algorithm>()
{

    while (child != nullptr)
    {
        ChanceNode<Algorithm> *victim = child;
        child = child->next;
        delete victim;
    }
    if (prev != nullptr)
    {
        prev->next = next;
    }
    else if (parent != nullptr)
    {
        parent->child = next;
    }
}

template <typename Algorithm>
ChanceNode<Algorithm>::~ChanceNode<Algorithm>()
{
    while (child != nullptr)
    {
        MatrixNode<Algorithm> *victim = child;
        child = child->next;
        delete victim;
    }
    if (prev != nullptr)
    {
        prev->next = next;
    }
    else if (parent != nullptr)
    {
        parent->child = next;
    }
};