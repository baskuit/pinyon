
#pragma once

#include "../libsurskit/math.hh"
#include "session.hh"


struct Exp3SearchSessionData;

class Exp3SearchSession : public SearchSession {
public:

    float eta = .01f;

    Exp3SearchSession ();
    Exp3SearchSession (prng* device, State* state, float eta) : 
    SearchSession(device, new MatrixNode(), state, new MonteCarlo(device)), eta(eta) {};
    // Exp3SearchSession (MatrixNode* root, State* state, Model* model, float eta) :
    // SearchSession(root, state, model), eta(eta) {};

    Exp3SearchSessionData answer ();

    MatrixNode* search (MatrixNode* matrix_node_current, State* state);
    void search (int playouts);
    void expand (PairActions& pair, InferenceData& inference, MatrixNode* matrix_node, State* state, Model* model);

    void forecast(float* forecasts, float* gains, int k);
    
};

// struct Exp3SearchSessionData : SearchSessionData {

//     int rows = 0;
//     int cols = 0;
//     int playouts_ = 0;
//     float cumulative_score0_ = 0.f;
//     float cumulative_score1_ = 0.f;
//     std::vector<int> visits0_;
//     std::vector<int> visits1_;
//     std::vector<float> strategy0; // the real result
//     std::vector<float> strategy1;
//     Exp3SearchSessionData (const PairActions& pair) : 
//     rows(pair.rows), cols(pair.cols) {
//         visits0_.resize(rows, 0);
//         visits1_.resize(cols, 0);
//         strategy0.resize(rows, 0.f);
//         strategy1.resize(cols, 0.f);
//     }
//     // Exp3SearchSessionData (int rows, int cols) : 
//     // rows(rows), cols(cols), 
//     // visits0(new int[rows] {0}), visits1(new int[cols] {0}), 
//     // strategy0(new float[rows] {0.f}), strategy1(new float[cols] {0.f})  {};

//     // ~Exp3SearchSessionData () {
//     //     delete [] visits0;
//     //     delete [] visits1;
//     //     delete [] strategy0;
//     //     delete [] strategy1;
//     // }

//     // 'Add' the results of two searches. New solved strategies are the weighted sums.
//     Exp3SearchSessionData operator+ (Exp3SearchSessionData& S) {
//         const int rows_min = rows < S.rows ? rows : S.rows;
//         const int cols_min = cols < S.cols ? cols : S.cols;
//         Exp3SearchSessionData result(rows_min, cols_min);
//         result.playouts = playouts + S.playouts;
//         const float portionA =   playouts / (float) result.playouts;
//         const float portionB = S.playouts / (float) result.playouts;
//         for (int row_idx = 0; row_idx < rows_min; ++row_idx) {
//             result.visits0[row_idx] = visits0[row_idx] + S.visits0[row_idx];
//             result.strategy0[row_idx] = portionA * strategy0[row_idx] + portionB * S.strategy0[row_idx];
//         }
//         for (int col_idx = 0; col_idx < cols_min; ++col_idx) {
//             result.visits1[col_idx] = visits1[col_idx] + S.visits1[col_idx];
//             result.strategy1[col_idx] = portionA * strategy1[col_idx] + portionB * S.strategy1[col_idx];
//         }
//         result.cumulative_score0 = cumulative_score0 + S.cumulative_score0;
//         result.cumulative_score1 = cumulative_score1 + S.cumulative_score1;
//         return result;
//     }
// };

// in the nodes