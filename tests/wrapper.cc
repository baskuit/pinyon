#include <surskit.hh>

using Types = MoldState<2>;

int main() {
    W::Types::State state{Types{}, typename Types::State{10}};

    size_t x, y;
    state.get_actions(x, y);
    std::cout << x + y << std::endl;

}