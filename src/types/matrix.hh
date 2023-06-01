#pragma once

#include <vector>
#include <algorithm>

template <typename T>
class Matrix : public std::vector<T>
{
public:
    int rows, cols;

    Matrix(){};
    Matrix(int rows, int cols) : std::vector<T>(rows * cols), rows(rows), cols(cols)
    {
    }

    void fill(int rows, int cols)
    {
        this->rows = rows;
        this->cols = cols;
        this->resize(rows * cols);
    }

    void fill(int rows, int cols, T value)
    {
        this->rows = rows;
        this->cols = cols;
        const int n = rows * cols;
        this->resize(n);
        std::fill(this->begin(), this->begin() + n, value);
    }

    T &get(int i, int j)
    {
        return (*this)[i * cols + j];
    }

    Matrix operator*(T t)
    {
        const Matrix &M = *this;
        Matrix output(M.rows, M.cols);
        std::transform(this->begin(), this->begin() + rows * cols, output.begin(),
                       [t](T a)
                       { return a * t; });
        return output;
    }
    Matrix operator+(T t)
    {
        const Matrix &M = *this;
        Matrix output(M.rows, M.cols);
        std::transform(this->begin(), this->begin() + rows * cols, output.begin(),
                       [t](T a)
                       { return a + t; });
        return output;
    }

    Matrix operator+(const Matrix &t) const
    {
        const Matrix &M = *this;
        Matrix output(M.rows, M.cols);
        std::transform(this->begin(), this->begin() + rows * cols, t.begin(), output.begin(),
                       [](T a, T b)
                       { return a + b; }); // Perform element-wise addition
        return output;
    }

    T max()
    {
        const int entries = rows * cols;
        return *std::max_element(this->begin(), this->begin() + entries);
    }

    T min()
    {
        const int entries = rows * cols;
        return *std::min_element(this->begin(), this->begin() + entries);
    }
};
