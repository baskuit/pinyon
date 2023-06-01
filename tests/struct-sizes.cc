// #include <surskit.hh>
#include <types/matrix.hh>
#include <tree/tree.hh>

#include <Eigen/Dense>

#include <iostream>
#include <vector>

struct SmallTypes {
    using VectorAction = std::vector<uint8_t>;
    using VectorInt = std::vector<uint8_t>;
    using VectorReal = std::vector<float>;
    struct ModelOutput {
        float x, y; std::vector<float> a, b;
    };
    using Observation = uint8_t[1];
    using Probability = float;
};

struct CurrentTypes {
    using VectorAction = std::vector<int>;
    using VectorInt = std::vector<int>;
    using VectorReal = std::vector<double>;

    struct ModelOutput {
        double x, y; std::vector<double> a, b;
    };
    using Observation = uint8_t[1];
    using Probability = double;
};

template <class _Types>
class EmptyAlgorithm {
public:
    struct MatrixStats {
        std::vector<float> row_gains, col_gains;
        std::vector<uint32_t> row_visits, col_visits;
    };
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

    ChanceNode<Algorithm> *child = nullptr;
    MatrixNode<Algorithm> *next = nullptr;

    uint8_t is_terminal = false;
    // bool is_expanded = false;

    typename Types::VectorAction row_actions;
    // typename Types::VectorAction col_actions;
    // typename Types::Observation obs;
    // typename Types::Probability prob;
    // typename Types::ModelOutput inference;
    typename Types::MatrixStats stats;
};


int main () {

    double bytes_per_block = 64;

    std::cout << "Small: " << sizeof(MatrixNode<EmptyAlgorithm<SmallTypes>>) / bytes_per_block << "" << std::endl;
    std::cout << "Current: " << sizeof(MatrixNode<EmptyAlgorithm<CurrentTypes>>) / bytes_per_block << std::endl;
    std::cout << "Smallest: " << sizeof(MatrixNodeSmaller<EmptyAlgorithm<SmallTypes>>) / bytes_per_block << "" << std::endl;

    const Eigen::MatrixXd matrix = Eigen::MatrixXd::Random(3, 3);
    const Matrix<double> libmatrix(3, 3);

    std::cout << "Eigen: " << sizeof(matrix) / bytes_per_block << "" << std::endl;
    std::cout << "Current: " << sizeof(libmatrix) / bytes_per_block << std::endl;


    return 0;
}