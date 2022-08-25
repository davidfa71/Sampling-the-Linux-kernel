//
//  FastSifter.cpp
//  fastOrder
//
//  Created by David Fernandez Amoros on 07/10/2019.
//  Copyright © 2019 David Fernandez Amoros. All rights reserved.
//

#include "FastSifter.hpp"

// Basic constructor
FastSifter::FastSifter(bool        _verbose,
                       std::string varFile,
                       std::string expFile,
                       std::string _outputFile,
                       std::string _histogram,
                       std::string _blocks,
                       double     _threshold,
                       bool       _nosimplify,
                       bool       _noGrouping,
                       bool       _noSubExp,
                       int        _window) :
            outputFile(_outputFile),
            verbose(_verbose),
            histogram(_histogram),
            blocks(_blocks),
            window(_window),
            threshold(_threshold),
            nosimplify(_nosimplify),
            noGrouping(_noGrouping),
            noSubExp(_noSubExp) {
    numSwaps = 0;
    readInfo(varFile, expFile);
    addSymbols2Table(variables);
    init();
}

FastSifter::FastSifter(std::vector<std::string>&        _variables,
                  std::vector<synExp*>&                 _expList,
                  double                                _threshold,
                  bool                                  _nosimplify,
                  bool                                  _noGrouping,
                  bool                                  _noSubExp,
                  int                                   _window) :  variables(_variables),
                      expList(_expList),
                      threshold(_threshold),
                      nosimplify(_nosimplify),
                      noGrouping(_noGrouping),
                      noSubExp(_noSubExp),
                      window(_window) {
   numSwaps = 0;
   init();
}

void FastSifter::prepVarOrdering() {
    index2pos.clear();
    pos2index.clear();
    for(int x = 0; x < variables.size(); x++) {
        index2pos.push_back(x);
        pos2index.push_back(x);
    }
    computeComponents();
    computeIntersections();
    initSwap();
}

void FastSifter::goPerm() {
    prepVarOrdering();
    performPerm();
}

void FastSifter::performPerm() {
    generateSequence(window);
    score_t origScore = computeAllScore();
    std::cerr << "Original score " <<   std::fixed << std::showbase << std::right
               << origScore << std::endl;
    double improvement;
    std::cerr.precision(15);

    for(auto& p : pcomp->listComponents()) {
        if (p.second > 1) {
            int start  = p.first;
            int length = p.second;
            std::cerr << std::endl;
            std::cerr << "Component with start " << start << " and length " << length << std::endl;
            // We build all the constraints involved in this component
            for(int pos = start; pos < start+length; pos++)
                for(auto exp : expressionsVar[pos2index[pos]])
                    compVectors.insert(exp);
            
            score = computeScore(compVectors);
            std::cerr << "Order Score:        " << std::fixed << std::showbase << std::right
            << score << std::endl;
            do {
                score_t sc = score;
                sliding_window(p.first, p.second);
                improvement = 100.0*(sc - score)/sc;
                if (improvement < 0.0000000001)
                    improvement = 0;
                std::cerr   << "Intermediate score: "
                << std::setw(10) << std::right << std::showbase << std::fixed << score
                << " improvement "        << std::right << std::setw(10)
                << std::showbase << std::fixed
                << improvement << "%" << std::endl;
            } while (improvement > threshold && improvement > 0.00001);
        }
    }
    score = computeAllScore();
    std::cerr << std::endl <<  "Final score:        " << std::setw(10) << std::showbase << score
    << " reduction   "        << std::right << std::setw(10)
    << 100.0*(origScore - score)/origScore << "%" << std::endl;
    std::cerr << "Number of swaps " <<  numSwaps << std::endl;
}

void FastSifter::sliding_window(int start, int length) {
    int pos = start;
    bool firstTime = true;
    for(;;) {
        if (pos+window <= start+length)
            members = window;
        else
            members = start+length - pos;
        // We always do it the first time regardless
        // unless there is only one of course
        if (members == 1 || (!firstTime && (members != window)))
            break;
        int leftDisplacement = applyBestPerm(pos);
        firstTime = false;
        if (leftDisplacement == 0) {
            pos++;
            if (verbose) std::cerr << pos << " ";
        }
        else {
            if (pos >= start+leftDisplacement) {
                if (verbose) std::cerr << "-" << leftDisplacement << " ";
                    pos = pos-leftDisplacement;
            }
            else {
                if (pos != start) {
                     if (verbose)  std::cerr << "-" << pos - start << " ";
                    pos = start;
                }
                else {
                    pos++;
                    if (verbose) std::cerr << pos << " ";
                }
            }
        }
    }
}

void FastSifter::goBackToBest(int bestPermNum, int startPos) {
    //std::cerr << "members " << members << std::endl;
    for(auto c : pathBack[members-2][bestPermNum]) {
        swapPos(startPos+c);
    }
}
// After calling applyPerms, we get the number of the best permutation.
// Applying a swapPos(0) gives back the original permutation (amazing).
// so we can reapply the permutations until we get to the optimum.
//
// This is great to easily get back the appropriate index2pos, pos2index,
// minVector, maxVector, vSpan and score
//
// You can also get there faster by looking if the best permutation is past
// the middle, by going backwards (e.g. reapplying the permutation in reverse
// order to get back to the best one)

