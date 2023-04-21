#pragma once

#include "math.hh"
#include "gambit.h"
#include "linalg/vertenum.imp"
#include "state/test_states.hh"

namespace LibGambit
{

    bool EqZero(const double &x)
    {
        const double eps = std::pow(10.0, -15.0);
        return (x <= eps && x >= -eps);
    }

    using BFS = Gambit::linalg::BFS<double>;

    template <class TypeList>
    void solve_bimatrix(
        typename TypeList::MatrixReal &row_payoff_matrix,
        typename TypeList::MatrixReal &col_payoff_matrix,
        typename TypeList::VectorReal &row_strategy,
        typename TypeList::VectorReal &col_strategy)
    {
        const int rows = row_payoff_matrix.rows;
        const int cols = row_payoff_matrix.cols;
        /*
        Accept surksit style matrices and use gambit protocol to solve
        */

        // Construct matrices A1, A2 // T = double
        Gambit::Matrix<double> A1(1, rows,
                                  1, cols);
        Gambit::Matrix<double> A2(1, cols,
                                  1, rows);

        const double max = std::max(row_payoff_matrix.max(), col_payoff_matrix.max());
        const double min = std::min(row_payoff_matrix.min(), col_payoff_matrix.min());
        if (max == min)
        {
            return;
        }
        const double fac = 1 / (max - min);

        for (size_t i = 1; i <= rows; i++)
        {
            for (size_t j = 1; j <= cols; j++)
            {
                A1(i, j) = fac * (row_payoff_matrix.get(i-1, j-1) - min);
                A2(j, i) = fac * (col_payoff_matrix.get(i-1, j-1) - min);
            }
        }

        // Construct vectors b1, b2 // Length rows, cols with default value -1
        Gambit::Vector<double> b1(1, rows);
        Gambit::Vector<double> b2(1, cols);
        b1 = (double)-1;
        b2 = (double)-1;

        // enumerate vertices of A1 x + b1 <= 0 and A2 x + b2 <= 0
        Gambit::linalg::VertexEnumerator<double> poly1(A1, b1); // Instantiantion calls method 'Enum()'
        Gambit::linalg::VertexEnumerator<double> poly2(A2, b2);

        const Gambit::List<BFS> &verts1(poly1.VertexList()); // List is computed in prior instantiation. This simply returns ref to that LIst member
        const Gambit::List<BFS> &verts2(poly2.VertexList());
        int m_v1 = verts1.Length();
        int m_v2 = verts2.Length();

        Gambit::Array<int> vert1id(m_v1);
        Gambit::Array<int> vert2id(m_v2);
        for (int i = 1; i <= vert1id.Length(); vert1id[i++] = 0)
            ;
        for (int i = 1; i <= vert2id.Length(); vert2id[i++] = 0)
            ; // Initialize vert1id, 2id lists with resp. lengths as 0

        int i = 0;
        int id1 = 0, id2 = 0;

        for (int i2 = 2; i2 <= m_v2; i2++)
        {
            const BFS &bfs1 = verts2[i2];
            i++;
            for (int i1 = 2; i1 <= m_v1; i1++)
            {
                const BFS &bfs2 = verts1[i1];

                // check if solution is nash
                // need only check complementarity, since it is feasible
                bool nash = true;
                for (size_t k = 1; nash && k <= rows; k++)
                {
                    if (bfs1.count(k) && bfs2.count(-k))
                    {
                        nash = EqZero(bfs1[k] * bfs2[-k]); // each value for
                    }
                }
                for (size_t k = 1; nash && k <= cols; k++)
                {
                    if (bfs2.count(k) && bfs1.count(-k))
                    {
                        nash = EqZero(bfs2[k] * bfs1[-k]);
                    }
                }

                if (nash)
                {
                    double row_sum = 0;
                    double col_sum = 0;
                    for (size_t k = 1; k <= rows; k++)
                    {
                        if (bfs1.count(k))
                        {
                            const double x = -bfs1[k];
                            row_strategy[k - 1] = x;
                            row_sum += x;
                        }
                        else
                        {
                            row_strategy[k - 1] = 0;
                        }
                    }
                    for (size_t k = 1; k <= cols; k++)
                    {
                        if (bfs2.count(k))
                        {
                            const double x = -bfs2[k];
                            col_strategy[k - 1] = x;
                            col_sum += x;
                        }
                        else
                        {
                            col_strategy[k - 1] = 0;
                        }
                    }
                    for (size_t k = 1; k <= rows; k++)
                    {
                        row_strategy[k - 1] /= row_sum;
                    }
                    for (size_t k = 1; k <= cols; k++)
                    {
                        col_strategy[k - 1] /= col_sum;
                    }
                    return; //TODO continue calculating?
                }
            }
        }
    }

}; // End namespace Gambit