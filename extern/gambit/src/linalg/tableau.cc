//
// This file is part of Gambit
// Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/liblinear/tableau.imp
// Implementation of tableau class
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "tableau.h"

namespace Gambit
{

  namespace linalg
  {

    // ---------------------------------------------------------------------------
    //                   Tableau<double> method definitions
    // ---------------------------------------------------------------------------

    // Constructors and Destructor

    Tableau<double>::Tableau(const Matrix<double> &A, const Vector<double> &b)
        : TableauInterface<double>(A, b), B(*this), tmpcol(b.First(), b.Last())
    {
      Solve(b, solution);
    }

    Tableau<double>::Tableau(const Matrix<double> &A, const Array<int> &art,
                             const Vector<double> &b)
        : TableauInterface<double>(A, art, b), B(*this), tmpcol(b.First(), b.Last())
    {
      Solve(b, solution);
    }

    Tableau<double>::Tableau(const Tableau<double> &orig)
        : TableauInterface<double>(orig), B(orig.B, *this), tmpcol(orig.tmpcol)
    {
    }

    Tableau<double>::~Tableau() = default;

    Tableau<double> &Tableau<double>::operator=(const Tableau<double> &orig)
    {
      TableauInterface<double>::operator=(orig);
      if (this != &orig)
      {
        B.Copy(orig.B, *this);
        tmpcol = orig.tmpcol;
      }
      return *this;
    }

    //
    // pivoting operations
    //

    bool Tableau<double>::CanPivot(int outlabel, int col) const
    {
      const_cast<Tableau<double> *>(this)->SolveColumn(col, tmpcol);
      double val = tmpcol[basis.Find(outlabel)];
      if (val <= eps2 && val >= -eps2)
        return false;
      return true;
    }

    void Tableau<double>::Pivot(int outrow, int col)
    {
      if (!RowIndex(outrow) || !ValidIndex(col))
        throw BadPivot();

      // int outlabel = Label(outrow);
      // gout << "\noutrow:" << outrow;
      // gout << " outlabel: " << outlabel;
      // gout << " inlabel: " << col;
      // BigDump(gout);
      basis.Pivot(outrow, col);

      B.update(outrow, col);
      Solve(*b, solution);
      npivots++;
      // BigDump(gout);
    }

    void Tableau<double>::SolveColumn(int col, Vector<double> &out)
    {
      //** can we use tmpcol here, instead of allocating new vector?
      Vector<double> tmpcol2(MinRow(), MaxRow());
      GetColumn(col, tmpcol2);
      Solve(tmpcol2, out);
    }

    void Tableau<double>::BasisVector(Vector<double> &out) const
    {
      out = solution;
    }

    //
    // raw Tableau functions
    //

    void Tableau<double>::Refactor()
    {
      B.refactor();
      //** is re-solve necessary here?
      Solve(*b, solution);
    }

    void Tableau<double>::SetRefactor(int n)
    {
      B.SetRefactor(n);
    }

    void Tableau<double>::SetConst(const Vector<double> &bnew)
    {
      if (bnew.First() != b->First() || bnew.Last() != b->Last())
        throw DimensionException();
      b = &bnew;
      Solve(*b, solution);
    }

    //** this function is not currently used.  Drop it?
    void Tableau<double>::SetBasis(const Basis &in)
    {
      basis = in;
      B.refactor();
      Solve(*b, solution);
    }

    void Tableau<double>::Solve(const Vector<double> &b, Vector<double> &x)
    {
      B.solve(b, x);
    }

    void Tableau<double>::SolveT(const Vector<double> &c, Vector<double> &y)
    {
      B.solveT(c, y);
      //** gout << "\nTableau<double>::SolveT(), y: " << y;
      //   gout << "\nc: " << c;
    }

    bool Tableau<double>::IsFeasible()
    {
      //** is it really necessary to solve first here?
      Solve(*b, solution);
      for (int i = solution.First(); i <= solution.Last(); i++)
        if (solution[i] >= eps2)
          return false;
      return true;
    }

    bool Tableau<double>::IsLexMin()
    {
      int i, j;
      for (i = MinRow(); i <= MaxRow(); i++)
        if (EqZero(solution[i]))
          for (j = -MaxRow(); j < Label(i); j++)
            if (j != 0)
            {
              SolveColumn(j, tmpcol);
              if (LtZero(tmpcol[i]))
                return false;
            }
      return true;
    }

  } // end namespace Gambit::linalg

} // end namespace Gambit
