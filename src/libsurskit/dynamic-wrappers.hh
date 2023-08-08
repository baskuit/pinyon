#pragma once

#include <types/matrix.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

#include <memory>

namespace W
{
    struct State;
    struct Model;
    struct Search;
    struct MatrixNode;

    struct Types : DefaultTypes<double, int, bool, double>
    // Actions=int, using the fact that the order of get_actions() is assumed to be fixed
    {
        using ModelInput = W::State;
        struct ModelOutput
        {
            PairReal<double> value;
            std::vector<double> row_policy, col_policy;
        };
        using State = W::State;
        using Model = W::Model;
        using Search = W::Search;
        using MatrixNode = W::MatrixNode;
    };

    /*



    EEE



    */

    namespace Detail
    {
        struct State
        {
            virtual std::unique_ptr<Detail::State> clone() const = 0;
            virtual void get_actions(size_t &rows, size_t &cols) = 0;
            virtual void apply_actions(Types::Action row_idx, Types::Action col_idx) = 0;
            virtual bool is_terminal() const = 0;
            virtual Types::Value get_payoff() const = 0;
        };
        template <IsPerfectInfoStateTypes T>
        struct StateWrapper : State
        {
            typename T::State state;

            template <typename... Args>
            StateWrapper(Args... args) : state{args...} {}

            std::unique_ptr<Detail::State> clone() const
            {
                return std::make_unique<StateWrapper>(*this);
            }

            void get_actions(size_t &rows, size_t &cols)
            {
                state.get_actions();
                rows = state.row_actions.size();
                cols = state.col_actions.size();
            };
            void apply_actions(typename Types::Action row_idx, typename Types::Action col_idx)
            {
                state.apply_actions(
                    state.row_actions[static_cast<int>(row_idx)],
                    state.col_actions[static_cast<int>(col_idx)]);
            };
            bool is_terminal() const
            {
                return state.is_terminal();
            };
            Types::Value get_payoff() const
            {
                return Types::Value{
                    static_cast<double>(state.payoff.get_row_value()),
                    static_cast<double>(state.payoff.get_col_value())};
            }
        };

        struct Model
        {
            virtual std::unique_ptr<Detail::Model> clone() const = 0;
            virtual void get_inference(State *, Types::ModelOutput &) = 0;
        };
        template <IsValueModelTypes T>
        struct ModelWrapper : Model
        {
            typename T::Model model;

            template <typename... Args>
            ModelWrapper(Args... args) : model{args...} {}

            std::unique_ptr<Detail::Model> clone() const
            {
                return std::make_unique<ModelWrapper>(*this);
            }

            void get_inference(
                State *state_ptr,
                Types::ModelOutput &output)
            {
                typename T::State &state = dynamic_cast<StateWrapper<T> *>(state_ptr)->state;
                typename T::ModelOutput output_;
                model.get_inference(state, output_);
                output.value = Types::Value{output_.value.get_row_value(), output_.value.get_col_value()};
                // if constexpr (T::ModelOutput::row_policy)
                // {
                //     output.row_policy.resize(output_);
                // }
            }

            void get_state(
                State *state_ptr,
                Types::ModelInput &input)
            {
                typename T::State state = *dynamic_cast<typename T::State *>(state_ptr);
                // input.ptr = std::make_unique<typename T::State>(state);
            }
        };

        struct MatrixNode
        {
            virtual std::unique_ptr<Detail::MatrixNode> create_new() const = 0;
        };
        template <IsNodeTypes T>
        struct MatrixNodeWrapper : MatrixNode
        {
            typename T::MatrixNode matrix_node{};

            std::unique_ptr<Detail::MatrixNode> create_new() const
            {
                return std::make_unique<MatrixNodeWrapper>();
            }
        };

        struct Search
        {
            virtual std::unique_ptr<Detail::Search> clone() const = 0;
            virtual std::unique_ptr<Detail::MatrixNode> get_matrix_node () const = 0;
            virtual size_t run(
                size_t duration_ms,
                Types::PRNG &device,
                Detail::State *state_ptr,
                Detail::Model *model_ptr,
                Detail::MatrixNode *matrix_node_ptr) const = 0;
            virtual size_t run_for_iterations(
                size_t duration_ms,
                Types::PRNG &device,
                Detail::State *state_ptr,
                Detail::Model *model_ptr,
                Detail::MatrixNode *matrix_node_ptr) const = 0;
        };
        template <IsSearchTypes T>
        struct SearchWrapper : Search
        {
            typename T::Search session;

            template <typename... Args>
            SearchWrapper(Args... args) : session{args...} {}

            std::unique_ptr<Detail::Search> clone() const
            {
                return std::make_unique<SearchWrapper<T>>(*this);
            }

            std::unique_ptr<Detail::MatrixNode> get_matrix_node () const {
                return std::make_unique<MatrixNodeWrapper<T>>();
            }

            size_t run(
                size_t duration_ms,
                Types::PRNG &device,
                Detail::State *state_ptr,
                Detail::Model *model_ptr,
                Detail::MatrixNode *matrix_node_ptr) const
            {
                typename T::PRNG device_{device.uniform_64()};
                typename T::State &state = dynamic_cast<StateWrapper<T> *>(state_ptr)->state;
                typename T::Model &model = dynamic_cast<ModelWrapper<T> *>(model_ptr)->model;
                typename T::MatrixNode &matrix_node = dynamic_cast<MatrixNodeWrapper<T> *>(matrix_node_ptr)->matrix_node;
                size_t iterations = session.run(duration_ms, device_, state, model, matrix_node);
                return iterations;
            }
            size_t run_for_iterations(
                size_t iterations, Types::PRNG &device, Detail::State *state_ptr, Detail::Model *model_ptr, Detail::MatrixNode *matrix_node_ptr) const
            {
                typename T::PRNG device_{device.uniform_64()};
                typename T::State &state = dynamic_cast<StateWrapper<T> *>(state_ptr)->state;
                typename T::Model &model = dynamic_cast<ModelWrapper<T> *>(model_ptr)->model;
                typename T::MatrixNode &matrix_node = dynamic_cast<MatrixNodeWrapper<T> *>(matrix_node_ptr)->matrix_node;
                size_t ms = session.run_for_iterations(iterations, device_, state, model, matrix_node);
                return ms;
            }
        };

    }

