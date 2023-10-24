#include "pinyon.hh"

template <
    typename Types,
    size_t max_iterations = 1 << 10,
    size_t max_depth = 10>
struct TreeBanditFlat : Types
{

    class Search : public Types::BanditAlgorithm
    {
    public:
        using Types::BanditAlgorithm::BanditAlgorithm;
        Search() {}

        std::array<typename Types::MatrixStats, max_iterations> matrix_stats{};

        // reset every run call
        size_t iteration = 0;

        bool info[2 * max_iterations];

        std::unordered_map<uint64_t, int> transition{};

        // reset every iteration
        int depth = 0;
        int index = 0;
        typename Types::ModelOutput leaf_output;
        int rows, cols, row_idx, col_idx;

        std::array<typename Types::Outcome, max_depth> outcomes{}; // indices, mu

        std::array<int, max_depth> matrix_indices{}; // 0, 1, 4,

        void run_for_iterations(
            const size_t iterations,
            Types::PRNG &device,
            const Types::State &state,
            Types::Model &model)
        {
            transition = std::unordered_map<uint64_t, int>{};
            memset(info, false, 2 * iteration * sizeof(bool));
            for (iteration = 0; iteration < iterations; ++iteration)
            {
                typename Types::State state_copy = state;
                state_copy.randomize_transition(device);
                run_iteration(device, state_copy, model);
            }
        }

        void run_iteration(
            Types::PRNG &device,
            Types::State &state,
            Types::Model &model)
        {
            depth = 0;
            index = 0;

            // side by side: was_seen and is_expanded (state, e.g. expanded vectors)
            bool *info_ptr = &info[2 * index];

            while (*info_ptr && !state.is_terminal() && depth < max_depth)
            {
                typename Types::Outcome &outcome = outcomes[depth];
                typename Types::MatrixStats &stats = matrix_stats[index];

                // not really expanded
                if (!*(info_ptr + 1))
                {
                    rows = state.row_actions.size();
                    cols = state.col_actions.size();
                    this->expand_state_part(stats, rows, cols);
                    *(info_ptr + 1) = true;
                }

                this->select(device, stats, outcome);

                state.apply_actions(
                    state.row_actions[outcome.row_idx],
                    state.col_actions[outcome.col_idx]);
                state.get_actions();

                ++depth;
                uint64_t hash_ = hash(index, row_idx, col_idx, std::hash(state.get_obs()));

                if (transition[hash_] == 0)
                {
                    index = iteration;
                    transition[hash_] = iteration;
                    info_ptr = &info[2 * index];
                    break;
                }
                else
                {
                    index = transition[hash_];
                    info_ptr = &info[2 * index];
                }
                matrix_indices[depth] = index;
            }

            if (state.is_terminal())
            {
                leaf_output.value = state.payoff;
            }
            else
            {
                *info_ptr = true;
                model.inference(std::move(state), leaf_output);
            }
            this->expand_inference_part(matrix_stats[index], leaf_output);

            for (int d = 0; d < depth; ++d)
            {
                outcomes[d].value = leaf_output.value;
                this->update_matrix_stats(matrix_stats[matrix_indices[d]], outcomes[d]);
            }
        }

        uint64_t hash(int index, int row_idx, int col_idx, uint64_t obs_hash) const
        {
            size_t h = 17;
            h = h * 31 + index;
            h = h * 31 + row_idx;
            h = h * 31 + col_idx;
            h = h * 31 + static_cast<uint64_t>(obs_hash);
            h = h * 31 + tatic_cast<uint64_t>(obs_hash >> 32);
            return h;
        }
    };
};
