#include <iostream>
#include <vector>
#include <fstream>
#include "synExpDriver.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input exp file> <prefix output>" << std::endl;
        exit(-1);
    }
    
    std::string expFile = argv[1];
    std::string prefix  = argv[2];
    
    std::ifstream ef(expFile);
    // We parse the expression file
    kconf::synExpDriver driver;
    driver.parse_file(expFile);
    std::vector<synExp*> expList  = driver.getConstraints();
    String2Int si;
    std::map<int, std::string> is;
    int c        = 1;
    int clauses  = 0;
    int varCount = 0;
    int numvar   = 1;
    std::ostringstream cnfOut;
    std::ofstream outExp(prefix+".exp");
    std::ofstream cnf(prefix+".cnf");
    for(auto e : expList) {
        std::cerr   << "Tseitin " << c++ << " of " << expList.size()
                    << ": " << e << std::endl;
        if (e->isComment())
            if (si.find(e->getComment()) == si.end()) {
                si[e->getComment()] = varCount;
                is[varCount++] = e->getComment();
            }
                
        for(auto x : e->Tseitin(numvar)) {
            clauses++;
            outExp    << x << std::endl;
            for(auto s : x->giveSymbols()) {
                if (si.find(s) == si.end()) {
                    si[s] = varCount;
                    is[varCount++] = s;
                }
            }
            for(auto z : x->toDimacs(si))
                cnfOut << z << " ";
            cnfOut << "0";
            cnfOut << std::endl;
        }
    }
    //        std::cerr << "si " << std::endl;
    //        for(auto p : si)
    //            std::cerr << p.first << " " << p.second << std::endl;
    //        std::cerr << std::endl;
    outExp.close();
    cnf << "p cnf " << std::setw(6) << varCount << " " << std::setw(10)
                    << clauses << std::endl;
    for(auto p: is)
        cnf << "c " << p.first+1 << " " << p.second << std::endl;
    cnf << cnfOut.str();
    cnf.close();
}

