
cmake -B ../build/ cmake --graphviz=../build/depGraph.dot ..
dot -Tpng -o depGraph.png ../build/depGraph.dot