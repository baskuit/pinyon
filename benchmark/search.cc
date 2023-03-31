#include "state/test_states.hh"
#include "model/model.hh"
#include "algorithm/multithreaded.hh"
#include "algorithm/exp3p.hh"
#include "algorithm/matrix_ucb.hh"
#include "algorithm/matrix_pucb.hh"

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

using namespace std::chrono;

const int size = 2;


void foo1() {
using MoldState = MoldStateVector<size>;
using Model = MonteCarloModel<MoldState>;
using Exp3p = Exp3p<Model, TreeBandit>;
using MatrixUCB = MatrixPUCB<Model, TreeBandit>;

    const int depth = 20;
    MoldState game(depth);
    prng device(0);
    Model model(device);
    MatrixNode<MatrixUCB> root;
    MatrixUCB session;
    const int playouts = 100000;

    session.run(playouts, device, game, model, root);

}

void foo2() {
using MoldState = MoldStateVector<size>;
using Model = MonteCarloModel<MoldState>;
using Exp3p = Exp3p<Model, TreeBandit>;
using MatrixUCB = MatrixPUCB<Model, TreeBandit>;

    const int depth = 20;
    MoldState game(depth);
    prng device(0);
    Model model(device);
    MatrixNode<MatrixUCB> root;
    MatrixUCB session;
    const int playouts = 100000;

    session.run(playouts, device, game, model, root);

}

int main() {
    std::map<std::string, long long> execution_times;

    // measure the execution time of foo1
    auto start = high_resolution_clock::now();
    foo1();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    execution_times["foo1"] = duration.count();

    // measure the execution time of foo2
    start = high_resolution_clock::now();
    foo2();
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    execution_times["foo2"] = duration.count();

    // save the execution times to a file
    std::ofstream file("execution_times.txt", std::ios::app);
    auto now = system_clock::now();
    auto now_time = system_clock::to_time_t(now);
    file << "Test date/time: " << std::put_time(std::localtime(&now_time), "%F %T") << "\n";

    for (const auto& entry : execution_times) {
        file << entry.first << " " << entry.second << "\n";
    }

    file.close();

    return 0;
}
