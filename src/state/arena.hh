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
            // (std::transform(containers.begin(), containers.end(), std::back_inserter(searches),
            //                 [](auto &search)
            //                 { return &search; }),
            //  ...);
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
            W::Types::Search row_search = searches[static_cast<int>(row_action)];
            W::Types::Search col_search = searches[static_cast<int>(col_action)];

            // W::Types::Search *row_search = searches[static_cast<int>(row_action)]->clone();
            // W::Types::Search *col_search = searches[static_cast<int>(col_action)]->clone();

            // W::Types::State *state_copy = (*init_state_generator)(state_seed);
            // W::Types::Value row_first_payoff = play_vs(row_search, col_search, state_copy, model);
            // state_copy = (*init_state_generator)(state_seed);
            // W::Types::Value col_first_payoff = play_vs(col_search, row_search, state_copy, model);
            // W::Types::Value avg_payoff = (row_first_payoff + col_first_payoff) * 0.5;
            // this->payoff = static_cast<W::Types::Value>(avg_payoff.get_row_value(), avg_payoff.get_col_value());

            // this->terminal = true;
            // this->obs = typename Types::Obs{device.random_int(1 << 16)};

            // delete row_search;
            // delete col_search;
        }

    private:
        W::Types::Value play_vs(
            W::Types::Search *row_search,
            W::Types::Search *col_search,
            W::Types::State *state,
            W::Types::Model *model)
        {
            return {};
            // state.get_actions();
            // while (!state.is_terminal())
            // {
            //     std::vector<double> row_strategy, col_strategy;
            //     row_search->run_and_get_strategies(row_strategy, col_strategy, search_iterations, state, model);
            //     ActionIndex row_idx = device.sample_pdf(row_strategy);
            //     col_search->run_and_get_strategies(row_strategy, col_strategy, search_iterations, state, model);
            //     ActionIndex col_idx = device.sample_pdf(col_strategy);
            //     state.apply_actions(row_idx, col_idx);
            //     state.get_actions();
            // }
            // return W::Types::Value{state.row_payoff(), state.col_payoff()};
        }
    };
};