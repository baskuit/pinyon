typedef struct lrs_restart_dat   /* for restarting from a given cobasis         */
{
        long *facet;            /* cobasic indices for restart                  */

        long overide;           /* TRUE if Q parameters should be updated       */
        long restart;           /* TRUE if we supply restart cobasis            */
        long lrs;               /* TRUE if we are doing a lrs run               */
        long m;                 /* number of input rows                         */
        long d;                 /* number of cobasic indices                    */
        long count[10];         /* count[0]=rays(facets) [1]=verts. [2]=base [3]=pivots */
                                /* count[4]=integer vertices [5]=1 for hull     */
                                /* [6]=nlinearities [7]=deepest                 */
        long depth;             /* depth of restart node                        */
        long maxcobases;        /* if positive, after maxcobasis unexplored subtrees reported */
        long long maxdepth;     /* max depth to search to in treee              */
        long long mindepth;     /* do not backtrack above mindepth              */
        long printcobasis;      /* TRUE if option set in lrs, mplrs renumbers   */
        long redund;            /* TRUE if we are doing a redund run            */
        long fel;               /* TRUE if we are doing a fel run               */
  
        long verifyredund;      /* a worker checks redundancy and gives output  */
        long messages;          /* TRUE if lrs should post_output messages      */
        long *redineq;          /* a list of row numbers to check redundancy    */

	long size;		/* number of processes in mplrs MPI run */
	long rank;		/* my ID. 
				 * 0: master, 1: consumer, 2...size-1: workers*/
} lrs_restart_dat;

lrs_restart_dat* lrs_alloc_restart();

/**********************************************************************************
      Usage of lrs_restart_dat for communication between lrs and mplrs


lrs mode:   


-----------------------------------------------------------------------------------
redund mode:


-----------------------------------------------------------------------------------
fel mode: (fel=TRUE redund=lrs=FALSE verifyredund=FALSE)

rank=0  mplrs expects lrs to return m where m+1 is the dimension of redineq
        after one variable has been eliminated.
        Currently lrs allocs redineq and initializes redineq[i]=1 (is this necessary?)

rank=2,...,np-2  mplrs expects eacher worker to return a redineq array with same dimension m
        with redineq[i]=2 linearity, =1 possibly redundant, =-1 surely redundant, =0 otherwise
        mplrs copies the non-zero values into its own redineq array. After all workers finish
        mplrs chooses the worker with the most "1" values and converts them to "-1"

rank=1  after all workers terminated mplrs supplies its redineq array and sets verifyredund=TRUE. 
        lrs does a redund_run and produces ouput

Looks like we do not need verifyredund since it is equal to (rank==1)  ?

**********************************************************************************/
