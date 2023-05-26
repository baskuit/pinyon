#include "lib.h"
#include "float.h"

void prat_(const char *name, lrs_mp Nin, lrs_mp Din)
/*print the long precision rational Nt/Dt  */
{
  lrs_mp Nt, Dt;
  copy_(Nt, Nin);
  copy_(Dt, Din);
  reduce(Nt, Dt);
  // if (sign(Nt) != NEG)
  //   fprintf(lrs_ofp, " ");

  // fprintf(lrs_ofp, "%s%lld", name, *Nt);
  // if (*Dt != 1)
  //   fprintf(lrs_ofp, "/%lld", *Dt);

  // fprintf(lrs_ofp, " ");
}

int lrs_solve_nash_(game *g, long long *row_data, long long *col_data)
{
  lrs_dic *P1;      /* structure for holding current dictionary and indices */
  lrs_dat *Q1, *Q2; /* structure for holding static problem data            */

  lrs_mp_vector output1; /* holds one line of output; ray,vertex,facet,linearity */
  lrs_mp_vector output2; /* holds one line of output; ray,vertex,facet,linearity */
  lrs_mp_matrix Lin;     /* holds input linearities if any are found             */
  lrs_mp_matrix A2orig;
  lrs_dic *P2orig; /* we will save player 2's dictionary in getabasis      */

  long *linindex; /* for faster restart of player 2                       */

  long col; /* output column index for dictionary                   */
  long startcol = 0;
  long prune = FALSE;  /* if TRUE, getnextbasis will prune tree and backtrack  */
  long numequilib = 0; /* number of nash equilibria found                      */
  long oldnum = 0;

  /* global variables lrs_ifp and lrs_ofp are file pointers for input and output   */
  /* they default to stdin and stdout, but may be overidden by command line parms. */

  /*********************************************************************************/
  /* Step 1: Allocate lrs_dat, lrs_dic and set up the problem                      */
  /*********************************************************************************/
  // FirstTime = TRUE; /* This is done for each new game */

  Q1 = lrs_alloc_dat("LRS globals"); /* allocate and init structure for static problem data */
  if (Q1 == NULL)
  {
    return 0;
  }

  Q1->nash = TRUE;
  Q1->n = g->nstrats[ROW] + 2;
  Q1->m = g->nstrats[ROW] + g->nstrats[COL] + 1;

  Q1->debug = Debug_flag;
  Q1->verbose = Verbose_flag;

  double max_reward = DBL_MIN;
  double max_row_reward;
  double reward;
  long long col_data_copy[Q1->n];

  P1 = lrs_alloc_dic(Q1); /* allocate and initialize lrs_dic */
  if (P1 == NULL)
  {
    return 0;
  }

  BuildRep(P1, Q1, g, 1, 0);

  output1 = lrs_alloc_mp_vector(Q1->n + Q1->m); /* output holds one line of output from dictionary     */

  /* allocate and init structure for player 2's problem data */
  Q2 = lrs_alloc_dat("LRS globals");
  if (Q2 == NULL)
  {
    return 0;
  }

  Q2->debug = Debug_flag;
  Q2->verbose = Verbose_flag;

  Q2->nash = TRUE;
  Q2->n = g->nstrats[COL] + 2;
  Q2->m = g->nstrats[ROW] + g->nstrats[COL] + 1;

  P2orig = lrs_alloc_dic(Q2); /* allocate and initialize lrs_dic */
  if (P2orig == NULL)
  {
    return 0;
  }
  BuildRep(P2orig, Q2, g, 0, 1);
  A2orig = P2orig->A;

  output2 = lrs_alloc_mp_vector(Q1->n + Q1->m); /* output holds one line of output from dictionary     */

  linindex = calloc((P2orig->m + P2orig->d + 2), sizeof(long)); /* for next time */

  // fprintf(lrs_ofp, "\n");
  //  fprintf (lrs_ofp, "***** %ld %ld rational\n", Q1->n, Q2->n);

  /*********************************************************************************/
  /* Step 2: Find a starting cobasis from default of specified order               */
  /*         P1 is created to hold  active dictionary data and may be cached       */
  /*         Lin is created if necessary to hold linearity space                   */
  /*         Print linearity space if any, and retrieve output from first dict.    */
  /*********************************************************************************/

  if (!lrs_getfirstbasis(&P1, Q1, &Lin, TRUE))
    return 1;

  // if (Q1->dualdeg)
  // {
  //   printf("\n*Warning! Dual degenerate, ouput may be incomplete");
  //   printf("\n*Recommendation: Add dualperturb option before maximize in first input file\n");
  // }

  // if (Q1->unbounded)
  // {
  //   printf("\n*Warning! Unbounded starting dictionary for p1, output may be incomplete");
  //   printf("\n*Recommendation: Change/remove maximize option, or include bounds \n");
  // }

  /* Pivot to a starting dictionary                      */
  /* There may have been column redundancy               */
  /* If so the linearity space is obtained and redundant */
  /* columns are removed. User can access linearity space */
  /* from lrs_mp_matrix Lin dimensions nredundcol x d+1  */

  if (Q1->homogeneous && Q1->hull)
    startcol++; /* col zero not treated as redundant   */
  col = startcol;
  // for (col = startcol; col < Q1->nredundcol; col++) /* print linearity space               */
  //   lrs_printoutput(Q1, Lin[col]);                  /* Array Lin[][] holds the coeffs.     */

  /*********************************************************************************/
  /* Step 3: Terminate if lponly option set, otherwise initiate a reverse          */
  /*         search from the starting dictionary. Get output for each new dict.    */
  /*********************************************************************************/

  /* We initiate reverse search from this dictionary       */
  /* getting new dictionaries until the search is complete */
  /* User can access each output line from output which is */
  /* vertex/ray/facet from the lrs_mp_vector output         */
  /* prune is TRUE if tree should be pruned at current node */
  do
  {
    prune = lrs_checkbound(P1, Q1);
    if (!prune && lrs_getsolution(P1, Q1, output1, col))
    {

      oldnum = numequilib;

      // nash2_main_(P1, Q1, P2orig, Q2, &numequilib, output2, linindex, row_strategy);
      lrs_dic *P2;       /* This can get resized, cached etc. Loaded from P2orig */
      lrs_mp_matrix Lin; /* holds input linearities if any are found             */
      long col;          /* output column index for dictionary                   */
      long startcol = 0;
      long prune = FALSE; /* if TRUE, getnextbasis will prune tree and backtrack  */
      long nlinearity;
      long *linearity;
      // static long firstwarning = TRUE;   /* FALSE if dual deg warning for Q2 already given     */
      // static long firstunbounded = TRUE; /* FALSE if dual deg warning for Q2 already given     */

      long i, j;

      /* global variables lrs_ifp and lrs_ofp are file pointers for input and output   */
      /* they default to stdin and stdout, but may be overidden by command line parms. */

      /*********************************************************************************/
      /* Step 1: Allocate lrs_dat, lrs_dic and set up the problem                      */
      /*********************************************************************************/

      P2 = lrs_getdic(Q2);
      copy_dict(Q2, P2, P2orig);

      /* Here we take the linearities generated by the current vertex of player 1*/
      /* and append them to the linearity in player 2's input matrix             */
      /* next is the key magic linking player 1 and 2 */
      /* be careful if you mess with this!            */

      linearity = Q2->linearity;
      nlinearity = 0;
      for (i = Q1->lastdv + 1; i <= P1->m; i++)
      {
        if (!zero_(P1->A[P1->Row[i]][0]))
        {
          j = Q1->inequality[P1->B[i] - Q1->lastdv];
          if (Q1->nlinearity == 0 || j < Q1->linearity[0])
            linearity[nlinearity++] = j;
        }
      }
      /* add back in the linearity for probs summing to one */
      if (Q1->nlinearity > 0)
        linearity[nlinearity++] = Q1->linearity[0];

      /*sort linearities */
      for (i = 1; i < nlinearity; i++)
        reorder(linearity, nlinearity);

      // if (Q2->verbose)
      // {
      //   fprintf(lrs_ofp, "\np2: linearities %ld", nlinearity);
      //   for (i = 0; i < nlinearity; i++)
      //     fprintf(lrs_ofp, " %ld", linearity[i]);
      // }

      Q2->nlinearity = nlinearity;
      Q2->polytope = FALSE;

      /*********************************************************************************/
      /* Step 2: Find a starting cobasis from default of specified order               */
      /*         P2 is created to hold  active dictionary data and may be cached        */
      /*         Lin is created if necessary to hold linearity space                   */
      /*         Print linearity space if any, and retrieve output from first dict.    */
      /*********************************************************************************/

      if (!lrs_getfirstbasis2(&P2, Q2, P2orig, &Lin, TRUE, linindex))
        goto sayonara;
      // if (firstwarning && Q2->dualdeg)
      // {
      //   firstwarning = FALSE;
      //   printf("\n*Warning! Dual degenerate, ouput may be incomplete");
      //   printf("\n*Recommendation: Add dualperturb option before maximize in second input file\n");
      // }
      // if (firstunbounded && Q2->unbounded)
      // {
      //   firstunbounded = FALSE;
      //   printf("\n*Warning! Unbounded starting dictionary for p2, output may be incomplete");
      //   printf("\n*Recommendation: Change/remove maximize option, or include bounds \n");
      // }

      /* Pivot to a starting dictionary                      */
      /* There may have been column redundancy               */
      /* If so the linearity space is obtained and redundant */
      /* columns are removed. User can access linearity space */
      /* from lrs_mp_matrix Lin dimensions nredundcol x d+1  */

      if (Q2->homogeneous && Q2->hull)
        startcol++; /* col zero not treated as redundant   */

      /* for (col = startcol; col < Q2->nredundcol; col++) */ /* print linearity space               */
      /*lrs_printoutput (Q2, Lin[col]); */                    /* Array Lin[][] holds the coeffs.     */

      /*********************************************************************************/
      /* Step 3: Terminate if lponly option set, otherwise initiate a reverse          */
      /*         search from the starting dictionary. Get output for each new dict.    */
      /*********************************************************************************/

      /* We initiate reverse search from this dictionary       */
      /* getting new dictionaries until the search is complete */
      /* User can access each output line from output which is */
      /* vertex/ray/facet from the lrs_mp_vector output         */
      /* prune is TRUE if tree should be pruned at current node */
      do
      {
        max_row_reward = DBL_MIN;
        prune = lrs_checkbound(P2, Q2);
        col = 0;
        if (!prune && lrs_getsolution(P2, Q2, output2, col))
        {
          if (Q2->verbose)
            prat_(" \np1's obj value: ", P2->objnum, P2->objden);

          long lrs_nashoutput1 = TRUE;
          long i1;
          long origin = TRUE;
          /* do not print the origin for either player */
          for (i1 = 1; i1 < Q2->n; i1++)
            if (!zero_(output2[i1]))
              origin = FALSE;

          if (origin)
            lrs_nashoutput1 = FALSE;

          if (lrs_nashoutput1)
          {
            // fprintf(lrs_ofp, "%ld ", 2L);
            for (i1 = 1; i1 < Q2->n; i1++)
              prat_("", output2[i1], output2[0]);
            reward = *output2[Q2->n - 1] / (double)*output2[0];
            if (reward > max_row_reward)
            {
              max_row_reward = reward;
              for (i1 = 0; i1 < Q2->n; i1++)
              {
                col_data_copy[i1] = *output2[i1];
              }
            }
            // fprintf(lrs_ofp, "\n");
            // fflush(lrs_ofp);
            numequilib++;
          }
        }
      } while (lrs_getnextbasis(&P2, Q2, prune));

    sayonara:
      lrs_free_dic(P2, Q2);

      if (numequilib > oldnum || Q1->verbose)
      {
        if (Q1->verbose)
          prat_(" \np2's obj value: ", P1->objnum, P1->objden);

        long lrs_nashoutput2 = TRUE;
        long i1;
        long origin = TRUE;
        /* do not print the origin for either player */
        for (i1 = 1; i1 < Q1->n; i1++)
          if (!zero_(output1[i1]))
            origin = FALSE;

        if (origin)
          lrs_nashoutput2 = FALSE;

        if (lrs_nashoutput2)
        {
          // fprintf(lrs_ofp, "%ld ", 1L);
          for (i1 = 1; i1 < Q1->n; i1++)
            prat_("", output1[i1], output1[0]);
          reward = (*output1[Q2->n - 1] / (double)*output1[0]) + max_row_reward;
          if (reward > max_reward)
          {
            max_reward = reward;
            for (i1 = 0; i1 < Q2->n; i1++)
            {
              row_data[i1] = *output1[i1];
            }
            memcpy(col_data, col_data_copy, Q2->n * sizeof(long long));
          }
          // fprintf(lrs_ofp, "\n");
          // fflush(lrs_ofp);
        }

        // fprintf(lrs_ofp, "\n");
      }
    }
  } while (lrs_getnextbasis(&P1, Q1, prune));

  // fprintf(lrs_ofp, "*Number of equilibria found: %ld", numequilib);
  // fprintf(lrs_ofp, "\n*Player 1: vertices=%ld bases=%ld pivots=%ld", Q1->count[1], Q1->count[2], Q1->count[3]);
  // fprintf(lrs_ofp, "\n*Player 2: vertices=%ld bases=%ld pivots=%ld", Q2->count[1], Q2->count[2], Q2->count[3]);

  lrs_clear_mp_vector(output1, Q1->m + Q1->n);
  lrs_clear_mp_vector(output2, Q1->m + Q1->n);

  lrs_free_dic(P1, Q1); /* deallocate lrs_dic */
  lrs_free_dat(Q1);     /* deallocate lrs_dat */

  /* 2015.10.10  new code to clear P2orig */
  Q2->Qhead = P2orig; /* reset this or you crash free_dic */
  P2orig->A = A2orig; /* reset this or you crash free_dic */

  lrs_free_dic(P2orig, Q2); /* deallocate lrs_dic */
  lrs_free_dat(Q2);         /* deallocate lrs_dat */

  free(linindex);

  //  lrs_close("nash:");
  // fprintf(lrs_ofp, "\n");
  return 0;
}

void init_game(game *g, int rows, int cols, int *row_num, int *row_den, int *col_num, int *col_den)
{
  lrs_init("*lrsnash:");
  g->nstrats[0] = rows;
  g->nstrats[1] = cols;
  int flat_idx = 0;
  for (int row_idx = 0; row_idx < rows; ++row_idx)
  {
    for (int col_idx = 0; col_idx < cols; ++col_idx)
    {
      g->payoff[row_idx][col_idx][0].num = row_num[flat_idx];
      g->payoff[row_idx][col_idx][0].den = row_den[flat_idx];
      g->payoff[row_idx][col_idx][1].num = col_num[flat_idx];
      g->payoff[row_idx][col_idx][1].den = col_den[flat_idx];
      ++flat_idx;
    }
  }
}

void _init_game(game *g, int rows, int cols)
{
  lrs_init("*lrsnash:");
  g->nstrats[0] = rows;
  g->nstrats[1] = cols;
}

void solve(game *g, long long *row_data, long long *col_data)
{
  lrs_solve_nash_(g, row_data, col_data);
}