    /*



    Types::State etc



    */

    struct State : PerfectInfoState<Types>
    {
        std::unique_ptr<Detail::State> ptr;

        State(const State &other)
        {
            ptr = other.ptr->clone();
        }

        State &operator=(const State &other)
        {
            ptr = other.ptr->clone();
            return *this;
        }

        template <IsStateTypes T, typename... Args>
        State(T, const Args &...args) : ptr(std::make_unique<Detail::StateWrapper<T>>(args...))
        {
            typename T::State &sss = (dynamic_cast<Detail::StateWrapper<T> *>(&*ptr))->state;
            std::cout << "constr: max depth: " << sss.max_depth << std::endl;
        }

        bool is_terminal() const
        {
            return ptr->is_terminal();
        }

        Types::Value get_payoff() const
        {
            return ptr->get_payoff();
        }

        void get_actions(
            Types::VectorAction &row_actions,
            Types::VectorAction &col_actions) const
        {
            size_t rows, cols;
            ptr->get_actions(rows, cols);
            row_actions.resize(rows);
            col_actions.resize(cols);
            for (int i = 0; i < rows; ++i)
            {
                row_actions[i] = typename Types::Action{i};
            }
            for (int i = 0; i < cols; ++i)
            {
                col_actions[i] = typename Types::Action{i};
            }
        };

        void randomize_transitions(
            Types::PRNG &)
        {
        }

        void get_actions()
        {
            size_t rows, cols;
            ptr->get_actions(rows, cols);
            this->row_actions.resize(rows);
            this->col_actions.resize(cols);
            for (int i = 0; i < rows; ++i)
            {
                this->row_actions[i] = typename Types::Action{i};
            }
            for (int i = 0; i < cols; ++i)
            {
                this->col_actions[i] = typename Types::Action{i};
            }
        }

        void apply_actions(
            Types::Action row_action,
            Types::Action col_action)
        {
            ptr->apply_actions(row_action, col_action);
        }
    };

    struct Model
    {
        std::unique_ptr<Detail::Model> ptr;

        template <IsValueModelTypes T, typename... Args>
        Model(T, const Args &...args) : ptr(std::make_unique<Detail::ModelWrapper<T>>(args...)) {}

        Model(const Model &other)
        {
            ptr = other.ptr->clone();
        }

        Model &operator=(const Model &other)
        {
            ptr = other.ptr->clone();
            return *this;
        }

        void get_inference(
            Types::ModelInput &input,
            Types::ModelOutput &output)
        {
            Detail::State *state_ptr = &*input.ptr;
            ptr->get_inference(state_ptr, output);
        }

        void get_input(
            Types::State &input,
            Types::ModelInput &state)
        {
            input = state;
        }
    };

    struct MatrixNode
    {
        std::unique_ptr<Detail::MatrixNode> ptr;

        MatrixNode (std::unique_ptr<Detail::MatrixNode> ptr) : ptr{std::move(ptr)} {}

        template <IsNodeTypes T>
        MatrixNode(T) : ptr{std::make_unique<Detail::MatrixNodeWrapper<T>>()} {}
    };

    struct Search
    {
        std::unique_ptr<Detail::Search> ptr;

        template <IsSearchTypes T, typename... Args>
        Search(T, const Args &...args) : ptr(std::make_unique<Detail::SearchWrapper<T>>(args...)) {}

        Search(const Search &other)
        {
            ptr = other.ptr->clone();
        }

        Search &operator=(const Search &other)
        {
            ptr = other.ptr->clone();
            return *this;
        }

        MatrixNode get_matrix_node () const {
            return MatrixNode{ptr->get_matrix_node()};
        }

        size_t run(
            size_t duration_ms,
            Types::PRNG &device,
            Types::State &state,
            Types::Model &model,
            Types::MatrixNode &matrix_node) const
        {
            Detail::State *state_ptr = &*(state.ptr);
            Detail::Model *model_ptr = &*model.ptr;
            Detail::MatrixNode *matrix_node_ptr = &*matrix_node.ptr;
            return ptr->run(duration_ms, device, state_ptr, model_ptr, matrix_node_ptr);
        }

        size_t run_for_iterations(
            size_t iterations,
            Types::PRNG &device,
            Types::State &state,
            Types::Model &model,
            Types::MatrixNode &matrix_node) const
        {
            Detail::State *state_ptr = &*(state.ptr);
            Detail::Model *model_ptr = &*model.ptr;
            Detail::MatrixNode *matrix_node_ptr = &*matrix_node.ptr;
            return ptr->run_for_iterations(iterations, device, state_ptr, model_ptr, matrix_node_ptr);
        }
    };

    template <typename T>
    Types::State get_w_state(const typename T::State &state)
    {
        return Types::State{T{}, state};
    }

    template <typename T>
    Types::Model get_w_model(const typename T::Model &model)
    {
        return Types::Model{T{}, model};
    }

    template <typename T>
    Types::Search get_w_search(const typename T::Search &search)
    {
        return Types::Search{T{}, search};
    }

}
