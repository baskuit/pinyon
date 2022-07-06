#pragma once
#include <math.h>
#include <assert.h>
#include <stdexcept>
#include <algorithm>

#include "random.hh"
#include "rational.hh"

namespace math {

    void power_norm(double* input, int k, double power, double* output) {
        double sum = 0;
        for (int i = 0; i < k; ++i) {
            double x = std::pow(input[i], power);
            output[i] = x;
            sum += x;
        }
        for (int i = 0; i < k; ++i) {
            output[i] = output[i]/sum;
        }
    }

    template <typename T, int size>
    void power_norm(std::array<T, size> input, int k, double power, std::array<T, size>& output) {
        double sum = 0;
        for (int i = 0; i < k; ++i) {
            double x = std::pow(input[i], power);
            output[i] = x;
            sum += x;
        }
        for (int i = 0; i < k; ++i) {
            output[i] = output[i]/sum;
        }
    }

    template <typename T, typename U, int size>
    void power_norm(std::array<T, size> input, int k, double power, std::array<U, size>& output) {
        double sum = 0;
        for (int i = 0; i < k; ++i) {
            U x = std::pow(static_cast<U>(input[i]), power);
            output[i] = x;
            sum += x;
        }
        for (int i = 0; i < k; ++i) {
            output[i] = output[i]/sum;
        }
    }

}

namespace Linear {

template <typename T, int size>
struct Vector {
    int length = 0;
    std::array<T, size> data;

    template <int k>
    Vector<T, size> (std::array<T, k>& array) {
        length = std::min(size, k);
        for (int i = 0; i < length; ++i) {
            data[i] = array[i];
        }
    }

    Vector (int length) : length(length) {
        for (int i = 0; i < length; ++i) {
            data[i] = Rational(0, 1);
        }
    }
};

template <typename T, int size>
struct Matrix {
    int rows = 0;
    int cols = 0;

    Matrix () {}

    Matrix (int rows, int cols) :
        rows(rows), cols(cols) {}

    Matrix<T,size> operator* (const Matrix<T, size>& M);
    virtual T get (int row_idx, int col_idx) = 0;
    virtual void set (int row_idx, int col_idx, T value) = 0;
};

template <typename T, int size>
struct Bimatrix {
    int rows = 0;
    int cols = 0;

    Bimatrix () {}

    Bimatrix (int rows, int cols) :
        rows(rows), cols(cols) {}

    virtual T get0 (int row_idx, int col_idx) = 0;
    virtual T get1 (int row_idx, int col_idx) = 0;
    virtual void set0 (int row_idx, int col_idx, T value) = 0;
    virtual void set1 (int row_idx, int col_idx, T value) = 0;
};

template <typename T, int size>
struct Matrix2D : Matrix<T, size> {
    std::array<std::array<T, size>, size> data;

    Matrix2D () {}

    Matrix2D (int rows, int cols) :
    Matrix<T, size>(rows, cols) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = Rational(1, 2);
            }
        }
    }

    Matrix2D (int rows, int cols, Rational value) :
    Matrix<T, size>(rows, cols) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = value;
            }
        }
    }

    Matrix2D<T, size> operator* (const Matrix2D<T, size>& M) {
        assert(this->cols = M.rows);
        Matrix2D<T, size> product(this->rows, M.cols, Rational(0, 1));
        for (int i = 0; i < this->rows; ++i) {
            for (int j = 0; j < M.cols; ++j) {
                for (int k = 0; k < this->cols; ++k) {
                    product.data[i][j] = product.data[i][j] + (this->data[i][k] * M.data[k][j]);
                }
            }
        }
        return product;
    }

    T get (int row_idx, int col_idx) {
        return data[row_idx][col_idx];
    }

    void set (int row_idx, int col_idx, T value) {
        data[row_idx][col_idx] = value;
    }

    void print () {
        for (int i = 0; i < this->rows; ++i) {
            std::cout << "[";
            for (int j = 0; j < this->cols; ++j) {
                std::cout << get(i, j) << ", ";
            }
            std::cout << "]" << std::endl;
        }
    }
};

