// #include <surskit.hh>
#include <tree/tree.hh>

#include <iostream>
#include <vector>

struct SmallTypes {
    using VectorAction = std::vector<uint8_t>;
    using VectorInt = std::vector<uint8_t>;
    using VectorReal = std::vector<float>;
    struct ModelOutput {
        float x, y; std::vector<float> a, b;
    };
    using Observation = struct A {};
    using Probability = float;
};

struct CurrentTypes {
    using VectorAction = std::vector<int>;
    using VectorInt = std::vector<int>;
    using VectorReal = std::vector<double>;

    struct ModelOutput {
        double x, y; std::vector<double> a, b;
    };
    using Observation = struct A {};
    using Probability = double;
};

template <class _Types>
class EmptyAlgorithm {
public:
    struct MatrixStats {};
    struct ChanceStats {};

    struct Types : _Types {
        using MatrixStats = EmptyAlgorithm::MatrixStats;
        using ChanceStats = EmptyAlgorithm::ChanceStats;
    };
};

template <typename Algorithm>
class MatrixNodeSmaller : public AbstractNode<Algorithm>
{
public:
    struct Types : AbstractNode<Algorithm>::Types
    {
    };

    ChanceNode<Algorithm> *parent = nullptr;
    ChanceNode<Algorithm> *child = nullptr;
    MatrixNode<Algorithm> *prev = nullptr;
    MatrixNode<Algorithm> *next = nullptr;

    bool is_terminal = false;
    bool is_expanded = false;

    typename Types::VectorAction row_actions;
    typename Types::VectorAction col_actions;
    typename Types::Observation obs;
    typename Types::Probability prob;
    typename Types::ModelOutput inference;
    typename Types::MatrixStats stats;
}


int main () {

    std::cout << "Small: " << sizeof(MatrixNode<EmptyAlgorithm<SmallTypes>>) / 8.0 << std::endl;
    std::cout << "Current: " << sizeof(MatrixNode<EmptyAlgorithm<CurrentTypes>>) / 8.0 << std::endl;

    return 0;
}