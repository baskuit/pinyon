#pragma once

#include <types/matrix.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

#include <memory>

namespace W
{

    template <IsStateTypes>
    struct StateWrapper;
    template <IsValueModelTypes>
    struct ModelWrapper;
    template <IsSearchTypes>
    struct SearchWrapper;

    /*
    State
    */

    struct State
    {
        State() {}
        virtual void get_actions() = 0;
        virtual void get_actions(size_t &rows, size_t &cols) = 0;
        virtual void apply_actions(size_t row_idx, size_t col_idx) = 0;
        virtual bool is_terminal() const = 0;

        virtual bool is_constant_sum() = 0;

        virtual PairReal<double> payoff() = 0;
        virtual double row_payoff() = 0;
        virtual double col_payoff() = 0;

        virtual bool is_solved() = 0;
        virtual void get_payoff_matrix(Matrix<PairReal<double>> &payoff_matrix) = 0;

        template <typename Types>
        std::shared_ptr<typename Types::State> derive_ptr()
        {
            StateWrapper<Types> *self = dynamic_cast<StateWrapper<Types> *>(this);
            return self->ptr;
        }
    };

    template <IsStateTypes Types>
    struct StateWrapper : State
    {
        std::shared_ptr<typename Types::State> ptr;

        StateWrapper(const typename Types::State &state) : ptr{std::make_shared<typename Types::State>(state)} {}

        template <typename... Args>
        StateWrapper(Args... args) : ptr(std::make_shared<typename Types::State>(args...)) {}

        operator typename Types::State()
        {
            return *ptr;
        }

        StateWrapper clone()
        {
            return StateWrapper{*ptr};
        }

        void get_actions()
        {
            ptr->get_actions();
        }
        void get_actions(size_t &rows, size_t &cols)
        {
            ptr->get_actions();
            rows = ptr->row_actions.size();
            cols = ptr->col_actions.size();
        };
        void apply_actions(size_t row_idx, size_t col_idx)
        {
            if (row_idx < ptr->row_actions.size() && col_idx < ptr->col_actions.size())
            {
                ptr->apply_actions(ptr->row_actions[row_idx], ptr->col_actions[col_idx]);
            }
            else
            {
                throw(std::exception());
            }
        };
        bool is_terminal() const
        {
            return ptr->is_terminal();
        };

        bool is_constant_sum()
        {
            // return Types::Value::IS_CONSTANT_SUM;
            return false;
        };
        PairReal<double> payoff()
        {
            // TODODODODODOOD
            // return PairReal<double>{
            //     static_cast<double>(ptr->payoff.get_row_value()), 
            //     static_cast<double>(ptr->payoff.get_col_value())};
            return PairReal<double>{0, 0};
        }
        double row_payoff()
        {
            // return static_cast<double>(ptr->payoff.get_row_value());
            return 0;
        };
        double col_payoff()
        {
            // return static_cast<double>(ptr->payoff.get_col_value());
            return 0;
        };
        bool is_solved()
        {
            return false;
        };
        std::vector<double> row_strategy()
        {
            return std::vector<double>{};
        };
        std::vector<double> col_strategy()
        {
            return std::vector<double>{};
        };
        void get_payoff_matrix(Matrix<PairReal<double>> &payoff_matrix)
        {
            // if constexpr (std::derived_from<typename Types::State, SolvedState<typename Types>>)
            // {
            //     typename Types::MatrixValue matrix;
            //     ptr->get_payoff_matrix(matrix);
            //     payoff_matrix = matrix;
            //     // payoff_matrix.rows = matrix.rows;
            //     // payoff_matrix.cols = matrix.cols;
            //     // size_t entry_idx = 0;
            //     // for (auto value : matrix)
            //     // {
            //     //     payoff_matrix[entry_idx].row_value = static_cast<double>(value.get_row_payoff());
            //     //     payoff_matrix[entry_idx].col_value = static_cast<double>(value.get_col_payoff());
            //     //     ++entry_idx;
            //     // }
            // }
        }
    };

    /*
    Model
    */

    struct Model
    {
        struct Output
        {
            std::vector<double> row_policy, col_policy;
            double row_value, col_value;
        };
        virtual Output get_inference(State &state) = 0;
        template <typename Types>
        std::shared_ptr<typename Types::Model> derive_ptr()
        {
            ModelWrapper<Types> *self = dynamic_cast<ModelWrapper<Types> *>(this);
            return self->ptr;
        }
    };

    template <IsValueModelTypes Types>
    struct ModelWrapper : Model
    {
        std::shared_ptr<typename Types::Model> ptr;
        typename Types::Model::ModelOutput output{};
        ModelWrapper(const typename Types::Model &model) : ptr{std::make_shared<typename Types::Model>(model)} {}
        ModelWrapper(const typename Types::Model &&model) : ptr{std::make_shared<typename Types::Model>(model)} {}
        ModelWrapper(const ModelWrapper &model_wrapper) : ptr{model_wrapper.ptr} {}
        template <typename... Args>
        ModelWrapper(Args... args) : ptr(std::make_shared<typename Types::Model>(args...)) {}

