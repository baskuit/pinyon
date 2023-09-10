#pragma once

#include <torch/torch.h>

class TwoLayerMLP : public torch::nn::Module
{
private:
    torch::nn::Linear fc1 = nullptr;
    torch::nn::Linear fc2 = nullptr;

public:
    TwoLayerMLP(const size_t n_input, const size_t n_hidden = 512)
        : fc1{register_module("fc1", torch::nn::Linear(n_input, n_hidden))},
          fc2{register_module("fc2", torch::nn::Linear(n_hidden, 1))}
    {
    }

    torch::Tensor forward(torch::Tensor x)
    {
        x = torch::relu(fc1->forward(x));
        x = fc2->forward(x);
        x = torch::sigmoid(x);
        return x;
    }
};

struct MLPTower : torch::nn::Module
{
    torch::nn::Sequential layers;

    MLPTower(int n_layers = 2, int input = 386, int hidden = 512)
    {
        for (int i = 0; i < n_layers - 1; ++i)
        {
            layers->push_back(torch::nn::Linear(i == 0 ? 100 : hidden, hidden));
            layers->push_back(torch::nn::ReLU());
        }
        layers->push_back(torch::nn::Linear(hidden, 1));

        register_module("layers", layers);
    }

    torch::Tensor forward(torch::Tensor x)
    {
        return layers->forward(x);
    }
};
