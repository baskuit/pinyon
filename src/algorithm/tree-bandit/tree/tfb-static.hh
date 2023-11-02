#include "pinyon.hh"

template <
    typename Types,
    typename Options>
struct TreeBanditFlat : Types
{

    class Search : public Types::BanditAlgorithm
    {
    public:  
        using Types::BanditAlgorithm::BanditAlgorithm;

        Search(const Types::BanditAlgorithm &base) : Types::BanditAlgorithm{base} {}

        static std::array<typename Types::MatrixStats, Options::max_iterations> matrix_stats;

        // reset every run call
        static size_t iteration;

        static bool info[2 * Options::max_iterations];

        std::unordered_map<uint64_t, int> transition{};

        // reset every iteration
        static int depth;
        static int index;
        static typename Types::ModelOutput leaf_output;
        static int rows, cols;

        static constexpr std::hash<typename Types::Obs::type> hash_function{};

        static std::array<typename Types::Outcome, Options::max_depth> outcomes;

        static std::array<int, Options::max_depth> matrix_indices;

        size_t run_for_iterations(
            const size_t iterations,
            Types::PRNG &device,
            const Types::State &state,
            Types::Model &model)
        {
            transition = std::unordered_map<uint64_t, int>{};
            memset(info, false, 2 * Options::max_iterations * sizeof(bool));
            const auto start = std::chrono::high_resolution_clock::now();
            for (iteration = 0; iteration < iterations; ++iteration)
            {
                typename Types::State state_copy = state;
                state_copy.randomize_transition(device);
                run_iteration(device, state_copy, model);
            }
            const auto end = std::chrono::high_resolution_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            return duration.count();
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

            while (*info_ptr && !state.is_terminal() && depth < Options::max_depth)
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
                uint64_t hash_ = hash(index, outcome.row_idx, outcome.col_idx, hash_function(state.get_obs().get()));

                if (transition[hash_] == 0)
                {
                    index = iteration;
                    transition[hash_] = iteration;
                    info_ptr = &info[2 * index];
                    if constexpr (std::is_same_v<typename Options::return_after_expand, void>)
                    {
                        break;
                    }
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
                if constexpr (std::is_same_v<typename Options::update_using_average, void>)
                {
                    outcomes[d].value = leaf_output.value;
                }
                else
                {
                    // TODO fix!! [0]
                    this->get_empirical_value(matrix_stats[0], outcomes[d].value);
                }

                this->update_matrix_stats(matrix_stats[matrix_indices[d]], outcomes[d]);
            }
        }

        inline uint64_t hash(int index, int row_idx, int col_idx, uint64_t obs_hash) const
        {
            size_t h = 17;
            h = h * 31 + index;
            h = h * 31 + row_idx;
            h = h * 31 + col_idx;
            h = h * 31 + static_cast<uint64_t>(obs_hash);
            h = h * 31 + static_cast<uint64_t>(obs_hash >> 32);
            return h;
        }
    };
};

template <
    typename Types,
    typename Options>
std::array<typename Types::MatrixStats, Options::max_iterations> TreeBanditFlat<Types, Options>::Search::matrix_stats = {};

template <
    typename Types,
    typename Options>
size_t TreeBanditFlat<Types, Options>::Search::iteration = 0;

template <
    typename Types,
    typename Options>
int TreeBanditFlat<Types, Options>::Search::depth = {};

template <
    typename Types,
    typename Options>
int TreeBanditFlat<Types, Options>::Search::index = {};

template <
    typename Types,
    typename Options>
typename Types::ModelOutput TreeBanditFlat<Types, Options>::Search::leaf_output = {};

template <
    typename Types,
    typename Options>
std::array<typename Types::Outcome, Options::max_depth> TreeBanditFlat<Types, Options>::Search::outcomes = {};

template <
    typename Types,
    typename Options>
std::array<int, Options::max_depth> TreeBanditFlat<Types, Options>::Search::matrix_indices = {};

template <
    typename Types,
    typename Options>
int TreeBanditFlat<Types, Options>::Search::rows = 0;

template <
    typename Types,
    typename Options>
int TreeBanditFlat<Types, Options>::Search::cols = 0;

template <
    typename Types,
    typename Options>
 bool TreeBanditFlat<Types, Options>::Search::info[2 * Options::max_iterations]{};