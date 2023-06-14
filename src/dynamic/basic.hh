#pragma once

#include <memory>
#include <vector>
#include <string>
#include <any>
#include <unordered_map>

#include <types/matrix.hh>
#include <tree/tree.hh>

#define SEARCH_PARAMS(State, Model, BanditAlgorithm, TreeBandit)                                            \
    template <                                                                                              \
        class State,                                                                                        \
        template <                                                                                          \
            class>                                                                                          \
        class Model,                                                                                        \
        template <                                                                                          \
            class,                                                                                          \
            template <class, class, template <class> class, template <class> class, template <class> class> \
            class,                                                                                          \
            template <class> class,                                                                         \
            template <class> class>                                                                         \
        class BanditAlgorithm,                                                                              \
        template <                                                                                          \
            class,                                                                                          \
            class,                                                                                          \
            template <class> class,                                                                         \
            template <class> class,                                                                         \
            template <class> class>                                                                         \
        class TreeBandit>

namespace W
{

    template <typename>
    struct StateWrapper;
    template <typename>
    struct ModelWrapper;
    template <typename>
    struct SearchWrapper;

    struct State
    {
        State() {}
        virtual void get_actions() = 0;
        virtual void get_actions(size_t &rows, size_t &cols) = 0;
        virtual void apply_actions(int row_idx, int col_idx) = 0;
        virtual bool is_terminal() = 0;
        virtual double row_payoff() = 0;
        virtual double col_payoff() = 0;
        virtual bool is_solved() = 0;
        virtual void get_payoff_matrix(Matrix<PairDouble> &payoff_matrix) = 0;

        template <typename T>
        std::shared_ptr<T> deref()
        {
            StateWrapper<T> *self = dynamic_cast<StateWrapper<T> *>(this);
            return self->ptr;
        }
    };
    template <class State>
    struct StateGenerator
    {
        StateGenerator() {}
        virtual StateWrapper<State> operator*() = 0;
    };
    struct Model
    {
        struct Output
        {
            std::vector<double> row_policy, col_policy;
            double row_value, col_value;
        };
        virtual Output get_inference(State &state) = 0;
        template <typename T>
        std::shared_ptr<T> deref()
        {
            ModelWrapper<T> *self = dynamic_cast<ModelWrapper<T> *>(this);
            return self->ptr;
        }
    };
    struct Search // = Algorithm + Tree
    {
        struct TreeData
        {
            // num nodes, branch factor etc
            // dont specialize into functions because its calculated by traversing tree
            // thus why not calculate everything. faster if we every need more than one piece of info..
        };

