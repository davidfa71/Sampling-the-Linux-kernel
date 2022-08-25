// $Id: Tansform.cc 39 2008-08-03 10:07:15Z tb $
/** \file Tansform.cc Implementation of the kconf::Tansform class. */

#include <fstream>
#include <sstream>

#include "Transform.hpp"


Transform::Transform() {}


void    Transform::readVars(std::string varFile) {
    // We read the variables in order
    std::ifstream vf(varFile.c_str());
    if (!vf.good()) {
        //throw std::invalid_argument("Couldn't open file "+varFile);
        std::cerr << "Couldn't open file " << varFile << std::endl;
        exit(-1);
    }
    std::string temp;
    while (vf >> temp)
        variables.push_back(temp);
}

void    Transform::readExp(std::string expFile) {
    // We parse the expression file
    kconf::synExpDriver driver;
    driver.parse_file(expFile);
    expList  = driver.getConstraints();
    std::cerr << "Read " << expList.size() << " expressions" << std::endl;
    // We initialize the order and the inverse mapping
    for(int x = 0; x < variables.size(); x++) {
        var2pos.push_back(x);
        pos2var.push_back(x);
        varMap[variables.at(x)] = x;
    }
    
}

void Transform::checkErrors() {
    if (expList.size() == 0) return;
    for(int x = 0; x < var2pos.size(); x++) {
        if (var2pos[pos2var[x]] != x) {
            std::cerr << "Type 1 error x " << x
            << " pos2var[" << x << "] " << pos2var[x]
            << " var2pos[" << pos2var[x] << "] " << var2pos[pos2var[x]] << std::endl;
            exit(-1);
        }
        if (pos2var[var2pos[x]] != x) {
            std::cerr << "Type 2 error x " << x << std::endl;
            exit(-1);
        }
    }
    int min = std::numeric_limits<int>::max();
    int max = -1;
    for(int p : var2pos) {
        if (p < min) min = p;
        if (p > max) max = p;
    }
    if (min != 0 || max != variables.size()-1) {
        std::ostringstream ost;
        ost << "1 Error in treatVarSAndConstraints. min is " << min
                << " and max is " << max << std::endl;
        throw std::logic_error(ost.str());
    }
    
    min = std::numeric_limits<int>::max();
    max = -1;
    for(int p : var2pos) {
        if (p < min) min = p;
        if (p > max) max = p;
    }
    if (min != 0 || max != variables.size()-1) {
        std::cerr << "2 Error in treatVarSAndConstraints. min is " << min << " and max is " << max << std::endl;
        exit(-1);
    }
    
}
void Transform::minimalTreatment() {
    //std::cerr << "There are " << expList.size() << " expressions " << std::endl;
    if (simplify) {
        addSymbols2Table(variables);
        expList = simplifyConstraints(expList);
        std::cerr << "And now there are " << expList.size() << " expressions " << std::endl;
    }
    int counter = 0;
    for(const std::string& s : variables)
        varMap[s] = counter++;
      
    // We start with the identity order
    pos2var.resize(variables.size());
    var2pos.resize(variables.size());
    for(int x = 0; x < variables.size(); x++) {
        pos2var[x] = x;
        var2pos[x] = x;
    }
    
    // We create numeric indices for the constraints
    for(synExp* s: expList)
        if (s != synTrue)
            s->computeIndices(varMap);
}

