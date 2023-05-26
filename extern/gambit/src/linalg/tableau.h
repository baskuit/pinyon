//
// This file is part of Gambit
// Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/liblinear/tableau.h
// Interface to tableau class
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

#ifndef TABLEAU_H
#define TABLEAU_H

#include "btableau.h"
#include "ludecomp.h"

namespace Gambit
{

  namespace linalg
  {

    template <class T>
    class Tableau;
    template <class T>
    class LPTableau;

    // ---------------------------------------------------------------------------
    // We have different implementations of Tableau for double and gbtRational,
    // but with the same interface
    // ---------------------------------------------------------------------------

    template <>
    class Tableau<double> : public TableauInterface<double>
    {
    public:
      // constructors and destructors
      Tableau(const Matrix<double> &A, const Vector<double> &b);
      Tableau(const Matrix<double> &A, const Array<int> &art,
              const Vector<double> &b);
      Tableau(const Tableau<double> &);
      ~Tableau() override;

      Tableau<double> &operator=(const Tableau<double> &);

      // pivoting
      bool CanPivot(int outgoing, int incoming) const override;
      void Pivot(int outrow, int col) override;                         // pivot -- outgoing is row, incoming is column
      void BasisVector(Vector<double> &x) const override;               // solve M x = (*b)
      void SolveColumn(int, Vector<double> &) override;                 // column in new basis
      void Solve(const Vector<double> &b, Vector<double> &x) override;  // solve M x = b
      void SolveT(const Vector<double> &c, Vector<double> &y) override; // solve y M = c

      // raw Tableau functions

      void Refactor() override;
      void SetRefactor(int) override;

      void SetConst(const Vector<double> &bnew);
      void SetBasis(const Basis &); // set new Tableau

      bool IsFeasible();
      bool IsLexMin();

    private:
      // The LU decomposition of the tableau
      LUdecomp<double> B;
      // A temporary column vector, to avoid frequent allocation
      mutable Vector<double> tmpcol;
    };

  } // end namespace Gambit::linalg

} // end namespace Gambit

#endif // TABLEAU_H