// Returns bestPerm
int FastSifter::applyBestPerm(int pos) {
    score_t bestScore = score;
    int bestPermNum = applyPerms(pos, bestScore, bestPermNum);
    goBackToBest(bestPermNum, pos);
    if (abs(score - bestScore) > 0.0000001) {
        std::ostringstream ost;
        ost << "score is " << score << " and bestScore is " << bestScore
        << " they should be the same, Dif is " << (score -bestScore)
            << " pos is " << pos << std::endl;
        throw std::logic_error(ost.str());
    }
    return leftMove[members-2][bestPermNum];
}
// This function returns the permutation number in the sequence
// which provided the best score
int FastSifter::applyPerms(int pos, score_t& bestScore, int& bestPermNum) {
    int permsApplied = 0;
    score_t     initialScore = score;
    bestPermNum = 0;
    int oldDisplacement = 0, newDisplacement;
    // Permutations start being interesting at size 2
    for(int c : permSeq[members-2]) {
        swapPos(pos+c);
        permsApplied++;
        if (bestScore >= score) {
            // If there is a draw we keep the permutation that goes back the most
            // in the sliding window to ensure we get the optimum in one pass
            newDisplacement = leftMove[members-2][permsApplied];
            if (score < initialScore &&
                (bestScore > score || oldDisplacement < newDisplacement)) {
                bestScore       = score;
                bestPermNum     = permsApplied;
                oldDisplacement = newDisplacement;
            }
        }
    }
    //std::cerr << "Best permutation is "; printVector(bestPerm); std::cerr << std::endl;
    //std::cerr << "Current permutation is "; printVector(permutation); std::cerr << std::endl;
    return bestPermNum;
}
void FastSifter::goSifting() {
    prepVarOrdering();
    performSifting();
    if (!noGrouping) {
    // Now reorder each group in the rewriteRules
    std::vector<std::string> newVars;
    std::vector<std::pair<int, int>> startLength;
    for(auto index : pos2index) {
        for(auto& s : rewriteRules[variables.at(index)]) {
            if (std::find(newVars.begin(), newVars.end(), s) != newVars.end()) {
                std::cerr   << "Repeating var " << s << " from rewriteRules of "
                            << variables.at(index) << std::endl;
                for(auto x : pos2index) {
                    for(auto& z : rewriteRules[variables.at(x)])
                        if (z == s)
                            std::cerr << "Present in rewriterules of " << x << " "
                                <<  variables.at(x) << std::endl;
                }
                std::cerr << "pos2index:";
                for(auto q : pos2index) std::cerr << q << " (" << variables[q] << ") ";
                std::cerr << std::endl;
                    
                throw std::logic_error("");
            }
            newVars.push_back(s);
        }
    }
    variables = newVars;
    expList = origExpList;

    std::vector<synExp*> tempList = expList;
    std::cerr << "Adding expressions...";
    if (!noSubExp) {
        for(auto e : tempList)
            if (e->get_type() == synExp_Compound) {
                addSubExpressions(e->first());
                if (e->second() != NULL) {
                    addSubExpressions(e->second());
                }
                if (e->third() != NULL) {
                    addSubExpressions(e->third());
                }
            }
    }
    fillReverseIndex();
    init();
    prepVarOrdering();
    std::cerr << std::endl;
    for(auto&p : rewriteRules)
        if (p.second.size() > 1) {
            int start = index2pos[varMap[p.first]];
            std::cerr   << "Sifting group with start " << start
            << " and length " << p.second.size() << std::endl;
            //for(int i = start; i < start+p.second.size(); i++)
            //    std::cerr << variables[i] << " ";
            std::cerr << std::endl;
            siftComponent(start, p.second.size());
            std::vector<std::string> newOrder;
            for(int i = 0; i < p.second.size();i++)
                newOrder.push_back(variables[pos2index[start+i]]);
            rewriteRules[p.first] = newOrder;
        }
    }
}
void FastSifter::writeResults() {
    if (blocks != "") {
        std::ofstream groups(blocks);
        for(auto index : pos2index) {
            if (rewriteRules[variables.at(index)].size() > 1) {
                for(auto& s : rewriteRules[variables.at(index)])
                    groups << s << " ";
                
                groups << std::endl;

            }
        }
        groups.close();
    }
    std::ofstream ovar(outputFile);
    for(auto index : pos2index) {
        for(auto& s : rewriteRules[variables.at(index)]) {
            ovar << s << " ";
        }
        rewriteRules.erase(variables.at(index));
    }
    for(auto& p : rewriteRules)
        for(auto& s : p.second)
            ovar << " " << s;
    
    ovar << std::endl;
    ovar.close();
    if (histogram != "") {
        std::ofstream hist(histogram);
        // Now a distribution of the spans
        std::map<int, int> dist;
        for(int i = 0; i < expressions.size(); i++)
            dist[computeSpan(i)]++;
        
        for(auto p : dist)
            hist << std::setw(6) << p.first << " "
                 << std::setw(6) << p.second << std::endl;
    }
}

