#include <wrapper/basic.hh>

#include <utility>

template <IsValueModelTypes Types>
class Arena : public PerfectInfoState<Types>
{
public:
    struct T : Types {
        using State = Arena;
    };

    size_t search_iterations;
    W::StateWrapper<typename Types::State::T> (*init_state_generator)(Types::Seed){nullptr};
    W::ModelWrapper<typename Types::Model::T> model{device};

    typename Types::Seed state_seed{};

    prng device{};
    std::vector<W::Search *> searches{};

    template <typename... Containers>
    Arena(
        const size_t search_iterations,
        W::StateWrapper<typename Types::State::T> (*init_state_generator)(typename Types::Seed),
        const typename Types::Model &model,
        Containers &...containers) : search_iterations{search_iterations}, init_state_generator{init_state_generator}, model{model}
    {
        (std::transform(containers.begin(), containers.end(), std::back_inserter(searches),
                        [](auto &search)
                        { return &search; }),
         ...);
        // append pointer to search wrapper to this->searches

        const size_t size = searches.size();
        this->row_actions.resize(size);
        this->col_actions.resize(size);
        for (int i = 0; i < size; ++i)
        {
            this->row_actions[i] = typename Types::Action{i};
            this->col_actions[i] = typename Types::Action{i};
        }
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
        W::Search *row_search = searches[static_cast<int>(row_action)]->clone();
        W::Search *col_search = searches[static_cast<int>(col_action)]->clone();

        W::StateWrapper<Types> state_copy = (*init_state_generator)(state_seed);
        PairReal<double> row_first_payoff = play_vs(row_search, col_search, state_copy, model);
        state_copy = (*init_state_generator)(state_seed);
        PairReal<double> col_first_payoff = play_vs(col_search, row_search, state_copy, model);

        PairReal<double> avg_payoff = (row_first_payoff + col_first_payoff) * 0.5;
        this->payoff = static_cast<Types::Value>(avg_payoff.get_row_value(), avg_payoff.get_col_value());

        this->terminal = true;
        this->obs = typename Types::Obs{device.random_int(1 << 16)};

        delete row_search;
        delete col_search;
    }

private:
    PairReal<double> play_vs(
        W::Search *row_search,
        W::Search *col_search,
        W::State &state,
        W::Model &model)
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
        return PairReal<double>{state.row_payoff(), state.col_payoff()};
    }
};