void Transform::treatVarsAndConstraints() {
    minimalTreatment();
    
    if (!fast) {
        computeComponents();
        checkErrors();
        
        // variables is the inverse of varMap, it does no longer indicate ordering.
        // This vector DOES indicate variable ordering.
        var2pos.clear();
        var2pos.resize(variables.size());
        if (varOrderFile != "") {
            std::ifstream of(varOrderFile.c_str());
            if (!of.good()) {
                throw std::invalid_argument("Couldn't open file "+varOrderFile);
            }
            std::string temp;
            while (of >> temp)
                ordering.push_back(temp);
        }
        else
            ordering = variables;
        
        int c = 0;
        for(int var : pos2var) {
            var2pos[var] = c++;
        }
        
        for(synExp* s: expList) {
            if (s != synTrue) {
                s->computeIndices(varMap);
            }
        }
        if (doCNF) {
            std::ofstream dimacs(base+".dimacs");
            dimacs << "p cnf " <<  varMap.size() << " " << expList.size() << std::endl;
            for(synExp* s : expList) {
                for(int i : s->toDimacs(varMap))
                    // First variable has to be 1, not 0. And so on...
                    dimacs << i << " ";
                dimacs << "0" << std::endl;
            }
        }
        delete pcomp;
    }
}

void Transform::computeComponents() {
    if (static_components) {
        pcomp = new MultiComponents(var2pos, pos2var);
        for(synExp* s : expList) {
            pcomp->join(s, false);
        }
        
        pcomp->reorder();
        pcomp->checkLengths();
        int biggest = -1;
        for(std::pair<int, int> p : pcomp->listComponents())
            if (p.second > biggest)
                biggest = p.second;
        
        std::cerr << pcomp->getNumComponents() << " components. Biggest "
        << biggest << " number of variables " << pos2var.size() << std::endl;
        
        pos2var = pcomp->getOrder();
        var2pos.clear();
        var2pos.resize(variables.size());
        int c = 0;
        for(auto var: pos2var)
            var2pos[var] = c++;
        
    }
    else
        pcomp = new OneComponent(var2pos, pos2var);
}

constraintSorter* Transform::newCnstOrder(const std::string& s) {
    constraintSorter* sorter = NULL;
    if (s == "minimax")
        sorter = new minmaxSorter(adapter, expList);
    else
        if (s == "minspan")
            sorter = new minspanSorter(adapter, expList);
        else if (s == "remember")
            sorter = new RememberSorter(adapter, expList);
        else if (s == "smartspan")
            sorter = new smartspanSorter(adapter, expList);
        else if (s == "none")
            sorter = new nullSorter(adapter, expList);
        else if (s != "none") {
            std::cerr << "Unknown constraint sorter " << s << std::endl;
            exit(-1);
        }
    return sorter;
}

void Transform::buildBDD() {
    setUpAdapter(verbose);
    bddBuilder myBuilder(expList);
    myBuilder.setStopNodes(stopNodes);
    myBuilder.setUndo(doUndo);
    configureBuilder(myBuilder);
    myBuilder.build(inflation);
    std::string bddName;
    adapter->saveBDD(base, suffix);
    delete adapter;
};

void Transform::setFileNames() {
    if (suffix == "") {
        varfilename          = base + ".reorder";
        statsfile            = base + ".data";
        tempBDDName          = base + ".temp";
        appliedExpFilename   = base + ".applied";
        unappliedExpFilename = base + ".unapplied";
    }
    else {
        varfilename          = base + "-" + suffix + ".reorder";
        statsfile            = base + "_" + suffix + ".data";
        tempBDDName          = base + "-" + suffix + ".temp";
        appliedExpFilename   = base + "-" + suffix + ".applied";
        unappliedExpFilename = base + "-" + suffix + ".unapplied";
    }
}

void Transform::prepareContinueBuilding() {
    std::cerr << "Reading BDD... " << contBDD;
    adapter->readBDD(contBDD, "");
    std::cerr << " done. " << adapter->nodecount()
                << " nodes. " << std::endl;
    std::cerr << "numvars " << adapter->getNumVars() << std::endl;
    std::cerr << "var2pos size " << var2pos.size() << std::endl;
    for(std::string var : variables) {
        try {
            adapter->getVarIndex(var);
        }
        catch (std::out_of_range e) {
            adapter->newVar(var);
            std::cerr << "Adding variable " << std::setw(80) << var << "with index " << adapter->getVarIndex(var) << " to position " << adapter->getNumVars() - 1 << std::endl;
            int pos = adapter->getNumVars() - 1;
            int ind = adapter->getVarIndex(var);
            var2pos[ind] = pos;
            pos2var[pos] = ind;
            varMap[var] = ind;
        }
    }
    adapter->changeOrder(pos2var);
    // Recompute numeric indices for the constraints
    for(synExp* s: expList)
        if (s != synTrue)
            s->computeIndices(varMap);
}

