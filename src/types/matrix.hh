#pragma once

#include <types/value.hh>

#include <vector>
#include <algorithm>

template <typename T>
class DataMatrix : public std::vector<T>
{
public:
    size_t rows, cols;

    DataMatrix(){};
    DataMatrix(size_t rows, size_t cols) : std::vector<T>(rows * cols), rows(rows), cols(cols)
    {
    }

    template <typename U>
    explicit DataMatrix(const DataMatrix<U> &matrix)
    {
        rows = matrix.rows;
        cols = matrix.cols;
        std::transform(matrix.begin(), matrix.end(), this->begin(), [](const U &element)
                       { return static_cast<T>(element); });
    }

    void fill(size_t rows, size_t cols)
    {
        this->rows = rows;
        this->cols = cols;
        this->resize(rows * cols);
    }

    void fill(size_t rows, size_t cols, T value)
    {
        this->rows = rows;
        this->cols = cols;
        const size_t n = rows * cols;
        this->resize(n);
        std::fill(this->begin(), this->begin() + n, value);
    }

    T &get(size_t i, size_t j)
    {
        return (*this)[i * cols + j];
    }
};

template <typename T>
class Matrix : public std::vector<T>
{
public:
    size_t rows, cols;

    Matrix(){};
    Matrix(size_t rows, size_t cols) : std::vector<T>(rows * cols), rows(rows), cols(cols)
    {
    }

    template <typename U>
    Matrix(const Matrix<U> &matrix)
    {
        rows = matrix.rows;
        cols = matrix.cols;
        std::transform(matrix.begin(), matrix.end(), this->begin(), [](const U &element)
                       { return static_cast<T>(element); });
    }

    void fill(size_t rows, size_t cols)
    {
        this->rows = rows;
        this->cols = cols;
        this->resize(rows * cols);
    }

    void fill(size_t rows, size_t cols, T value)
    {
        this->rows = rows;
        this->cols = cols;
        const size_t n = rows * cols;
        this->resize(n);
        std::fill(this->begin(), this->begin() + n, value);
    }

    T &get(size_t i, size_t j)
    {
        return (*this)[i * cols + j];
    }

    Matrix operator*(T t) const
    {
        const Matrix &M = *this;
        Matrix output(M.rows, M.cols);
        std::transform(this->begin(), this->begin() + rows * cols, output.begin(),
                       [t](T a)
                       { return a * t; });
        return output;
    }

    Matrix operator+(T t) const
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
                       { return a + b; });
        return output;
    }

    T max() const
    {
        const size_t entries = rows * cols;
        return *std::max_element(this->begin(), this->begin() + entries);
    }

    T min() const
    {
        const size_t entries = rows * cols;
        return *std::min_element(this->begin(), this->begin() + entries);
    }

    void print() const
    {
        for (size_t row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (size_t col_idx = 0; col_idx < cols; ++col_idx)
            {
                std::cout << (*this)[row_idx * cols + col_idx].row_value.value.get_d() << ' '; // TODO REMOVE
            }
            std::cout << std::endl;
        }
    }
};

template <typename T>
class Matrix<PairReal<T>> : public std::vector<PairReal<T>>
{
public:
    size_t rows, cols;

    Matrix(){};
    Matrix(size_t rows, size_t cols) : std::vector<PairReal<T>>(rows * cols), rows(rows), cols(cols)
    {
    }

    operator Matrix<PairReal<double>>() const
    {
        Matrix<PairReal<T>> output{this->rows, this->cols};
        for (int entry_idx = 0; entry_idx < rows * cols; ++entry_idx)
        {
            auto value = (*this)[entry_idx];
            output[entry_idx] = PairReal<T>{static_cast<double>(value.get_row_value()), static_cast<double>(value.get_col_value())};
        }
        return output;
    }

    void fill(size_t rows, size_t cols)
    {
        this->rows = rows;
        this->cols = cols;
        this->resize(rows * cols);
    }

    void fill(size_t rows, size_t cols, PairReal<T> value)
    {
        this->rows = rows;
        this->cols = cols;
        const size_t n = rows * cols;
        this->resize(n);
        std::fill(this->begin(), this->begin() + n, value);
    }

    PairReal<T> &get(size_t i, size_t j)
    {
        return (*this)[i * cols + j];
    }

    Matrix operator*(PairReal<T> t) const
    {
        const Matrix &M = *this;
        Matrix output(M.rows, M.cols);
        std::transform(this->begin(), this->begin() + rows * cols, output.begin(),
                       [t](PairReal<T> a)
                       { return a * t; });
        return output;
    }

    Matrix operator+(PairReal<T> t) const
    {
        const Matrix &M = *this;
        Matrix output(M.rows, M.cols);
        std::transform(this->begin(), this->begin() + rows * cols, output.begin(),
                       [t](PairReal<T> a)
                       { return a + t; });
        return output;
    }

    template <typename U>
    Matrix operator+(U t) const
    {
        const Matrix &M = *this;
        Matrix output(M.rows, M.cols);
        std::transform(this->begin(), this->begin() + rows * cols, output.begin(),
                       [t](U a)
                       { return a + t; });
        return output;
    }

    Matrix operator+(const Matrix &t) const
    {
        const Matrix &M = *this;
        Matrix output(M.rows, M.cols);
        std::transform(this->begin(), this->begin() + rows * cols, t.begin(), output.begin(),
                       [](PairReal<T> a, PairReal<T> b)
                       { return a + b; });
        return output;
    }

    T max() const
    {
        const size_t entries = rows * cols;
        auto max_row = std::max_element(this->begin(), this->begin() + entries,
                                        [](const auto &a, const auto &b)
                                        {
                                            return a.get_row_value() < b.get_row_value();
                                        });
        auto max_col = std::max_element(this->begin(), this->begin() + entries,
                                        [](const auto &a, const auto &b)
                                        {
                                            return a.get_col_value() < b.get_col_value();
                                        });
        return std::max(max_row->get_row_value(), max_col->get_col_value());
    }

    T min() const
    {
        const size_t entries = rows * cols;
        auto min_row = std::min_element(this->begin(), this->begin() + entries,
                                        [](const auto &a, const auto &b)
                                        {
                                            return a.get_row_value() < b.get_row_value();
                                        });
        auto min_col = std::min_element(this->begin(), this->begin() + entries,
                                        [](const auto &a, const auto &b)
                                        {
                                            return a.get_col_value() < b.get_col_value();
                                        });
        return std::min(min_row->get_row_value(), min_col->get_col_value());
    }

    void print() const __attribute__((noinline))
    {
        for (size_t row_idx = 0; row_idx < rows; ++row_idx)
        {
            for (size_t col_idx = 0; col_idx < cols; ++col_idx)
            {
                std::cout << (*this)[row_idx * cols + col_idx] << ' ';
            }
            std::cout << std::endl;
        }
    }
};