        operator typename Types::Model()
        {
            return *ptr;
        }

        Output get_inference(State &state)
        {
            auto raw_state = *state.derive_ptr<Types>();
            ptr->get_inference(raw_state, output);
            return Output{std::vector<double>{}, std::vector<double>{}, 0, 0};
        };
    };

    /*
    Algorithm
    */

    struct Search
    {
        struct TreeData
        {
            // num nodes, branch factor etc
            // dont specialize into functions because its calculated by traversing tree
            // thus why not calculate everything. faster if we every need more than one piece of info..
        };

        TreeData tree_data;
        virtual Search *clone() = 0;
        virtual void run(size_t iterations, State &state, Model &model) = 0;
        virtual void run_and_get_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy, size_t iterations, State &state, Model &model) = 0;
        virtual double exploitability(State &state) = 0;
        virtual double exploitability(Matrix<PairReal<double>> &matrix) = 0;
        virtual void get_empirical_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy) = 0;
        // virtual std::vector<double> row_strategy() = 0;
        // virtual std::vector<double> col_strategy() = 0;
        virtual void reset() = 0;
        template <typename Types>
        std::shared_ptr<typename Types::Search> derive_ptr()
        {
            SearchWrapper<Types> *self = dynamic_cast<SearchWrapper<Types> *>(this);
            return self->ptr;
        }
    };

    template <IsSearchTypes Types>
    struct SearchWrapper : Search
    {
        std::shared_ptr<typename Types::Search> ptr;
        std::shared_ptr<typename Types::MatrixNode> root;

        SearchWrapper(const typename Types::Search &session) : ptr{std::make_shared<typename Types::Search>(session)}, root{std::make_shared<typename Types::MatrixNode>()} {}

        template <typename... Args>
        SearchWrapper(Args... args) : ptr(std::make_shared<typename Types::Search>(args...)), root{std::make_shared<typename Types::MatrixNode>()} {}

        Search *clone()
        {
            return new SearchWrapper{*ptr};
        }

        void run(size_t iterations, State &state, Model &model)
        {
            auto state_ptr = state.derive_ptr<Types>();
            auto model_ptr = model.derive_ptr<Types>();
            typename Types::PRNG device{};
            ptr->run(iterations, device, *state_ptr, *model_ptr, *root);
        }

        void run_and_get_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy, size_t iterations, State &state, Model &model)
        {
            auto state_ptr = state.derive_ptr<Types>();
            auto model_ptr = model.derive_ptr<Types>();
            typename Types::PRNG device{};
            typename Types::MatrixNode root_temp;
            ptr->run(iterations, device, *state_ptr, *model_ptr, root_temp);

            row_strategy.clear();
            col_strategy.clear();
            typename Types::VectorReal r, c;
            ptr->get_empirical_strategies(root_temp.stats, r, c);
            for (int i = 0; i < r.size(); ++i)
            {
                row_strategy.push_back(static_cast<double>(r[i]));
            }
            for (int i = 0; i < c.size(); ++i)
            {
                col_strategy.push_back(static_cast<double>(c[i]));
            }
        }

        void reset()
        {
            root = std::make_shared<typename Types::MatrixNode>();
        }

        void get_empirical_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy)
        {
            row_strategy.clear();
            col_strategy.clear();
            typename Types::VectorReal r, c;
            ptr->get_empirical_strategies(root->stats, r, c);
            for (int i = 0; i < r.size(); ++i)
            {
                row_strategy.push_back(static_cast<double>(r[i]));
            }
            for (int i = 0; i < c.size(); ++i)
            {
                col_strategy.push_back(static_cast<double>(c[i]));
            }
        };
        void get_refined_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy){};
        void get_emprical_value(double &row_value, double &col_value)
        {
            auto x = ptr->get_empirical_value(&*root);
        };
        double exploitability(State &state)
        {
            typename Types::VectorReal r, c;
            ptr->get_empirical_strategies(root->stats, r, c);
            Matrix<PairReal<double>> matrix;
            state.get_payoff_matrix(matrix);
            double expl = static_cast<double>(math::exploitability(matrix, r, c));
            return expl;
        }; // aka expected regret because we use empirical strategies

        double exploitability(Matrix<PairReal<double>> &matrix)
        {
            typename Types::VectorReal r, c;
            ptr->get_empirical_strategies(root->stats, r, c);
            double expl = static_cast<double>(math::exploitability(matrix, r, c));
            return expl;
        }
    };
};

// State, Model, Search, update_model(state = inference...)?