std::vector<std::string> FastSifter::giveResults() {
    std::vector<std::string> res;
    for(auto index : pos2index) {
        for(auto& s : rewriteRules[variables.at(index)])
            res.push_back(s);
        rewriteRules.erase(variables.at(index));
    }
    for(auto& p : rewriteRules)
        for(auto& s : p.second)
            res.push_back(s);

    return res;
}

void FastSifter::performSifting() {
    double myStart = get_cpu_time();
    score_t initial = computeAllScore();

    std::cerr << "Sifting components" << std::endl;
    for(auto& p : pcomp->listComponents())
        if (p.second > 1)
            siftComponent(p.first, p.second);
    
    score_t finalScore = computeAllScore();
    if (initial != 0)
        std::cerr << "Initial      score " << std::setw(15) << initial << "  final score "
                << finalScore << " improvement " << 100.0*(initial - finalScore)/initial
            << "% " << showtime(get_cpu_time() - myStart) << std::endl;
    else
        std::cerr << "Initial      score " << std::setw(15) << initial << "  final score " << finalScore << " improvement " << "not applicable" << std::endl;
    std::cerr << "Number of swaps " <<  numSwaps << std::endl;
}

void FastSifter::addSubExpressions(synExp* e) {
    if (e->get_type() == synExp_Compound) {
        if (!e->isLiteral()) {
            if (!e->isNot())
                expList.push_back(e);
            //std::cerr << "Adding subexpression " << e << " to expList" << std::endl;
            //if (e->giveSymbols().size() == 2)
                addSubExpressions(e->first());
            //else {
                if (e->second() != NULL) {
                    addSubExpressions(e->second());
                }
                if (e->third() != NULL) {
                    addSubExpressions(e->third());
                }
            //}
        }
    }
}
// Reads the variables and the constraints and builds some structures
void FastSifter::readInfo(std::string varFile, std::string expFile) {
    // We read the variables in order
    std::ifstream vf(varFile);
    std::ifstream ef(expFile);
    std::string temp;
    while (vf >> temp) {
        variables.push_back(temp);
        VariableTable::addVariable(temp, temp);
    }
    // We parse the expression file
    kconf::synExpDriver driver;
    driver.parse_file(expFile);
    std::vector<synExp*>  tempList;
    tempList = driver.getConstraints();
    if (!nosimplify)
        expList  = simplifyConstraints(tempList);
    else
        expList = tempList;
    
    origExpList = expList;
    
    if (!noGrouping)
        groupVariables();
    else
         fillReverseIndex();
    
    tempList = expList;
    std::cerr << "Adding expressions...";
    if (!noSubExp) {
        for(auto e : tempList)
            if (e->get_type() == synExp_Compound) {
                addSubExpressions(e->first());
                if (e->second() != NULL) {
                    addSubExpressions(e->second());
                }
                if (e->third() != NULL) {
                    addSubExpressions(e->third());
                }
            }
    }
    std::cerr << " done" << std::endl;
   
}

void FastSifter::init() {
    expressions.clear();
    // We initialize the order and the inverse mapping
    varMap.clear();
    for(int x = 0; x < variables.size(); x++) {
        varMap[variables[x]] = x;
    }
    std::set<std::set<std::string>> sets;
    // Now we go with the expressions
    for(auto x : expList)
        if (!x->isLiteral()) {
            std::unordered_set<int> vtemp;
            for(auto i : x->giveSymbols()) {
                while (varMap.find(i) == varMap.end())
                    i = representedBy[i];
                try {
                    vtemp.insert(varMap.at(i));
                }
                catch(std::exception e) {
                    std::cerr << "Set in question is " << std::endl;
                    for(auto i : x->giveSymbols())
                        std::cerr << i << " ";
                    std::cerr << std::endl;
                    lookForVariable(i);
                    throw std::logic_error("var "+i+" not in the varMap");
                }
            }
            if (vtemp.size() > 1)
                expressions.push_back(vtemp);
        }
    makeExpressionsVar();
    computeRelatedVars();
    int maxSize;
    if (variables.size() > expressions.size())
        maxSize = variables.size();
    else
        maxSize = expressions.size();
    
    scoreTable.clear();
    for(int i = 0; i < maxSize; i++)
        if (i == 0)
            scoreTable.push_back(0);
        else
            scoreTable.push_back((int)(100000000.0*log(i)));
            //scoreTable.push_back(i*i*i*i*i);
             //scoreTable.push_back(i*i);
            //scoreTable.push_back(log(i));
}
void FastSifter::aImpliesB(std::string a, std::string b) {
    equiSet.insert(std::pair<std::string, std::string>(a,b));
    //std::cerr << a << " implies " << b << std::endl;
    if (equiSet.find(std::pair<std::string, std::string>(b,a)) != equiSet.end()) {
        //std::cerr << a << " and " << b << " are equivalent" << std::endl;
        std::string toDelete;
        std::string other;
        
        if (a.find("_EQ_") !=  std::string::npos) {
            toDelete = a;
            other    =b;
        }
        else {
            toDelete = b;
            other    = a;
        }
        substituteVariable(toDelete, other);
    }
}


