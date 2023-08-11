#include <libsurskit/dynamic-wrappers.hh>
#include <types/types.hh>

#include <utility>

struct Arena : SimpleTypes
{

    class State : public PerfectInfoState<SimpleTypes>
    {
    public:
        size_t search_iterations;
        W::Types::State (*state_generator)(SimpleTypes::Seed){nullptr};
        SimpleTypes::Seed state_seed{};

        W::Types::Model model;
        std::vector<W::Types::Search> searches{};

        State(
            const size_t search_iterations,
            W::Types::State (*init_state_generator)(SimpleTypes::Seed),
            W::Types::Model &model,
            std::vector<W::Types::Search> &searches)
            : search_iterations{search_iterations}, state_generator{init_state_generator}, model{model}, searches{searches}
        {
            this->init_range_actions(searches.size());
        }



        void get_actions() const {}

        void get_actions(
            SimpleTypes::VectorAction &row_actions,
            SimpleTypes::VectorAction &col_actions) const
        {
            row_actions = this->row_actions;
            col_actions = this->col_actions;
        }

        void randomize_transition(SimpleTypes::PRNG &device)
        {
            state_seed = device.uniform_64();
        }

        void apply_actions(
            SimpleTypes::Action row_action,
            SimpleTypes::Action col_action)
        {
            W::Types::PRNG device;
            W::Types::Search row_search = searches[static_cast<int>(row_action)];
            W::Types::Search col_search = searches[static_cast<int>(col_action)];
            // copy constr will clone the unique_ptr member

            W::Types::State state = (*state_generator)(state_seed);
            W::Types::Value row_first_payoff = play_vs(device, row_search, col_search, state, model);
            W::Types::Value col_first_payoff = play_vs(device, col_search, row_search, state, model);

            W::Types::Value avg_payoff = (row_first_payoff + col_first_payoff) * 0.5;

            this->payoff = SimpleTypes::Value{avg_payoff.get_row_value(), avg_payoff.get_col_value()};
            this->terminal = true;
            this->obs = SimpleTypes::Obs{device.get_seed()};

        }

    private:
        W::Types::Value play_vs(
            W::Types::PRNG &device,
            W::Types::Search &row_search,
            W::Types::Search &col_search,
            const W::Types::State &state_,
            W::Types::Model &model)
        {
            W::Types::State state = state_;
            state.get_actions();
            while (!state.is_terminal())
            {
                W::Types::VectorReal row_strategy, col_strategy;
                W::Types::MatrixNode matrix_node{row_search.get_matrix_node()};
                row_search.run_for_iterations(search_iterations, device, state, model, matrix_node);
                row_search.get_strategies(matrix_node, row_strategy, col_strategy);
                int row_idx = device.sample_pdf(row_strategy);
                int col_idx = device.sample_pdf(col_strategy);
                state.apply_actions(row_idx, col_idx);
                state.get_actions();
            }
            return state.get_payoff();
        }
    };
};