template <typename T, int size>
struct Bimatrix2D : Bimatrix<T, size> {
    std::array<std::array<T, size>, size> data0;
    std::array<std::array<T, size>, size> data1;

    Bimatrix2D () {}

    Bimatrix2D (int rows, int cols) :
    Bimatrix<T, size>(rows, cols) {
        // for (int i = 0; i < rows; ++i) {
        //     for (int j = 0; j < cols; ++j) {
        //         data0[i][j] = Rational(1, 2);
        //         data1[i][j] = Rational(1, 2);
        //     }
        // }
    }

    T get0 (int row_idx, int col_idx) {return data0[row_idx][col_idx];}
    T get1 (int row_idx, int col_idx) {return data1[row_idx][col_idx];}
    void set0 (int row_idx, int col_idx, T value) {data0[row_idx][col_idx] = value;}
    void set1 (int row_idx, int col_idx, T value) {data1[row_idx][col_idx] = value;}

    void print () {
        for (int i = 0; i < this->rows; ++i) {
            std::cout << "[";
            for (int j = 0; j < this->cols; ++j) {
                std::cout << get0(i, j) << ", ";
            }
            std::cout << "]" << std::endl;
        }
        for (int i = 0; i < this->rows; ++i) {
            std::cout << "[";
            for (int j = 0; j < this->cols; ++j) {
                std::cout << get1(i, j) << ", ";
            }
            std::cout << "]" << std::endl;
        }
    }
    //TODO delete this
    Bimatrix2D<T, size> operator+ (const Matrix2D<int, size>& M) {
        assert(this->cols = M.rows);
        Bimatrix2D<T, size> product(this->rows, M.cols);
        for (int i = 0; i < this->rows; ++i) {
            for (int j = 0; j < M.cols; ++j) {
                product.data0[i][j] = this->data0[i][j] / M.data[i][j];
                product.data1[i][j] = this->data1[i][j] / M.data[i][j];
            }
        }
        return product;
    }
};

}; // End namespace Linear

