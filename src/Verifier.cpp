

#include<string>
#include"Proof.hpp"

int main(int argc, char** argv){
    std::string filePath(argv[1]);
    HyperslateFileData f = parseHyperslateFile(filePath);
    Proof p(f);
    return 0;
}