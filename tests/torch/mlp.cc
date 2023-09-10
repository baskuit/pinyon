#include <pinyon.hh>

struct MoldStateTensorTypes : MoldState<> {

    class Model : public TwoLayerMLP {
        public:
        // (100)

        Model () : TwoLayerMLP{386} {
        }

        void add_to_batch_input(
            MoldState<>::State &&state,
            torch::Tensor &model_batch_input
        ) const {
            model_batch_input = torch::cat({model_batch_input, torch::rand({1, 386})});
        }
    };
};

int main () {

    using Types = MoldStateTensorTypes;
    using NewTypes = LibtorchBatchModel<Types>;

    const size_t batch_size = 1 << 8;
    const size_t minibatches = 1 << 2;

    NewTypes::Model model{Types::Model{}, batch_size, minibatches};

    NewTypes::State state{2, 10};
    torch::Tensor input = torch::empty({0, 386});
    model.add_to_batch_input(std::move(state), input);
    torch::Tensor output;

    model.inference(input, output);

    return 0;

}