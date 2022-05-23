#include <array>

// WIP

typedef int Action;



struct State;
template <int rows, int cols>
struct MatrixNode {

    std::array<int, rows> gains0;
    std::array<int, cols> gains1;
    int dims = rows*cols;
    std::array<int, dims> children;

};



int main () {
    MatrixNode<4, 4> x;
}