        TreeData tree_data;
        virtual void run(size_t iterations, State &state, Model &model) = 0;
        virtual double exploitability(State &state) = 0;
        virtual void get_empirical_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy) = 0;
        // virtual std::vector<double> row_strategy() = 0;
        // virtual std::vector<double> col_strategy() = 0;
        virtual void reset() = 0;
        template <typename T>
        std::shared_ptr<T> deref()
        {
            SearchWrapper<T> *self = dynamic_cast<SearchWrapper<T> *>(this);
            return self->ptr;
        }
        // expensive to calculate, so store as member
    };

    /*

    Derived - Type Specific Wrappers

    */

    template <typename _State>
    struct StateWrapper : State
    {
        std::shared_ptr<_State> ptr;
        StateWrapper(const _State &state) : ptr{std::make_shared<_State>(state)} {}
        StateWrapper(const _State &&state) : ptr{std::make_shared<_State>(state)} {}
        StateWrapper(const StateWrapper &state_wrapper) : ptr{state_wrapper.ptr} {}
        template <typename... Args>
        StateWrapper(Args... args) : ptr(std::make_shared<_State>(args...)) {}

        operator _State()
        {
            return *ptr;
        }

        StateWrapper clone()
        {
            return StateWrapper{*ptr};
        }

        bool is_constant_sum()
        {
            return typename _State::Types::Value::IS_CONSTANT_SUM();
        };
        double payoff_sum()
        {
            return typename _State::Types::Value::PAYOFF_SUM();
        };

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
        void apply_actions(int row_idx, int col_idx)
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
        bool is_terminal()
        {
            return ptr->is_terminal;
        };
        double row_payoff()
        {
            return ptr->payoff.get_row_value();
        };
        double col_payoff()
        {
            return ptr->payoff.get_col_value();
        };
        bool is_solved()
        {
            return std::derived_from<_State, SolvedState<typename _State::Types>>;
        };
        std::vector<double> row_strategy()
        {
            return std::vector<double>{};
        };
        std::vector<double> col_strategy()
        {
            return std::vector<double>{};
        };
        void get_payoff_matrix(Matrix<PairDouble> &payoff_matrix)
        {
            if constexpr (std::derived_from<_State, SolvedState<typename _State::Types>>)
            {
                typename _State::Types::MatrixValue matrix;
                ptr->get_payoff_matrix(matrix);
                payoff_matrix.rows = matrix.rows;
                payoff_matrix.cols = matrix.cols;
                size_t entry_idx = 0;
                for (auto value : matrix)
                {
                    payoff_matrix[entry_idx].row_value = static_cast<double>(value.get_row_payoff());
                    payoff_matrix[entry_idx].col_value = static_cast<double>(value.get_col_payoff());
                    ++entry_idx;
                }
            }
        }
    };

    template <typename _Model>
    struct ModelWrapper : Model
    {
        std::shared_ptr<_Model> ptr;
        typename _Model::ModelOutput output{};
        ModelWrapper(const _Model &model) : ptr{std::make_shared<_Model>(model)} {}
        ModelWrapper(const _Model &&model) : ptr{std::make_shared<_Model>(model)} {}
        ModelWrapper(const ModelWrapper &model_wrapper) : ptr{model_wrapper.ptr} {}
        template <typename... Args>
        ModelWrapper(Args... args) : ptr(std::make_shared<_Model>(args...)) {}

        operator _Model()
        {
            return *ptr;
        }

        Output get_inference(State &state)
        {
            auto raw_state = *state.deref<typename _Model::Types::State>();
            ptr->get_inference(raw_state, output);
            return Output{std::vector<double>{}, std::vector<double>{}, 0, 0};
        };
    };

    // template <typename... Args>
    // struct SearchWrapper;

    template <typename _Algorithm>
    struct SearchWrapper : Search
    {
        std::shared_ptr<_Algorithm> ptr;
        std::shared_ptr<MatrixNode<_Algorithm>> root;
        // SearchWrapper(const _Algorithm &state) :  ptr{std::make_shared<_Algorithm>(state)}, root{std::make_shared<MatrixNode<_Algorithm>>{}} {}
        SearchWrapper(const _Algorithm &state) : ptr{std::make_shared<_Algorithm>(state)}, root{std::make_shared<MatrixNode<_Algorithm>>()} {}
        // SearchWrapper(const _Algorithm &&state) : ptr{std::make_shared<_Algorithm>(state)}, root{std::make_shared<MatrixNode<_Algorithm>>{}} {}
        // SearchWrapper(const SearchWrapper &state_wrapper) : ptr{state_wrapper.ptr}, root{std::make_shared<MatrixNode<_Algorithm>>{}} {}
        template <typename... Args>
        SearchWrapper(Args... args) : ptr(std::make_shared<_Algorithm>(args...)), root{std::make_shared<MatrixNode<_Algorithm>>()} {}

        void run(size_t iterations, State &state, Model &model)
        {
            auto state_ptr = state.deref<typename _Algorithm::Types::State>();
            auto model_ptr = model.deref<typename _Algorithm::Types::Model>();
            typename _Algorithm::Types::PRNG device{};
            ptr->run(iterations, device, *state_ptr, *model_ptr, *root);
        };

        void reset()
        {
            *root = MatrixNode<_Algorithm>{};
        }

        void get_empirical_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy)
        {
            row_strategy.clear();
            col_strategy.clear();
            typename _Algorithm::Types::VectorReal r{root->row_actions.size()}, c{root->col_actions.size()};
            ptr->get_empirical_strategies(&*root, r, c);
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
            typename _Algorithm::Types::VectorReal r{root->row_actions.size()}, c{root->col_actions.size()};
            ptr->get_empirical_strategies(&*root, r, c);
            Matrix<PairDouble> matrix;
            state.get_payoff_matrix(matrix);
            double expl = static_cast<double>(math::exploitability<typename _Algorithm::Types>(matrix, r, c));
            return expl;
        }; // aka expected regret because we use empirical strategies
    };

    // SEARCH_PARAMS(State_, Model_, BanditAlgo_, TreeAlgo_)
    // using Search = SearchWrapper<BanditAlgo_<Model_<State_>, TreeAlgo_, MatrixNode, ChanceNode>>;

    // template <typename _Model, template <typename> typename __Algorithm>
    // struct SearchWrapper<__Algorithm<_Model>>
    // {
    //     using _Algorithm = __Algorithm<_Model>;
    //     std::shared_ptr<_Algorithm> ptr;
    //     SearchWrapper(const _Algorithm &state) : ptr{std::make_shared<_Algorithm>(state)} {}
    //     SearchWrapper(const _Algorithm &&state) : ptr{std::make_shared<_Algorithm>(state)} {}
    //     SearchWrapper(const SearchWrapper &state_wrapper) : ptr{std::make_shared<_Algorithm>(*state_wrapper.ptr)} {}
    //     template <typename... Args>
    //     SearchWrapper(Args... args) : ptr(std::make_shared<_Algorithm>(args...)) {}
    //     // SearchWrapper<Battle,

    //     void run(size_t iterations, State &state, Model &model){};
    //     void get_empirical_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy){};
    //     void get_refined_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy){};
    //     void get_emprical_value(double &row_value, double &col_value){};
    //     double get_exploitability() { return 0; }; // aka expected regret because we use empirical strategies
    // };

    template <typename _MatrixNode>
    struct MatrixNodeWrapper
    {
        std::shared_ptr<_MatrixNode> ptr;
        std::unordered_map<std::string, std::any> stats() = 0;
        void get_metadata() = 0;
    };
};

// State, Model, Search, update_model(state = inference...)?