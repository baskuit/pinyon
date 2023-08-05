#pragma once

#include <types/matrix.hh>
#include <algorithm/algorithm.hh>
#include <tree/tree.hh>

#include <memory>

/*

W is not a type list. W::Types is the type list

*/

namespace W
{
    struct State;
    struct Model;
    struct Search;

    struct Types : DefaultTypes<double, int, bool, double>
    {
        using ModelInput = State;
        struct ModelOutput
        {
            PairReal<double> value;
            std::vector<double> row_policy, col_policy;
        };
        using State = W::State;
        using Model = W::Model;
        using Search = W::Search;
        using MatrixNode = int;
    };

    namespace Detail
    {

        struct State
        {
            virtual void get_actions(Types::VectorAction &, Types::VectorAction &) = 0;
            virtual void get_actions(size_t &rows, size_t &cols) = 0;
            virtual void apply_actions(Types::Action row_idx, Types::Action col_idx) = 0;
            virtual bool is_terminal() const = 0;
            virtual Types::Value get_payoff() = 0;
        };
        template <IsPerfectInfoStateTypes T>
        struct StateWrapper : State
        {
            typename T::State state;

            StateWrapper(typename T::State const &state) : state{state} {}
            template <typename... Args>
            StateWrapper(Args... args) : state{args...} {}

            void get_actions()
            {
                state.get_actions();
            }
            void get_actions(size_t &rows, size_t &cols)
            {
                state.get_actions();
                rows = state.row_actions.size();
                cols = state.col_actions.size();
            };
            void apply_actions(typename Types::Action row_idx, typename Types::Action col_idx)
            {
                state.apply_actions(state.row_actions[row_idx], state.col_actions[col_idx]);
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
            virtual void get_inference(Types::ModelInput &, Types::ModelOutput &) = 0;
        };
        template <IsValueModelTypes T>
        struct ModelWrapper : Model
        {
            typename T::Model model;

            ModelWrapper(typename T::Model const &model) : model{model} {}
            template <typename... Args>
            ModelWrapper(Args... args) : model{args...} {}

            void get_inference(
                Types::ModelInput &,
                Types::ModelOutput &)
            {
            }
        };

        struct Search
        {
            virtual void run() = 0;
        };
        template <IsSearchTypes T>
        struct SearchWrapper
        {
            typename T::Search session;

            SearchWrapper(typename T::Search const &session) : session{session} {}
            template <typename... Args>
            SearchWrapper(Args... args) : session{args...} {}

            void run() {}
        };

    };

    struct State
    {
        std::unique_ptr<Detail::State> ptr;
        template <IsTypeList Types, typename... Args>
        State(const Args &...args) : ptr(std::make_shared<Detail::StateWrapper<Types>>(args...)) {}
    };

    struct Model
    {
        std::unique_ptr<Detail::Model> ptr;
        template <IsTypeList Types, typename... Args>
        Model(const Args &...args) : ptr(std::make_shared<Detail::ModelWrapper<Types>>(args...)) {}
    };

    struct Search
    {
        std::unique_ptr<Detail::Search> ptr;
        template <IsTypeList Types, typename... Args>
        Search(const Args &...args) : ptr(std::make_shared<Detail::SearchWrapper<Types>>(args...)) {}
    };
};
