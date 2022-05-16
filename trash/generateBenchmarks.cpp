
#include<unordered_map>
#include<unordered_set>
#include<fstream>

void generateLineFormulas(std::vector<std::string>& formulas, int level){
    if(level == 0){
        std::string formula = "A";
        formulas.push_back(formula);
    }else{
        generateLineFormulas(formulas, level--);
        std::string formula = "(if A " + formulas.back() + ")";
        formulas.push_back(formula);
    }
}

int main(){

}