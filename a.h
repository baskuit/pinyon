#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <gmpxx.h>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

/*
TODO:
- new solving. should also return the best response of the subgame actions (in the sense we might be able to modify lrsnash so that it provides this for us)
The above should help the performance of
- BEST RESPONSE ()
    - Which to check first if cant modify lrsnash? Subgame or out-of-subgame actions?
    - make sure we can use the payoff from solving as an improved bound (vs just using current alpha/beta)
    - can we save work when one players strategy does not change? E.g. row_best_response using old info when col_strategy is the same.
    - do we add solving of newly added chance nodes (via inclusion of new best response) in this function? (prolly)
*/

/*

Overview and explanation of data structures

BaseData
{
    * 'what position to search'
    * 'what depth to run to'
    * 'what random device, what model to inference'
}

Matrix/Chance Node
{
    The priority here is reducing the size of all of this data as much as possible.
    Anything that can be stored elsewhere will be

    [] = Matrix Node
    O = Chance Node

    []
    | |
    | O ______
    O _______
    |  \  \  \ (seed, quantized prob)
    |  \  \  \
    [] [] [] []

    MatrixNode
    {
        'c-style array to ChanceNodes'
        'last searches action and strategy info for iterative deepning'
    }

    ChanceNode
    {
        'std::unordered_map taking hashed transition obs and storing Branches'
        'vector of pointers to those same branches, ordered by transition probability (for faster value rediscovery)'
        'iterator int for that vector'
        'counter for the number of random transitions tried'
    }

    Branch
    {
        'std::unique_ptr to MatrixNode at the "end of the branch"'
        'random seed to keep State in sync with the tree'
        'quantized prob for branch ordering'
    }
}

TempData
{
    All things related to the algorithms run time

    |DEPTH: 0          |  -> |         | ->  |         |
    |

    'subgame optimistic/pessimistic value matrices'
}

HeadData
{
    What data can be modified and reverted as we traverse depth?
        Rather than storing different copies in TempData?

    'depth can be inc/decremented as needed'
    ''
}
*/

struct Obs {};

uint64_t hash(Obs) {
    return {};
}

struct State {
    Obs obs{};

    void randomize_transition(uint64_t seed) {}
    std::vector<uint8_t> row_actions{}, col_actions{};
    template <typename T>
    void apply_actions(T, T) {}
    bool is_terminal() { return {}; }
    void get_actions() {}
    mpq_class get_payoff() const { return {}; }
    mpq_class get_prob() const { return {}; }
    const Obs &get_obs() const { return obs; }
};

struct Device {
    uint64_t uniform_64() {
        return rand();
    }
};

struct ModelOutput {
    mpq_class value{};
    // std::vector<doulbe> policy{};
};

struct Model {
    ModelOutput inference(State &&state) const {
        return {};
    }
};