void FastSifter::substituteVariable(std::string toDelete, std::string other) {
    std::cerr << "substituteVariable " << toDelete << ", " << other << std::endl;
    if (toDelete == other)
        return;
    
    if (!stillExists(toDelete) && !stillExists(other))
        return;

    if (!stillExists(toDelete)) {
        std::string temp = other;
        other = toDelete;
        toDelete = temp;
    }
    // If other is already deleted, look for a suitable representative
    while(std::find(variables.begin(), variables.end(), other) == variables.end()) {
        if (other == representedBy.at(other)) {
            std::ostringstream ost;
            ost << other << " is represented by itself";
            throw std::logic_error(ost.str());
        }
        other = representedBy.at(other);
    }
    if (toDelete == other)
        return;
    //std::cerr << "Deleting " << toDelete << " represented by " << other << std::endl;
    deleted++;
    
    for(auto& s : rewriteRules[toDelete])
        rewriteRules[other].push_back(s);
    
    rewriteRules.erase(toDelete);
    std::vector<std::string>::iterator its =
        find(variables.begin(), variables.end(), toDelete);
    
    if (its != variables.end()) {
        //std::cerr << "Deleting " << *its << " from variables" << std::endl;
        variables.erase(its);
    }
    else
        throw std::logic_error(toDelete+ " was already deleted from variables");
    
    std::set<std::string> related;
    for(auto s : xReverseIndex[toDelete])
        for(auto& x : *s)
            if (x != toDelete)
                related.insert(x);
    
//    std::cerr << "Related vars ";
//    for(auto& q: related) std::cerr << q << " ";
//    std::cerr << std::endl;
    
    for(auto& x : related)
        for(auto s2 : xReverseIndex[x]) {
            if (s2->find(toDelete) != s2->end()) {
                s2->erase(toDelete);
                s2->insert(other);
//                if (other == "boolean_ARCH_CLOCKSOURCE_DATA")
//                    std::cerr << "ZZZ 3 Because " << toDelete << " was in xReverseIndex of " << x << " inserting boolean_ARCH_CLOCKSOURCE_DATA in xReverseIndex of "
//                    << x << std::endl;
                xReverseIndex[other].insert(s2);
            }
        }
    xReverseIndex.erase(toDelete);
    if (toDelete != other) {
        std::cerr << toDelete << " representedBy " << other << std::endl;
        representedBy[toDelete] = other;
    }
//    if (lookForVariable(toDelete)) {
//        throw std::logic_error("Variable "+toDelete+ " was not really deleted");
//    }
}

