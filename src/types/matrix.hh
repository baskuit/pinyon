#pragma once

#include <vector>
#include <algorithm>
#include <types/value.hh>

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
        std::transform(matrix.begin(), matrix.end(), std::back_inserter(*this), [](const U &element)
                       { return static_cast<T>(element); });
    } // TODO check lol

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
};

template <typename U, bool IS_CONSTANT_SUM, int NUM, int DEN>
class Matrix<ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN>> : public std::vector<ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN>>
{
public:
    size_t rows, cols;

    Matrix(){};
    Matrix(size_t rows, size_t cols) : std::vector<ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN>>(rows * cols), rows(rows), cols(cols)
    {
    }

    operator Matrix<PairDouble>() const
    {
        Matrix<PairDouble> output{this->rows, this->cols};
        for (int entry_idx = 0; entry_idx < rows * cols; ++entry_idx)
        {
            auto value = (*this)[entry_idx];
            output[entry_idx] = PairDouble{static_cast<double>(value.get_row_value()), static_cast<double>(value.get_col_value())};
        }
        return output;
    }

    void fill(size_t rows, size_t cols)
    {
        this->rows = rows;
        this->cols = cols;
        this->resize(rows * cols);
    }

    void fill(size_t rows, size_t cols, ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN> value)
    {
        this->rows = rows;
        this->cols = cols;
        const size_t n = rows * cols;
        this->resize(n);
        std::fill(this->begin(), this->begin() + n, value);
    }

    ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN> &get(size_t i, size_t j)
    {
        return (*this)[i * cols + j];
    }

    Matrix operator*(ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN> t) const
    {
        const Matrix &M = *this;
        Matrix output(M.rows, M.cols);
        std::transform(this->begin(), this->begin() + rows * cols, output.begin(),
                       [t](ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN> a)
                       { return a * t; });
        return output;
    }

    Matrix operator+(ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN> t) const
    {
        const Matrix &M = *this;
        Matrix output(M.rows, M.cols);
        std::transform(this->begin(), this->begin() + rows * cols, output.begin(),
                       [t](ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN> a)
                       { return a + t; });
        return output;
    }

    template <typename T>
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
                       [](ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN> a, ValueStruct<U, IS_CONSTANT_SUM, NUM, DEN> b)
                       { return a + b; });
        return output;
    }

    U max() const
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

    U min() const
    {
        const size_t entries = rows * cols;
        auto min_row = std::min_element(this->begin(), this->begin() + entries,
                                        [](const auto &a, const auto &b)
                                        {
                                            return a.get_row_value() > b.get_row_value();
                                        });
        auto min_col = std::min_element(this->begin(), this->begin() + entries,
                                        [](const auto &a, const auto &b)
                                        {
                                            return a.get_col_value() > b.get_col_value();
                                        });
        return std::min(min_row->get_row_value(), min_col->get_col_value());
    } // TODO specialize for constant sum? LMAO
};