void Transform::setUpAdapter(bool xverbose) {
    verbose = xverbose;
    setFileNames();
    adapter = new cuddAdapter(cacheMultiplier);
    if (dynamic_components)
        adapter->useComponents(var2pos, pos2var);
    else
        adapter->changeOrder(pos2var);

    if (contBDD != "") {
        prepareContinueBuilding();
    }
    else {
        for(std::string var : variables)
            adapter->newVar(var);
        if (blockFile != "")
            adapter->setBlocks(blockFile);
        // Now apply the ordering that was computed before...
        adapter->shuffle(ordering);
    }
        
    adapter->checkOrder();
    synExp::numVars(variables.size());
}

void Transform::configureBuilder(bddBuilder& b) {
    if (!verbose)
        b.setVerboseOff();
    
    b.setAdapter(adapter);
    b.setVarFileName(varfilename);
    b.setStatsFileName(statsfile);

    constraintSorter* cs = newCnstOrder(constReorder);
    b.setConstraintSorter(cs);
    
    if (reorder)
        b.setReorderMethod(reorderMethod);
    else
        b.setReorderMethod("noreorder");
    
    b.setReorderInterval(reorderInterval);
    b.setMinNodes(minNodes);
    b.setLineLength(lineLength);
    b.setMaxNodes(maxNodes);
    b.setMinConstraint(minConstraint);
    b.setReorderEvery(reorderEvery);
    b.setTempBDDName(tempBDDName);
    b.setAppliedExpFilename(appliedExpFilename);
    b.setUnappliedExpFileName(unappliedExpFilename);
}
void   Transform::addToForest(std::vector<synExp*>& exps,
                              std::string& varname,
                              int c) {
    static_components  = false;
    simplify           = false;
    dynamic_components = true;
    minNodes           = 100000;
    fast               = true;
    std::cerr   << "Build tree for variable " << std::setw(60) << std::left << varname
                << " (" << c << ") with " << std::setw(5) << exps.size() << " expressions "
                << std::endl;
    bddBuilder builder(exps);
    builder.setStopNodes(stopNodes);
    builder.setAdapter(adapter);
    builder.setReorderMethod("noreorder");
    builder.setUndo(doUndo);
    builder.setFunctionName(varname);
    configureBuilder(builder);
    builder.build(inflation);
}

void Transform::buildManyBDDs() {
    std::string varname;
    std::vector<synExp*> vec;
    treatVarsAndConstraints();
    setUpAdapter(verbose);
    bool firstTime = true;
    int c = 0;
    reorder = false;
    for(auto e : expList) {
        if (e->isComment()) {
            if (firstTime) {
                varname = e->getComment();
            }
            else {
                addToForest(vec, varname, c++);
                vec.clear();
                varname = e->getComment();
            }
            firstTime = false;
        }
        vec.push_back(e);
    }
    addToForest(vec, varname, c++);
    adapter->saveBDDArray(base, suffix);
    std::set<std::pair<int, std::string>> ts;
    for(auto& head : adapter->getStringHeads())
        ts.insert(std::pair<int, std::string>(adapter->nodecount(head),
                                            head));
    for(auto& p : ts) {
            std::cerr   << std::setw(50) << std::left << p.second
                        << " " << std::setw(10) << p.first
                        << " nodes " << std::endl;
    }
    for(int i = 0;  i < adapter->getFunctionHeads().size(); i++)
        if (adapter->getFunctionHeads()[i] == adapter->getZero())
            std::cerr << "Head " << adapter->getStringHeads()[i] << " is false" << std::endl;

}
