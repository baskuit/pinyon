#include <libsurskit/dynamic-wrappers.hh>
#include <types/types.hh>

#include <utility>

struct Arena : SimpleTypes
{

    class State : public PerfectInfoState<SimpleTypes>
    {
    public:
        W::Types::State (*state_generator)(SimpleTypes::Seed){nullptr};
        std::vector<W::Types::Model> models{};

        SimpleTypes::Seed state_seed{};

        State(
            W::Types::State (*state_generator)(SimpleTypes::Seed),
            const std::vector<W::Types::Model> &models)
            : state_generator{state_generator}, models{models}
        {
            this->init_range_actions(models.size());
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
            if (row_action == col_action)
            {
                // skip if its a mirror match
                this->payoff = SimpleTypes::Value{.5, .5};
                this->obs = 0;
                this->terminal = true;
                return;
            }
            W::Types::PRNG device{state_seed};
            W::Types::Model row_model = models[static_cast<int>(row_action)];
            W::Types::Model col_model = models[static_cast<int>(col_action)];
            // copy constr will clone the unique_ptr member

            W::Types::State state = (*state_generator)(state_seed);
            W::Types::Value row_first_payoff = play_vs(device, row_model, col_model, state);
            W::Types::Value col_first_payoff = play_vs(device, col_model, row_model, state);

            // std::cout << row_first_payoff.get_row_value() << " " << row_first_payoff.get_col_value() <<
            // "   " <<  col_first_payoff.get_row_value() << " " << col_first_payoff.get_col_value() << std::endl;


            W::Types::Value col_first_payoff_flipped{col_first_payoff.get_col_value(), col_first_payoff.get_row_value()};

            W::Types::Value avg_payoff = (row_first_payoff + col_first_payoff_flipped) * 0.5;

            this->payoff = SimpleTypes::Value{avg_payoff.get_row_value(), avg_payoff.get_col_value()};
            this->terminal = true;
            this->obs = SimpleTypes::Obs{static_cast<int>(device.get_seed())};
        }

    private:
        W::Types::Value play_vs(
            W::Types::PRNG &device,
            W::Types::Model &row_model,
            W::Types::Model &col_model,
            const W::Types::State &state_) const
        {
            W::Types::State state = state_;
            W::Types::ModelOutput row_output, col_output;

            state.get_actions();
            while (!state.is_terminal())
            {
                row_model.get_inference(state, row_output);
                col_model.get_inference(state, col_output);
                const int row_idx = device.sample_pdf(row_output.row_policy);
                const int col_idx = device.sample_pdf(col_output.col_policy);

                // std::cout << "row" << std::endl;
                // math::print(row_output.row_policy);
                // math::print(col_output.row_policy);
                // std::cout << "col" << std::endl;
                // math::print(row_output.col_policy);
                // math::print(col_output.col_policy);
                // std::cout << std::endl;

                state.apply_actions(row_idx, col_idx);
                state.get_actions();
            }
            // std::cout << "___________" << std::endl;
            return state.get_payoff();
        }
    };
};
