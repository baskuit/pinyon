#pragma once

#include <types/matrix.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

#include <memory>

namespace W
{

    /*

    Forward declaration

    */

    struct State;
    struct Model;
    struct Search;

    template <IsStateTypes Types>
    struct StateWrapper;
    template <IsValueModelTypes Types>
    struct ModelWrapper;
    template <IsSearchTypes Types>
    struct SearchWrapper;

    /*
    
    TypesList
    
    */

    struct Types
    {
        using Rational = Rational<>;
        using Real = double;
        using Probability = double;
        using Observation = bool;
        using Value = PairReal<double>;
        using Action = int;
        using VectorAction = std::vector<int>;
        using VectorReal = std::vector<double>;
        using VectorInt = std::vector<int>;
        using Mutex = std::mutex;
        using PRNG = prng;

        using ModelInput = W::State;
        struct ModelOutput
        {
            std::vector<double> row_policy, col_policy;
            double row_value, col_value;
        };
        using ModelBatchInput = std::vector<ModelInput>;
        using ModelBatchOutput = std::vector<ModelOutput>;

        using State = W::State;
        using Model = W::Model;
        using Search = W::Search;
    };

    /*
    
    State
    
    */

    struct State
    {
        virtual void get_actions(Types::VectorAction &, Types::VectorAction &) = 0;
        virtual void get_actions(size_t &rows, size_t &cols) = 0;
        virtual void apply_actions(Types::Action row_idx, Types::Action col_idx) = 0;
        virtual bool is_terminal() const = 0;
        virtual Types::Value get_payoff() = 0;

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

        StateWrapper(typename Types::State const &state) : ptr{std::make_shared<typename Types::State>(state)}
        {
            state.
        }
        template <typename... Args>
        StateWrapper(Args... args) : ptr(std::make_shared<typename Types::State>(args...)) {}

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
        PairReal<double> get_payoff() const
        {
            // TODO
            return PairReal<double>{0, 0};
        }

        operator typename Types::State()
        {
            return *ptr;
        }
        StateWrapper clone()
        {
            return StateWrapper{*ptr};
        }
    };

    /*

    Model

    */

    struct Model
    {
        virtual void get_inference(Types::ModelInput &, Types::ModelOutput &) = 0;
        void get_inference(
            Types::ModelBatchInput &batch_input,
            Types::ModelBatchOutput &batch_output)
        {
            // assert(batch_input.size(), batch_output.size());
            for (int i = 0; i < batch_input.size(); ++i) {
                get_inference(batch_input[i], batch_output[i]);
            }
        }
    };

    template <IsValueModelTypes Types>
    struct ModelWrapper : Model
    {
        std::shared_ptr<typename Types::Model> ptr;
        ModelWrapper(const typename Types::Model &model) : ptr{std::make_shared<typename Types::Model>(model)} {}
        ModelWrapper(const typename Types::Model &&model) : ptr{std::make_shared<typename Types::Model>(model)} {}
        ModelWrapper(const ModelWrapper &model_wrapper) : ptr{model_wrapper.ptr} {}
        template <typename... Args>
        ModelWrapper(Args... args) : ptr(std::make_shared<typename Types::Model>(args...)) {}
        void get_inference(
            Types::ModelInput &input, 
            Types::ModelOutput &output)
        {
            auto raw_state = *input.derive_ptr<Types>();
            ptr->get_inference(raw_state, output);
        };

        operator typename Types::Model()
        {
            return *ptr;
        }
    };

    /*

    Search

    */

    struct Search
    {
        virtual void run(size_t iterations, State &state, Model &model) = 0;
        std::shared_ptr<typename Types::Search> derive_ptr()
        {
            SearchWrapper<Types> *self = dynamic_cast<SearchWrapper<Types> *>(this);
            return self->ptr;
        }

        virtual Search *clone() = 0;
    };

    template <IsSearchTypes Types>
    struct SearchWrapper : Search
    {
        std::shared_ptr<typename Types::Search> ptr;
        // std::shared_ptr<typename Types::MatrixNode> root;

        SearchWrapper(const typename Types::Search &session) : ptr{std::make_shared<typename Types::Search>(session)}, root{std::make_shared<typename Types::MatrixNode>()} {}

        template <typename... Args>
        SearchWrapper(Args... args) : ptr(std::make_shared<typename Types::Search>(args...)), root{std::make_shared<typename Types::MatrixNode>()} {}

        void run(size_t iterations, State &state, Model &model)
        {
            auto state_ptr = state.derive_ptr<Types>();
            auto model_ptr = model.derive_ptr<Types>();
            typename Types::PRNG device{};
            // ptr->run(iterations, device, *state_ptr, *model_ptr, *root);
        }

        Search *clone()
        {
            return new SearchWrapper{*ptr};
        }
    };
};
