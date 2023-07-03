#include <wrapper/basic.hh>

#include <utility>

template <class _State, class _Model>
class Arena : public PerfectInfoState<SimpleTypes>
{
public:
    struct Types : PerfectInfoState<SimpleTypes>::Types
    {
    };

    size_t iterations;
    W::StateWrapper<_State> (*init_state_generator)(typename Types::Seed){nullptr};
    W::ModelWrapper<_Model> model{device};

    typename Types::Seed init_state_seed{};

    prng device{};
    std::vector<W::Search *> searches{};

    template <typename... Containers>
    Arena(
        const size_t iterations,
        W::StateWrapper<_State> (*init_state_generator)(typename Types::Seed),
        const _Model &model,
        Containers&... containers) : iterations{iterations}, init_state_generator{init_state_generator}, model{model}
    {
        (std::transform(containers.begin(), containers.end(), std::back_inserter(searches),
                        [](auto &search)
                        { return &search; }),
         ...);
        // append pointer to search wrapper to this->searches

        const size_t size = searches.size();
        this->row_actions.fill(size);
        this->col_actions.fill(size);
        for (int i = 0; i < size; ++i)
        {
            this->row_actions[i] = typename Types::Action{i};
            this->col_actions[i] = typename Types::Action{i};
        }
    }

    void get_actions() {}

    void reseed(typename Types::PRNG &device)
    {
        init_state_seed = device.uniform_64();
    }

    void apply_actions(
        typename Types::Action row_action,
        typename Types::Action col_action)
    {
        W::Search *row_search = searches[static_cast<int>(row_action)]->clone();
        W::Search *col_search = searches[static_cast<int>(col_action)]->clone();

        W::StateWrapper<_State> state_copy = (*init_state_generator)(init_state_seed);
        PairDouble row_first_payoff = play_vs(row_search, col_search, state_copy, model);
        state_copy = (*init_state_generator)(init_state_seed);
        PairDouble col_first_payoff = play_vs(col_search, row_search, state_copy, model);

        PairDouble avg_payoff = (row_first_payoff + col_first_payoff) * 0.5;
        this->payoff = typename Types::Value{avg_payoff.get_row_value()};
        this->is_terminal = true;
        this->obs = typename Types::Observation{device.random_int(1 << 16)};

        delete row_search;
        delete col_search;
    }

    PairDouble play_vs(
        W::Search *row_search,
        W::Search *col_search,
        W::State &state,
        W::Model &model)
    {
        state.get_actions();
        while (!state.is_terminal())
        {
            std::vector<double> row_strategy, col_strategy;
            // row_search->reset();
            // col_search->reset();
            row_search->run_and_get_strategies(row_strategy, col_strategy, iterations, state, model);
            ActionIndex row_idx = device.sample_pdf(row_strategy);
            col_search->run_and_get_strategies(row_strategy, col_strategy, iterations, state, model);
            ActionIndex col_idx = device.sample_pdf(col_strategy);
            state.apply_actions(row_idx, col_idx);
            state.get_actions();
        }
        return PairDouble{state.row_payoff(), state.col_payoff()};
    }
};