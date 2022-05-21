#include <math.h>
#include <iostream>
#include "tree.cc"
#include "toy_states.cc"

// 'learning rate' used in exp3
float eta = 0.01f;

float randomProb () {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

int select (float* forecasts, int k) {
    float p = randomProb();
    for (int i = 0; i < k; ++i) {
        p -= forecasts[i];
        if (p <= 0) {
            return i;
        }
    }
    return 0;
}

// pass an array called forecasts to be filled with a selection prob. distro per exp3
void forecast(float* forecasts, float* gains, int k) {
    float max = 0;
    for (int i = 0; i < k; ++i) {
        float x = gains[i];
        if (x > max) {
            max = x;
        } 
    }
    float sum = 0;
    for (int i = 0; i < k; ++i) {
        gains[i] -= max;
        float x = gains[i];
        float y = std::exp(x * eta / k);
        forecasts[i] = y;
        sum += y;
    }
    for (int i = 0; i < k; ++i) {
        forecasts[i] *= (1-eta)/sum;
        forecasts[i] += eta/k;
    }
}

MatrixNode* Search (MatrixNode* Node, State* S) {

    if (Node->terminal == true) {
        return Node;
    } else {
        if (Node->expanded == true) {

            float forecasts0[Node->m];
            float forecasts1[Node->n];
            forecast(forecasts0, Node->gains0, Node->m);
            forecast(forecasts1, Node->gains1, Node->n);
            int i = select(forecasts0, Node->m);
            int j = select(forecasts1, Node->n);

            int key = S->transition(i, j);

            ChanceNode* Chance = Node->access(i, j);

            // The matrix node just after the one in the argument
            MatrixNode* Node_ = Chance->access(key, Rational(1, 1));

            // The last matrix node reached in the recursive chain.
            MatrixNode* Node__ = Search(Node_, S);

            float u0 = Node__->v0;
            float u1 = Node__->v1;

            // Causes an update of the selection probs for next round
            Node->gains0[i] += u0 / forecasts0[i];
            Node->gains1[j] += u1 / forecasts1[j];
            // Empirical Strategy, one of the outputs
            Node->visits0[i] += 1;
            Node->visits1[j] += 1;

            // Stats, currently unused
            Node->q0 += u0;
            Node->q1 += u1;
            Node->visits += 1;
            Chance->q0 += u0;
            Chance->q1 += u1; 
            Chance->visits += 1;

            return Node__;  

        } else {
            Node->expand(S);
            return Node;
        }
    }
}

void normalizeEmprical (float* strategy, int* arr, int m) {
    float sum = 0;
    for (int i = 0; i < m; ++i) {
        sum += arr[i];
    }
    for (int i = 0; i < m; ++i) {
        strategy[i] = arr[i]/sum;
    }
}




int main () {

    MatrixNode* root = new MatrixNode();
    int pp = 3;
    int length = 2;
    float payoff = (pp-1)/(float)pp; // root payoff not calculated automatically by transition
    State* S = new State('c', false, payoff, pp, length); //sucker punch game
    int playouts = 1000000;

    for (int playout = 0; playout < playouts; ++playout) {
        State* S_ = S->copy();
        Search(root, S_);
        delete S_; //memory management pog
    }

    //print emprical strategies and values
    float strategy0[S->m];
    float strategy1[S->n];
    normalizeEmprical(strategy0, root->visits0, S->m);
    normalizeEmprical(strategy1, root->visits1, S->n);
    for (int i = 0; i < root->m; ++i) {
        std::cout << strategy0[i] << ", ";
    }
    std::cout << std::endl;
    for (int j = 0; j < root->n; ++j) {
        std::cout << strategy1[j] << ", ";
    }
    std::cout << std::endl;
    std::cout << "Payoffs: " << root->q0/root->visits << ", " << root->q1/root->visits << std::endl;
    std::cout << "Actual player0 payoffs: " << S->payoff;

}