bool FastSifter::stillExists(std::string v) {
    return std::find(variables.begin(), variables.end(), v) != variables.end();
}
void FastSifter::findEquivalent() {
    // Look for equivalent variables, such as A -> B and B -> A
    // or mutually exclusive variables, such as not A or not B, or XOR(A, B)
    for(auto exp : expList) {
        // OR
        if (exp->isOr() && exp->first()->isLiteral() && exp->second()->isLiteral()) {
            // A -> B
            if (exp->first()->isNot() && exp->second()->isSymbol()) {
                std::string a = exp->first()->first()->getSymbol();
                std::string b = exp->second()->getSymbol();
                aImpliesB(a,b);
            }
            else
                // B -> A
                if (exp->first()->isSymbol() && exp->second()->isNot()) {
                    std::string a = exp->first()->getSymbol();
                    std::string b = exp->second()->first()->getSymbol();
                    aImpliesB(b,a);
                }
                else
                    // A -> ¬B
                    if (exp->first()->isNot() && exp->second()->isNot()) {
                    substituteVariable(exp->first()->first()->getSymbol(),
                                       exp->second()->first()->getSymbol());
                }
        }
        // Implies
        else if (exp->isImplies() && exp->first()->isLiteral() && exp->second()->isLiteral()) {
            if (exp->first()->isSymbol() && exp->second()->isSymbol()) {
                std::string a = exp->first()->getSymbol();
                std::string b = exp->second()->getSymbol();
                aImpliesB(a,b);
            }
            else
                if (exp->first()->isSymbol() && exp->second()->isNot()) {
                    substituteVariable(exp->first()->getSymbol(),
                                       exp->second()->first()->getSymbol());
                }
                else
                    if (exp->first()->isNot() && exp->second()->isSymbol()) {
                        substituteVariable(exp->first()->first()->getSymbol(),
                                           exp->second()->getSymbol());
                    }
        }
        // If
        else if (exp->isIf()               &&
                 exp->first()->isSymbol()  &&
                 exp->second()->isLiteral() &&
                 exp->third()->isLiteral()) {
            std::string a = exp->first()->getSymbol();
            if (exp->second()->isSymbol() && exp->third()->isNot() &&
                exp->third()->first()->getSymbol() == exp->second()->getSymbol()) {
                aImpliesB(a, exp->second()->getSymbol());
                aImpliesB(exp->second()->getSymbol(), a);
            }
            else
                if (exp->second()->isNot() && exp->third()->isSymbol() &&
                    exp->third()->getSymbol() == exp->second()->first()->getSymbol()) {
                    std::string b = exp->third()->getSymbol();
                    if (a.find("_EQ_") !=  std::string::npos) {
                        substituteVariable(a, b);
                    }
                    else {
                        substituteVariable(b, a);
                    }
                }
        }
        else // XOR
            if (exp->isXOR() && exp->get_parms().size() == 2) {
                std::string a = exp->get_parms()[0]->getSymbol();
                std::string b = exp->get_parms()[1]->getSymbol();
                if (a.find("_EQ_") !=  std::string::npos) {
                    substituteVariable(a, b);
                }
                else {
                    substituteVariable(b, a);
                }
            }
    }
}
void FastSifter::fillReverseIndex() {
    xReverseIndex.clear();
    for(auto s : expList) {
        std::set<std::string> *tempSet = new std::set<std::string>();
        for(auto& v : s->giveSymbols()) {
            if (representedBy.find(v) != representedBy.end())
                tempSet->insert(representedBy[v]);
            else
                tempSet->insert(v);
        }
        for(auto& v : s->giveSymbols())
            xReverseIndex[v].insert(tempSet);
    }
}
void FastSifter::groupVariables() {
    std::cerr << "Grouping variables...";
    bool changes;
    for(auto& v : variables)
        rewriteRules[v].push_back(v);
    // Fill reverse index with strings, no numbers yet
    fillReverseIndex();
    std::cerr << "start find equivalent" << std::endl;
    findEquivalent();
    std::cerr << "end find equivalent" << std::endl;
    do {
        //std::cerr << "-----------------" << std::endl;
        changes = false;
        xRelatedVars.clear();
        for(auto& v : variables)
            for(auto& s : xReverseIndex[v])
                for(auto& w : *s) {
                    xRelatedVars[v].insert(w);
                }
        for(auto& p : xRelatedVars)
            // If only related with one other var
            if (p.second.size() == 2) {
                std::set<std::string>::iterator its = p.second.begin();
                std::string other = *its++;
                if (other == p.first)
                    other = *its;
                // If the other has not been previously deleted...
                if (rewriteRules.find(other) != rewriteRules.end()) {
                    changes = true;
                    //std::cerr << p.first << " only interacts with " << other << std::endl;
                    substituteVariable(p.first, other);
                }
            }
        // Now, the other thing.
        // Maps the related set to the set of variables having that related set.
        // All the variables with the same related set must be reduced to one representative
        
        // It doesn't work as it is for bits, bc it reduces all of them to just one
        // variable
        
        std::map<std::set<std::string>, std::set<std::string>> theMap;
        for(auto& s : variables)
            theMap[xRelatedVars[s]].insert(s);

        for(auto& p : theMap) {
            while (p.second.size() > 1) {
                changes = true;
                std::string toDelete = *(p.second.begin());
                p.second.erase(toDelete);
                std::string other = *(p.second.begin());
                std::cerr << toDelete << " and " << other << " have the same related variables" << std::endl;
                substituteVariable(toDelete, other);
            }
        }
    } while (changes);
    std::cerr << "Variables deleted " << deleted << std::endl;
}
        
// Inverted index that connects a variable with the expressions it appears in
void FastSifter::makeExpressionsVar() {
    // For each var we make a list of the related expressions
    expressionsVar.resize(variables.size());
    for(int exp = 0; exp < expressions.size(); exp++)
        for(auto& var : expressions[exp]) {
            expressionsVar[var].insert(exp);
        }
}

void FastSifter::computeRelatedVars() {
    relatedVars.resize(variables.size());
    for(int var = 0; var < variables.size(); var++)
        for(auto exp : expressionsVar[var])
            for(auto other : expressions[exp])
                if (other != var)
                    relatedVars[var].insert(other);
}
void    FastSifter::computeIntersections() {
    for(int vec = 0; vec < expressions.size(); vec++)
        for(std::unordered_set<int>::iterator its = expressions[vec].begin();
            its != expressions[vec].end(); its++) {
            std::unordered_set<int>::iterator itx = its;
            itx++;
            while (itx != expressions[vec].end()) {
                if (*its < *itx)
                    intersection[*its][*itx].insert(vec);
                else
                    intersection[*itx][*its].insert(vec);
                itx++;
            }
        }
}


// Alloc and populate the structures that map the constraints with a given max and min position
void FastSifter::initSwap() {
    // First allocate the vector of bit sets
    minVector.resize(variables.size());
    maxVector.resize(variables.size());
    vSpan.resize(expressions.size());
    
    for(int pos = 0; pos < variables.size(); pos++) {
        minVector[pos] = new std::unordered_set<int>();
        maxVector[pos] = new std::unordered_set<int>();
    }
    // Now compute the max and min of each expression and also the span
    for(int vector = 0; vector < expressions.size(); vector++) {
        int minPos = std::numeric_limits<int>::max();
        int maxPos = 0;
        for(auto v : expressions[vector]) {
            if (index2pos[v] > maxPos)
                maxPos = index2pos[v];
            if (index2pos[v] < minPos)
                minPos = index2pos[v];
        }
        if (!expressions[vector].empty()) {
            minVector[minPos]->insert(vector);
            maxVector[maxPos]->insert(vector);
            vSpan[vector] = maxPos - minPos;
        }
        else
            vSpan[vector] = 0;
    }
}

