#include <surskit.hh>

using OldTypes = RandomTree<>;
using Types = TreeBanditThreadPool<Rand<EmptyModel<RandomTree<>>>>;

int main () {
    
    SearchModel<Types>::State x{};
    // SearchModel<Types>::State state{10};
    SearchModel<Types>::PRNG device{};
}