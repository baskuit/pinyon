#pragma once

#include <torch/torch.h>
#include "utils.hh"

class TwoLayerMLP : public torch::nn::Module
{
public:
    TwoLayerMLP()
    {
        fc1_ = register_module("fc1", torch::nn::Linear(size_of_battle, 512));
        fc2_ = register_module("fc2", torch::nn::Linear(512, 1));
    }

    torch::Tensor forward(torch::Tensor x)
    {
        x = torch::relu(fc1_->forward(x));
        x = fc2_->forward(x);
        return x;
    }

private:
    torch::nn::Linear fc1_ = nullptr;
    torch::nn::Linear fc2_ = nullptr;
};

struct MLPTower : torch::nn::Module {
    torch::nn::Sequential layers;

    MLPTower(int numLayers=2, int hiddenSize=512) {
        for (int i = 0; i <  numLayers - 1; ++i) {
            layers->push_back(torch::nn::Linear(i == 0 ? size_of_battle : hiddenSize, hiddenSize));
            layers->push_back(torch::nn::ReLU());
        }
        layers->push_back(torch::nn::Linear(hiddenSize, 1));

        register_module("layers", layers);
    }

    torch::Tensor forward(torch::Tensor x) {
        return layers->forward(x);
    }
};