void FastSifter::computeComponents() {
    pcomp = new MultiComponents(index2pos, pos2index);
    for(auto vec : expressions)
        pcomp->vectorJoin(vec);

    pcomp->reorder();
    pos2index = pcomp->getOrder();
    int c = 0;
    for(auto i : pos2index)
        index2pos[i] = c++;
}

void FastSifter::siftComponent(int start, int length) {
    // The actual indices of the variables in the component
    std::unordered_set<int> indexSet;
    std::unordered_set<int> compVectors;
    
    // We build all the constraints involved in this component
    for(int pos = start; pos < start+length; pos++)
        for(auto exp : expressionsVar[pos2index[pos]])
            compVectors.insert(exp);
    
    
    score = computeScore(compVectors);
    std::cerr << "Component with start " << start << " and length " << length << std::endl;
    for(int pos = start; pos < start+length; pos++) {
        indexSet.insert(pos2index[pos]);
    }
    std::cerr << "Initial      score " << std::right << std::fixed << std::setw(15) << score << std::endl;
    score_t lastScore, initScore = score;
    double bigStart = get_cpu_time();
    do {
        double startTime = get_cpu_time();
        lastScore = score;
        for(auto index : indexSet)
            sift(index2pos[index], start, length);
        if (lastScore != 0)
            std::cerr << "Intermediate score " << std::right << std::fixed << std::setw(15)
            << score << "        improvement "
            << std::right << std::fixed << std::setw(15)
            << 100.0*(lastScore - score)/lastScore << "% "
            <<  showtime(get_cpu_time() - startTime)
            << std::endl;
        else
            std::cerr << "Intermediate score " << std::right << std::fixed << std::setw(15) << score << "  improvement not applicable"  << std::endl;
    // We delete abs bc sometimes the difference is negative
    } while (lastScore - score > 0.00000001);
    if (initScore != 0)
        std::cerr << "Final score,       " << std::right << std::fixed << std::setw(15)
                << score << "  final improvement " << std::right << std::fixed
                << std::setw(15) << 100.0*(initScore - score)/initScore << "% "
                <<  showtime(get_cpu_time() - bigStart) << std::endl;
    else
        std::cerr << "Final score,       " << std::right << std::fixed << std::setw(15) << score << "  final improvement not applicable "
                  <<  showtime(get_cpu_time() - bigStart) << std::endl;
}

void FastSifter::sift(int pos, int start, int length)  {
    if (verbose) std::cerr << ".";
    int bestpos = pos;
    score_t bestScore = score;
    int upperBound, lowerBound;

    upperBound = start+length-1;
    lowerBound = start;

    int dir = (pos - start) > length/2 ? 1 : -1;
    // Traverse all positions to find the best one
    for(int laps = 0; laps < 2; laps++, dir *=-1) {
        if (dir == 1) {
            
            while  (pos < upperBound) {
                swapPos(pos);
                pos++;
                if (score < bestScore) {
                    bestpos = pos;
                    bestScore = score;
                }
            }
        }
        else {
            while (pos > lowerBound) {
                swapPos(pos-1);
                pos--;
                if (score < bestScore) {
                    bestpos = pos;
                    bestScore = score;
                }
            }
        }
    }
    // Now back to the best position
    if (bestpos > pos)
        while (bestpos > pos) {
            swapPos(pos);
            pos++;
        }
    else
        if (bestpos < pos)
            while (bestpos < pos) {
                swapPos(pos-1);
                pos--;
            }
}

