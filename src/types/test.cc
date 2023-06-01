#include <iostream>

#include <cmath>

using Int = long long unsigned int;

void convert(double &x, Int &p, Int &q, Int iterations)
{

    Int Integer_part = floor(x);
    double diff = (x - Integer_part);
    if (diff == 0) {
        return;
    }
    double greater_than_one = 1 / diff;


    if (iterations == 0) {
        p = Integer_part;
        q = 1;
        return;
    }

    convert(greater_than_one, p, q, --iterations);

    Int swap = p;
    p = q;
    q = swap;
    p += Integer_part * q;
}

double error (
    double x
) {
    Int p, q;
    Int iterations = 2;
    convert(x, p, q, iterations);
    double loss = x - p / (double) q;
    return loss;
}

int main()
{
    double x = 1;
    std::cout << error(1.4156456) << std::endl;
    return 0;
}