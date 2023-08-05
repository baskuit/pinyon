#include <wrapper/basic.hh>

#include <utility>

template <IsValueModelTypes Types>
struct Arena : Types::TypeList
{

    class State : public PerfectInfoState<Types::TypesList>
    {
    public:
        size_t search_iterations;
        W::Types::State *(*init_state_generator)(Types::Seed){nullptr};
        W::Types::Model *model{device};

        typename Types::Seed state_seed{};

        prng device{};
        std::vector<W::Types::Search *> searches{};

        template <typename... Containers>
        Arena(
            const size_t search_iterations,
            W::Types::State (*init_state_generator)(typename Types::Seed),
            const W::Types::Model *model,
            Containers &...containers)
            : search_iterations{search_iterations}, init_state_generator{init_state_generator}, model{model}
        {
            (std::transform(containers.begin(), containers.end(), std::back_inserter(searches),
                            [](auto &search)
                            { return &search; }),
             ...);

            this->init_range_actions(searches.size());
        }

        void get_actions() const {}

        void get_actions(
            Types::VectorAction &row_actions,
            Types::VectorAction &col_actions) const
        {
            row_actions = this->row_actions;
            col_actions = this->col_actions;
        }

        void randomize_transition(Types::PRNG &device)
        {
            state_seed = device.uniform_64();
        }

        void apply_actions(
            Types::Action row_action,
            Types::Action col_action)
        {
            W::Types::Search *row_search = searches[static_cast<int>(row_action)]->clone();
            W::Types::Search *col_search = searches[static_cast<int>(col_action)]->clone();

            W::Types::State *state_copy = (*init_state_generator)(state_seed);
            W::Types::Value row_first_payoff = play_vs(row_search, col_search, state_copy, model);
            state_copy = (*init_state_generator)(state_seed);
            W::Types::Value col_first_payoff = play_vs(col_search, row_search, state_copy, model);
            W::Types::Value avg_payoff = (row_first_payoff + col_first_payoff) * 0.5;
            this->payoff = static_cast<W::Types::Value>(avg_payoff.get_row_value(), avg_payoff.get_col_value());

            this->terminal = true;
            this->obs = typename Types::Obs{device.random_int(1 << 16)};

            delete row_search;
            delete col_search;
        }

    private:
        W::Types::Value play_vs(
            W::Types::Search *row_search,
            W::Types::Search *col_search,
            W::Types::State *state,
            W::Types::Model *model)
        {
            state.get_actions();
            while (!state.is_terminal())
            {
                std::vector<double> row_strategy, col_strategy;
                row_search->run_and_get_strategies(row_strategy, col_strategy, search_iterations, state, model);
                ActionIndex row_idx = device.sample_pdf(row_strategy);
                col_search->run_and_get_strategies(row_strategy, col_strategy, search_iterations, state, model);
                ActionIndex col_idx = device.sample_pdf(col_strategy);
                state.apply_actions(row_idx, col_idx);
                state.get_actions();
            }
            return W::Types::Value{state.row_payoff(), state.col_payoff()};
        }
    };
};