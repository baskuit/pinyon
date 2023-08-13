#include <surskit.hh>

int main () {

    RealType<mpq_class> x{};
    mpq_class y = static_cast<mpq_class>(x);
    // double z = static_cast<double>(y);

}