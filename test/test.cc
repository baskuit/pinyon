#include <assert.h>
#include <iostream>

class Model {
public:
    struct InferenceData {};
    InferenceData inference ();
};

class MonteCarlo : public Model {
public:
    struct InferenceData {
        double value_estimate0 = .7;
    };

    InferenceData inference () {
        InferenceData x;
        return x;
    }

};

template <typename Model>
struct A {
    A () {
        Model model;
        typename Model::InferenceData inference = model.inference();
        double x = inference.value_estimate0;
    }

};

// Passing a prng device to a function by value does not 'work'
// in the sense that calls inside the function will not progress the device outside the functions scope
// The reason for this is obvious

int main () {
    MonteCarlo model;
    MonteCarlo::InferenceData inference = model.inference();
    std::cout << inference.value_estimate0 << std::endl;
    return 0;
}