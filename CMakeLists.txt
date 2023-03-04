cmake_minimum_required(VERSION 3.0.0)
project(surskit VERSION 0.1.0)


set(GAMBIT_SOURCES
    gambit/src/core/dvector.cc
    gambit/src/core/function.cc
    gambit/src/core/integer.cc
    gambit/src/core/matrix.cc
    gambit/src/core/pvector.cc
    gambit/src/core/rational.cc
    gambit/src/core/sqmatrix.cc
    gambit/src/core/tinyxml.cc
    gambit/src/core/tinyxmlerror.cc
    gambit/src/core/tinyxmlparser.cc
    gambit/src/core/vector.cc

    gambit/src/games/behav.cc
    gambit/src/games/behavitr.cc
    gambit/src/games/behavspt.cc
    gambit/src/games/file.cc
    gambit/src/games/game.cc
    gambit/src/games/gameagg.cc
    gambit/src/games/gamebagg.cc
    gambit/src/games/gametable.cc
    gambit/src/games/gametree.cc
    gambit/src/games/mixed.cc
    gambit/src/games/nash.cc
    gambit/src/games/stratitr.cc
    gambit/src/games/stratspt.cc
    gambit/src/games/writer.cc

    gambit/src/games/agg/agg.cc
    gambit/src/games/agg/bagg.cc

    gambit/src/solvers/linalg/basis.cc
    gambit/src/solvers/linalg/btableau.cc
    gambit/src/solvers/linalg/lemketab.cc
    gambit/src/solvers/linalg/lhtab.cc
    gambit/src/solvers/linalg/lpsolve.cc
    gambit/src/solvers/linalg/lptab.cc
    gambit/src/solvers/linalg/ludecomp.cc
    gambit/src/solvers/linalg/tableau.cc

    gambit/src/solvers/enummixed/clique.cc
    gambit/src/solvers/enummixed/lrsenum.cc
    gambit/src/solvers/enummixed/enummixed.cc

    gambit/src/solvers/lrs/lrslib.c
    gambit/src/solvers/lrs/lrsmp.c
    gambit/src/solvers/lrs/lrsnashlib.c
)

INCLUDE_DIRECTORIES(
    src
    random_trees
	gambit/src
	gambit/src/core
	gambit/src/games
	gambit/src/games/agg
	gambit/src/solvers
	gambit/src/lrs
)

add_library(gambit STATIC ${GAMBIT_SOURCES})

add_executable(rewrite src/main.cc)
add_executable(random_trees random_trees/random_trees.cc)

target_link_libraries(rewrite gambit)
set_property(TARGET rewrite PROPERTY CXX_STANDARD 20)  
set_property(TARGET random_trees PROPERTY CXX_STANDARD 20)  

target_link_libraries(random_trees gambit)