namespace Bandit {

template <typename T, int size>
void softmax (
    std::array<T, size>& forecast, 
    std::array<T, size>& gains, 
    int k,
    double eta
) {
    double max = 0;
    for (int i = 0; i < k; ++i) {
        double x = gains[i];
        if (x > max) {
            max = x;
        } 
    }
    double sum = 0;
    for (int i = 0; i < k; ++i) {
        gains[i] -= max;
        double x = gains[i];
        double y = std::exp(x * eta);
        forecast[i] = y;
        sum += y;
    }
    for (int i = 0; i < k; ++i) {
        forecast[i] /= sum;
    }
};

template <typename T, int size>
void forecast (
    int rows,
    int cols,
    int time,
    std::array<T, size>& forecast0,
    std::array<T, size>& forecast1,
    std::array<T, size>& gains0,
    std::array<T, size>& gains1
) {
    if (rows == 1) {
        forecast0[0] = Rational(1, 1);
    } else {
        const double eta = .95 * sqrt(log(rows)/(time*rows));
        const double gamma_ = 1.05 * sqrt(rows*log(rows)/time);
        const double gamma = gamma_ < 1 ? gamma_ : 1;
        softmax<T, size>(forecast0, gains0, rows, eta);
        for (int row_idx = 0; row_idx < rows; ++row_idx) {
            double x = (1 - gamma) * forecast0[row_idx] + (gamma) / rows;
            forecast0[row_idx] = x;
        
        }
    }
    if (cols == 1) {
        forecast1[0] = Rational(1, 1);
    } else {
        const double eta = .95 * sqrt(log(cols)/(time*cols));
        const double gamma_ = 1.05 * sqrt(cols*log(cols)/time);
        const double gamma = gamma_ < 1 ? gamma_ : 1;
        softmax<T, size>(forecast1, gains1, cols, eta);
        for (int col_idx = 0; col_idx < cols; ++col_idx) {
            double x = (1 - gamma) * forecast1[col_idx] + (gamma) / cols;
            forecast1[col_idx] = x;
        }
    }
}

template <typename T, int size>
void SolveMatrix (
    prng& device,
    int playouts,
    Linear::Matrix<T, size>& M,
    std::array<T, size>& empirical0,
    std::array<T, size>& empirical1
) {
    std::array<T, size> forecast0 = {Rational(0, 1)};
    std::array<T, size> forecast1 = {Rational(0, 1)};
    std::array<T, size> gains0 = {Rational(0, 1)};
    std::array<T, size> gains1 = {Rational(0, 1)};

    for (int playout = 0; playout < playouts; ++playout) {
        forecast<T, size>(M.rows, M.cols, playouts, forecast0, forecast1, gains0, gains1);
        int row_idx = device.sample_pdf<T, size>(forecast0, M.rows);
        int col_idx = device.sample_pdf<T, size>(forecast1, M.cols);
        ++empirical0[row_idx];
        ++empirical1[col_idx];
        T u = M.get(row_idx, col_idx);
        gains0[row_idx] += u / forecast0[row_idx];
        gains1[col_idx] += (1-u) / forecast1[col_idx];
    }

    math::power_norm<T, size>(empirical0, M.rows, 1, empirical0);
    math::power_norm<T, size>(empirical1, M.cols, 1, empirical1);
}

template <typename T, int size>
T exploitability (
    Linear::Matrix<T, size>& M,
    std::array<T, size>& strategy0,
    std::array<T, size>& strategy1
) {
    std::array<T, size> best0 = {Rational(0, 1)};
    std::array<T, size> best1 = {Rational(0, 1)};
    for (int row_idx = 0; row_idx < M.rows; ++row_idx) {
        for (int col_idx = 0; col_idx < M.cols; ++col_idx) {
            const T u = M.get(row_idx, col_idx);
            best0[row_idx] += u * strategy1[col_idx];
            best1[col_idx] -= u * strategy0[row_idx];
        }
    }
    return *std::max_element(best0.begin(), best0.begin()+M.rows) + *std::max_element(best1.begin(), best1.begin()+M.cols);
}

template <typename T, int size>
// no matching function for call to 'SolveBimatrix(prng&, int, Linear::Bimatrix2D<double, 9>&, std::array<double, 9>&, std::array<double, 9>&)'
void SolveBimatrix (
    prng& device,
    int playouts,
    Linear::Bimatrix<T, size>& M,
    std::array<T, size>& empirical0,
    std::array<T, size>& empirical1
) {
    std::array<T, size> forecast0 = {Rational(0, 1)};
    std::array<T, size> forecast1 = {Rational(0, 1)};
    std::array<T, size> gains0 = {Rational(0, 1)};
    std::array<T, size> gains1 = {Rational(0, 1)};

    for (int playout = 0; playout < playouts; ++playout) {
        forecast<T, size>(M.rows, M.cols, playouts, forecast0, forecast1, gains0, gains1);
        int row_idx = device.sample_pdf<T, size>(forecast0, M.rows);
        int col_idx = device.sample_pdf<T, size>(forecast1, M.cols);
        ++empirical0[row_idx];
        ++empirical1[col_idx];
        gains0[row_idx] += M.get0(row_idx, col_idx) / forecast0[row_idx];
        gains1[col_idx] += M.get1(row_idx, col_idx) / forecast1[col_idx];
    }

    math::power_norm<T, size>(empirical0, M.rows, 1, empirical0);
    math::power_norm<T, size>(empirical1, M.cols, 1, empirical1);
}

}; // End namespace Bandit