// Swaps pos and pos+1
score_t FastSifter::swapPos(int pos) {
    numSwaps++;
    score_t dif         = 0;
    int varLeft         = pos2index[pos];
    int varRight        = pos2index[pos+1];
    index2pos[varLeft]  = pos + 1;
    index2pos[varRight] = pos;
    pos2index[pos]      = varRight;
    pos2index[pos+1]    = varLeft;
    
    std::unordered_set<int> *itmaxVectorPos        = maxVector[pos];
    std::unordered_set<int> *itmaxVectorPosPlusOne = maxVector[pos+1];
    std::swap(*itmaxVectorPos, *itmaxVectorPosPlusOne);
    
    // If a vector had max in pos, the new max is pos+1

    for(auto exp : *itmaxVectorPosPlusOne) {
        dif += giveScore(vSpan[exp]+1) - giveScore(vSpan[exp]);
        vSpan[exp]++;
    }
    
    int smallVar, bigVar;
    if (varLeft < varRight) {
        smallVar = varLeft;
        bigVar   = varRight;
    }
    else {
        smallVar = varRight;
        bigVar   = varLeft;
    }
    // If a vector had max in pos+1 (varRight), the new max is pos
    // unless the variable in varLeft is also present in the constraint
    std::unordered_map<int, std::unordered_set<int>>& fit = intersection[smallVar];
    std::unordered_map<int, std::unordered_set<int>>::iterator itm = fit.find(bigVar);
    if (itm != fit.end()) {
        for(auto exp : itm->second) {
            std::unordered_set<int>::iterator its = itmaxVectorPos->find(exp);
            if (its != itmaxVectorPos->end()) {
                itmaxVectorPosPlusOne->insert(exp);
                itmaxVectorPos->erase(its);
            }
        }
    }
    
    for(auto vec : *itmaxVectorPos) {
        dif += giveScore(vSpan[vec]-1) - giveScore(vSpan[vec]);
        vSpan[vec]--;
    }

    // Swap
    std::unordered_set<int> *itminVectorPos        = minVector[pos];
    std::unordered_set<int> *itminVectorPosPlusOne = minVector[pos+1];
    std::swap(*itminVectorPos, *itminVectorPosPlusOne);
    // If a constraint had a min in pos+1, the new min is pos
    for(auto vec : *itminVectorPos) {
        dif += giveScore(vSpan[vec]+1) - giveScore(vSpan[vec]);
        vSpan[vec]++;
    }
    
    // If a constraint had a min in pos (varLeft), the new min is pos+1
    // unless the variable varRight is also present in the constraint
    if (itm != fit.end()) {
        for(auto vec : itm->second) {
            std::unordered_set<int>::iterator its = itminVectorPosPlusOne->find(vec);
            if (its != itminVectorPosPlusOne->end()) {
                itminVectorPos->insert(vec);
                itminVectorPosPlusOne->erase(its);
            }
        }
    }

    for(auto vec : *itminVectorPosPlusOne) {
        dif += giveScore(vSpan[vec]-1) - giveScore(vSpan[vec]);
        vSpan[vec]--;
    }
    score += dif;
    return dif;
}

score_t FastSifter::computeAllScore() {
    score_t res = 0;
    for(int vec = 0; vec < expressions.size(); vec++)
        res += computeScore(vec);
    
    return res;
}
    
score_t FastSifter::computeScore(std::unordered_set<int>& vec) {
    score_t res = 0;
    for(auto e : vec)
        res += computeScore(e);
    
    return res;
}

score_t FastSifter::computeScore(int exp) {
    return giveScore(computeSpan(exp));
}

int FastSifter::computeSpan(int vec) {
    if (expressions[vec].empty()) return 0;
    int minPos = std::numeric_limits<int>::max();
    int maxPos = 0;
    for(auto v : expressions[vec]) {
        if (index2pos[v] > maxPos) maxPos = index2pos[v];
        if (index2pos[v] < minPos) minPos = index2pos[v];
    }
    return maxPos - minPos;
}

score_t FastSifter::giveScore(int span) {
    return scoreTable[span];
    //return span == 0 ? 0 : span*log(span);
}

void FastSifter::printVector(std::vector<int>& v) {
    for(int x : v)
        std::cerr << x << " ";
}

int FastSifter::maxLeftMove(std::vector<int>& v) {
    //printVector(v);
    int max = -1;
    for(int c = 0; c < v.size(); c++)
        if (v[c] - c > max)
            max = v[c] -c;
    return max;
}

// Build a path from the last perm to this permutation
std::vector<int> FastSifter::goBackSeq(std::vector<int> lastPerm,
                                       const std::vector<int>& thisPerm) {
    std::vector<int> res;
    int n = lastPerm.size();
    int x;
    for(int pos = 0; pos < n; pos++) {
        // Look for variable in position pos in the current permutation
        int vp = thisPerm[pos];
        for(x = pos; x < n; x++)
            if (lastPerm[x] == vp) break;
        
        while (x != pos) {
            res.push_back(x-1);
            std::swap(lastPerm[x-1], lastPerm[x]);
            x--;
        }
    }
    return res;
}


