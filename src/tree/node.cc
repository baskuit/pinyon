#include "node.hh"

// Return MatrixNode child from chance node, with given Transition Key
// If child is not found, it is created
MatrixNode* ChanceNode :: access (StateTransitionData data) {
        if (this->child == nullptr) {
            MatrixNode* child = new MatrixNode(this, data);
            this->child = child;
            return child;
        }
        MatrixNode* current = this->child; 
        MatrixNode* previous = this->child; 
        while (current != nullptr) {
            previous = current;
            if (current->transitionKey == data.transitionKey) {
                return current;
            }
            current = current->next;
        }
        MatrixNode* child = new MatrixNode(this, data);
        previous->next = child;
        return child;
}

ChanceNode* MatrixNode :: access (Action action0, Action action1) {
        if (this->child == nullptr) {
            ChanceNode* child = new ChanceNode(this, action0, action1);
            this->child = child;
            return child;
        }
        ChanceNode* current = this->child; 
        ChanceNode* previous = this->child;
        while (current != nullptr) {

            previous = current;
            if (current->action0 == action0 && current->action1 == action1) {    
                return current;
            }
            current = current->next;
        }
        ChanceNode* child = new ChanceNode(this, action0, action1);
        previous->next = child;
        return child;
}

void MatrixNode :: expand (State* state, Model* model) {

    this->expanded = true;
    PairActions* pair = state->actions();
    this->rows = pair->rows;
    this->cols = pair->cols;
    
    this->actions0 = new Action[this->rows];
    for (int row_idx = 0; row_idx < pair->rows; ++row_idx) {
        this->actions0[row_idx] = pair->actions0[row_idx];
    }
    this->actions1 = new Action[this->cols];
    for (int col_idx = 0; col_idx < pair->cols; ++col_idx) {
        this->actions1[col_idx] = pair->actions1[col_idx];
    }
    this->terminal = (rows*cols == 0);
    if (this->terminal) {
        return;
    }
    InferenceData data = model->inference(state);
    this->value_estimate0 = data.value_estimate0;
    this->value_estimate1 = data.value_estimate1;
    //this->strategy_prior0 not used in exp3 :/

    this->actions0 = pair->actions0;
    this->actions1 = pair->actions1;
    /*
    this->gains0 = new float[this->rows]{0.f};
    this->gains1 = new float[this->cols]{0.f};
    this->visits0 = new int[this->rows]{0};
    this->visits1 = new int[this->cols]{0};
    */
} 

MatrixNode :: ~MatrixNode() {
    if (child != nullptr) {
        delete child;
    }
    if (next != nullptr) {
        delete next;
    }
}

ChanceNode :: ~ChanceNode () {
    if (child != nullptr) {
        delete child;
    }
    if (next != nullptr) {
        delete next;
    }
};

// void MatrixNode :: print (int n = 0) {
//     if (this == nullptr) {
//         return;
//     }
//     std::cout << n << ": " << this << std::endl;
//     if (n == 2){
//         return;
//     }
//     if (this->child != nullptr) {
//         this->child->print(n+1);
//     }
// }

// void ChanceNode :: print (int n = 0) {
//     if (this == nullptr) {
//         return;
//     }
//     std::cout << n << ": " << this << std::endl;
//     if (this->child != nullptr) {
//         this->child->print(n+1);
//     }
// }