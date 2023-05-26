#include "lrsdriver.h"
#include "lrslib.h"
#include "lrsnashlib.h"

#ifndef LIBLRSNASH
#define LIBLRSNASH

#ifdef __cplusplus
extern "C"
{
#endif

    extern void init_game(game *g, int rows, int cols, int *row_num, int *row_den, int *col_num, int *col_den);
    /*
    Initializes the game using rational values. The arrays are flattened by row first.
    */

    extern void _init_game(game *g, int rows, int cols);

    extern void solve(game *g, long long *row_data, long long *col_data);
    /*
    row_data, col_data must have size rows + 2, cols + 2, respectively.
    Index 0 is the denominator, the next `row` entries are the numerator of the nash equilibrium strategy. The last index is the numberator of the payoff of the other player.
    Since only one strategy is returned, we find all valid joint strategies and return the one with the total expected payoff (for both players.)
    The hope is that this strategy is most suited for MatrixUCB (For a zero sum matrix, recall that all strategies have the same payoffs for each player.)
    */

#ifdef __cplusplus
}
#endif

#endif // LIBLRSNASH