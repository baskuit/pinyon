#include <surskit.hh>

using Types = MoldState<2>;

int main() {
    W::Types::State state{Types{}, typename Types::State{10}};

    W::Types::VectorAction x, y;
    state.get_actions(x, y);
    std::cout << x.size() + y.size() << std::endl;

}