void FastSifter::generateSequence(int n) {
    std::vector<int> seq  , previousSeq;
    std::vector<int> moves, currentPerm, lastPerm;
    std::vector<std::vector<int>> thisPathBack;
    if (n < 2) {
        std::cerr << "The argument to generateSequence must be at least 2. It is " << n << std::endl;
        exit(-1);
    }
    for(int x = 0; x < n; x++)
        currentPerm.push_back(x);
    
    lastPerm = currentPerm;
    std::swap(lastPerm[0], lastPerm[1]);
    std::vector<int> temp;
    temp.push_back(0);
    thisPathBack.push_back(temp);

    if (n == 2) {
        seq.push_back(0);
        moves.push_back(0);
        moves.push_back(1);
        permSeq.push_back(seq);
        leftMove.push_back(moves);
        pathBack.push_back(thisPathBack);
        
        return;
    }
    generateSequence(n-1);
    moves.push_back(0);

    previousSeq   = permSeq.back();
    int polarity = -1;
    for(int c = 0; c < previousSeq.size(); c++) {
        for(int j = 0; j < n-1; j++)
            if (polarity == -1) {
                seq.push_back(n-j-2);
                std::swap(currentPerm[n-j-2], currentPerm[n-j-1]);
                moves.push_back(maxLeftMove(currentPerm));
                thisPathBack.push_back(goBackSeq(lastPerm, currentPerm));
            }
            else {
                seq.push_back(j);
                std::swap(currentPerm[j], currentPerm[j+1]);
                moves.push_back(maxLeftMove(currentPerm));
                thisPathBack.push_back(goBackSeq(lastPerm, currentPerm));
            }

        if (polarity == -1) {
            seq.push_back(previousSeq[c]+1);
            std::swap(currentPerm[previousSeq[c]+1], currentPerm[previousSeq[c]+2]);
            moves.push_back(maxLeftMove(currentPerm));
            thisPathBack.push_back(goBackSeq(lastPerm, currentPerm));
        }
        else {
            seq.push_back(previousSeq[c]);
            std::swap(currentPerm[previousSeq[c]], currentPerm[previousSeq[c]+1]);
            moves.push_back(maxLeftMove(currentPerm));
            thisPathBack.push_back(goBackSeq(lastPerm, currentPerm));
        }

        polarity *= -1;
    }
    for(int j = 0; j < n-1; j++)
        if (polarity == -1) {
            seq.push_back(n-j-2);
            std::swap(currentPerm[n-j-2], currentPerm[n-j-1]);
            moves.push_back(maxLeftMove(currentPerm));
            thisPathBack.push_back(goBackSeq(lastPerm, currentPerm));
        }
        else {
            seq.push_back(j);
            std::swap(currentPerm[j], currentPerm[j+1]);
            moves.push_back(maxLeftMove(currentPerm));
            thisPathBack.push_back(goBackSeq(lastPerm, currentPerm));
        }

    permSeq.push_back(seq);
    leftMove.push_back(moves);
    pathBack.push_back(thisPathBack);
}

    
void FastSifter::checkSpan() {
    for(int e = 0; e < expressions.size();e++)
        if (vSpan[e] != computeSpan(e)) {
            std::cerr << e << " " << expList[e] << std::endl;
            std::cerr << "vSpan " << vSpan[e] << " real Span " << computeSpan(e) << std::endl;
            throw std::logic_error("Nooooo");
        }
}


std::vector<synExp*> FastSifter::simplifyConstraints(std::vector<synExp*> pendingConstraints) {
    bool changes;
    std::vector<synExp*>  otherConstraints;
    int numchanges = 0;
    int setToTrue  = 0;
    int setToFalse = 0;
    std::cerr << "Simplify Constraints...";
    do {
        changes = false;
        numchanges = 0;
        int counter = 1;
        otherConstraints.clear();
        for(synExp *e : pendingConstraints) {
            bool simplified = false;
            if (e->isSymbol()) {
                // Constraint is a variable
                std::string sn = e->getSymbol();
                synExp* wValue = VariableTable::getValue(sn);
                if (wValue != synTrue) {
                    simplified = true;
                    // If we have A and later not A or vice versa we may
                    // flip A from true to false forever. So we check and
                    // later we'll get an unsatisfiable set
                    if (wValue != synTrue && wValue != synFalse) {
                        setToTrue++;
                        numchanges++;
                        VariableTable::setValue(sn, synTrue);
                        changes = true;
                    }
                    else if (wValue == synFalse) {
                        std::cerr << "1 Var " << std::setw(50) << sn << " is true and false "
                        << " in expression (" << counter << ") " << e << std::endl;
                        exit(-1);
                    }
                    
                }
            }
            else {
                if (e->isNot() && e->first()->isSymbol()) {
                    // Constraint is "not Variable"
                    std::string sn = e->first()->getSymbol();
                    synExp* wValue = VariableTable::getValue(sn);
                    if (wValue != synFalse) {
                        simplified = true;
                        if (wValue != synTrue && wValue != synFalse) {
                            numchanges++;
                            setToFalse++;
                            VariableTable::setValue(sn, synFalse);
                            changes = true;
                        }
                        else
                            if (wValue == synTrue) {
                                std::cerr << "2 Var " << std::setw(50) << sn << " is true and false "
                                << " in expression (" << counter << ") " << e << std::endl;
                                exit(-1);
                            }
                    }
                }
            }
            if (!e->isLiteral()) {
                e = eval(e);
            }
            counter++;
            if (e != synTrue)
                otherConstraints.push_back(e);
        };
        pendingConstraints.clear();
        pendingConstraints = otherConstraints;
    }
    while (changes);
    std::cerr << " done" << std::endl;
    return pendingConstraints;
}

void    FastSifter::addSymbols2Table(std::vector<std::string> vars) {
    for(std::string s : vars) {
        VariableTable::addVariable(s, s);
    }
}


bool    FastSifter::lookForVariable(std::string var) {
    bool res = false;
    // First, look in the variables vector
    if (std::find(variables.begin(), variables.end(), var) != variables.end()) {
        std::cerr << var << " found in variables" << std::endl;
        res = true;
    }
    
    for(auto& p : xReverseIndex)
        for(auto s : p.second)
            if (s->find(var) != s->end()) {
                std::cerr << var << " found in xReverseIndex for " << p.first << std::endl;
                res = true;
            }
    return res;
}

