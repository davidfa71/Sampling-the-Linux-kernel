//
//  bddBuilder.cpp
//  myKconf
//
//  Created by david on 11/10/14.
//  Copyright (c) 2014 david. All rights reserved.
//

#include "bddBuilder.hpp"
bddBuilder::bddBuilder(std::vector<synExp*>&        consList) :  consList(consList) {
    reorder         = true;
    minNodes        = 0;
    maxNodes        = std::numeric_limits<int>::max();
    minConstraint   = -1;
    reorderEvery    = 1;
    stopNodes       = std::numeric_limits<int>::max();
    for(auto exp : consList)
        for(auto& var : exp->giveSymbols())
            occurrences[var]++;
};

bddBuilder::~bddBuilder() {
    delete myOrder;
}

void bddBuilder::build(double inflation) {
    bool lastReorder      = true;
    int  nreorder         = 0;
    int  constLastReorder = 0;
    bool componentChanges = false;
    varFile.open(varFileName);
    statsFile.open(statsFileName);
    
    long beginning = get_cpu_time();
    int  lastNodes = adapter->nodecount();
    writeNameOrder(adapter->giveOrder());
    int pos = 0;
    for(auto& var : adapter->giveOrder())
        originalPos[var] = pos++;
    
    apply(synTrue);
    std::vector<synExp*> pendingJoin;
    int backCount = 0;
    // Main loop. A new constraint is added to the BDD each time
    int  origSize    = consList.size();
    int  counter     = 0;
    bool force       = false;
    int  min         = std::numeric_limits<int>::max();
    int  minCounter  = 0;
    while (counter < consList.size()) {
        long start = get_cpu_time();
        myOrder->reorderConstraints(consList.begin()+counter,
                                    consList.end(),
                                    lastReorder || componentChanges);
        lastReorder      = false;
        componentChanges = false;
        synExp *pconst   = consList[counter+backCount];
        if (pconst->isComment() && verbose) {
            std::cout << "#"  << pconst->getComment() << std::endl;
            appliedConstraints.push_back(pconst);
            componentChanges = false;
            counter++;
            continue;
        }
        if (verbose)
            printPartOne(pconst,
                         counter+1,
                         origSize);
        
        apply(pconst);
        int nn = nodecount();

        if (isAlmostReorderTime(lastNodes, nn, inflation) && !force) {
            undo();
            if (nn < min) {
                min = nn;
                minCounter = counter+backCount;
            }
            std::cerr   << "<<<<<<<<<<<<<<<<<< " << std::setw(7) << showHuman(nn)
                        << " nodes. " << "Back counter " << std::setw(6) << backCount+1
                        << " " << (counter+backCount)*100/origSize << "%" << std::endl;
            if (counter+backCount == consList.size()-1) {
                std::cerr << "Setting up minimum of " << showHuman(min) << " nodes" << " in "
                          << minCounter << std::endl;
                exchange(counter, minCounter);
                appliedConstraints.push_back(consList[counter]);
                force       = true;
                backCount   = 0;
                min         = std::numeric_limits<int>::max();
                minCounter  = -1;
            }
            else
                backCount++;
            continue;
        }
        if (backCount > 0) {
            exchange(counter, counter+backCount);
            min         = std::numeric_limits<int>::max();
            writeApplied();
            writeUnApplied(consList.begin()+counter);
        }
        appliedConstraints.push_back(consList[counter]);

        force     = false;
        backCount = 0;
        
        if (reorder) {
            if (doUndo)
                pendingJoin.push_back(pconst);
            else
                adapter->join(pconst);
        }

        if (verbose)
            printPartTwo(nn);
        
        statsFile       << 100.0*counter/consList.size() << " " << nn << std::endl;
        if (nodecount() > stopNodes) {
            break;
        }
        if (counter > minConstraint ) {
            if (isReorderTime(counter, lastNodes, nn, constLastReorder, inflation)) {
                adapter->dropUndo();
                for(auto x : pendingJoin)
                    adapter->join(x);
                pendingJoin.clear();
                if (verbose)
                    std::cout << " -> " << std::flush;
                adapter->syncOrder();
                componentChanges = true;
                nn = nodecount();
                if (verbose)
                    std::cout << std::setw(8) << showHuman(nn);
                lastReorder      = true;
                if (isReorderTime(counter, lastNodes, nn, constLastReorder, inflation)) {
                    if (verbose)
                        std::cout << std::endl;
                    constLastReorder = counter;
                    nreorder++;
                    reorderProtocol(consList.begin()+counter);
                    if (verbose)
                        std::cout << " => " << std::setw(8) << showHuman(nodecount());
                    writeNameOrder(adapter->giveOrder());
                }
            }
            
        }
        if (verbose)
            std::cout   << " " << std::setw(15)       << showtime(get_cpu_time() - start)
                        << " total " << std::setw(15) << showtime(get_cpu_time() - beginning)
                        << std::endl;
        lastNodes = nodecount();
        myOrder->thisOneIsProcessed(pconst);
        pconst->thisOneIsProcessed();
        counter++;
    }
    writeApplied();
    remove(unappliedExpFilename.c_str());
    // Destroy the expressions
    for(auto p : consList)
        p->destroy();
    
//    int nn = adapter->nodecount();
//    if (reorder && !lastReorder && nn < maxNodes) {
//        std::cout    << get_timestamp()      << " Last reordering. Nodes "
//                    << showHuman(lastNodes) << std::endl;
//        std::cout    << std::endl << "Syncing components..." << std::flush;
//        // If nreorder is 0 we don't need to sync anything and we save the time of building the
//        // interaction matrix, which could take a while
//        adapter->syncOrder();
//        std::cout << "done" << std::endl << std::flush;
//        adapter->reorder(reorderMethod);
//        std::cout   << get_timestamp()          << " Reordering done. "
//        << showHuman(adapter->nodecount())     << " nodes. " << std::endl << std::flush;
//    }
//    // Write the last order
//    writeNameOrder(adapter->giveOrder());
    // Delete the tempBDD, appliedExp and unappledExp?
    //remove(tempBDDName.c_str());
    //remove(appliedExpFilename.c_str());
    //remove(unappliedExpFilename.c_str());
    //adapter->destroyInternal(internal);
    statsFile.close();
}

void bddBuilder::printOrder(std::vector<int> ord) {
    std::cout << std::endl << std::setw(5) << ord.size() << "              ";
    for(std::vector<int>::iterator itv = ord.begin(); itv != ord.end(); itv++)
        std::cout << *itv << " ";
    std::cout << std::endl;
    
}

void bddBuilder::writeNameOrder(const std::vector<std::string>& ord) {
    for(const std::string &s : ord)
        varFile << s << " ";
    
    varFile << std::endl;
}

//void bddBuilder::readVarOrder() {
//    std::cout << "Continuing from a previous order" << std::endl;
//    std::string var;
//    std::vector<std::string> tempvect;
//    int nvars   = 0;
//    int nconsts = 0;
//    int counter = 0;
//    std::ifstream in(varFileName.c_str());
//    if (in.good()) {
//        std::string temp, line;
//        while (std::getline(in, temp)) {
//            line = temp;
//        }
//        pos2var.clear();
//        std::istringstream ist(line);
 //       ist >> nconsts >> nvars;
//        std::cout << "nconsts " << nconsts << " nvars " << nvars << std::endl;
//        int c = 0;
//        var2pos.resize(varMap.size());
//        while (ist >> var) {
 //           var2pos[varMap[var]] = c;
//            pos2var[c++] = varMap[var];
//        }
//        reorderFromConst = nconsts;
//        std::cout   << "Pick up ordering from file. Reordering disabled until constraint #"
//                    << reorderFromConst << std::endl;
//    }
//}


//void bddBuilder::printPartOne(synExp* pconst, int counter) {
//    std::cout   << get_timestamp();
//    std::stringstream ost;
//    //ost  << std::setw(95) << std::left << pconst;
//    ost  << pconst;
//    std::string stConst = ost.str();
//    std::cout    << " Cnstr #" << std::setw(6)     << counter      << " "
//                << std::setw(3) << 100*counter/consList.size() << "% ";
//    std::string s;
//    s =  myOrder->giveInfo(pconst);
//    std::cout << s;
//    
//    std::cout << ": ";
//    if (stConst.length() <= lineLength)
//        std::cout <<  std::setw(lineLength) << std::left << stConst;
//    else
//        std::cout   <<   std::left <<  stConst  << std::endl
//        << std::setw(38+lineLength+s.length());
//}
void bddBuilder::printPartOne(synExp* pconst, int counter, int origSize) {
    std::stringstream ost, sConst;
    ost << get_timestamp();
    ost << " Cnstr #" << std::setw(6)     << counter      << " "
        << std::setw(3) << 100*counter/origSize << "% ";
    ost << myOrder->giveInfo(pconst);
    ost << ": ";
    std::string beginning = ost.str();
    sConst << pconst;
    std::string stConst = sConst.str();
    std::cout << beginning; 
    if (stConst.length() <= lineLength)
        std::cout <<  std::setw(lineLength) << std::left << stConst;
    else
        std::cout << std::left << stConst  << std::endl
                  << std::setw(beginning.length()+lineLength) << " ";
}

void bddBuilder::printPartTwo(int nn) {
    std::cout       << std::right << " Nodes "        << std::setw(8) << std::right
                    << showHuman(nn)    << std::flush;
}


bool bddBuilder::isReorderTime(int counter, int lastNodes, int nn, int constLastReorder, double inflation) {
    return reorder && (counter > reorderFromConst) &&
    ((reorderEvery == 1 && reorder && nn > (1.0+inflation/100.0)*lastNodes && nn > minNodes && nn < maxNodes
      && counter - constLastReorder > reorderInterval)
     || (reorderEvery > 1 && counter % reorderEvery == 0));
    
}

bool bddBuilder::isAlmostReorderTime(int lastNodes, int nn, double inflation) {
    //std::cerr << "lastNodes " << lastNodes << " inflation " << inflation << " nn " << nn
    //    << " minnodes "<< minNodes << std::endl;
    if (!doUndo)
        return false;
    if (lastNodes > 0)
        return nn > minNodes && nn > (1.0+inflation/100.0)*lastNodes;
    return false;
    
}

void bddBuilder::showVarsPositions(synExp* s) {
    std::cerr << "----------" << std::endl;
    for(auto& var : s->giveSymbols()) {
        int index = adapter->getVarIndex(var);
        int pos   = adapter->var2pos()[index];
        std::cerr << std::setw(40) << var << " pos "
                << std::setw(6) << pos
                << " original pos " << std::setw(6) << originalPos[var] << " ocurrences "
                << std::setw(6) << occurrences[var] << " level nodes "
                << adapter->getLevelNodes(pos) << std::endl;
    }
    std::cerr << "----------" << std::endl;
}
void bddBuilder::reorderProtocol(std::vector<synExp*>::iterator itl) {
    //showVarsPositions(*itl);
    // Write the BDD to a file
    //adapter->saveBDD(tempBDDName, "");
    //writeApplied(itl);
    // Write the constraint not yet applied
    //std::ofstream unapplied(unappliedExpFilename);
    //it = itl;
    //it++;
    //while (it != consList.end())
    //    unapplied << *it++ << std::endl;
    
    //unapplied.close();
    // Finally reorder
    adapter->reorder(reorderMethod);
    //showVarsPositions(*itl);
}

void bddBuilder::writeUnApplied(std::vector<synExp*>::iterator itl) {
    std::ofstream unapplied(unappliedExpFilename);
    while (itl != consList.end())
        unapplied << *itl++ << std::endl;
    unapplied.close();
}
void bddBuilder::writeApplied() {
// Write the constraints already applied
std::ofstream applied(appliedExpFilename);
for(auto x : appliedConstraints)
    applied << x << std::endl;

applied.close();
}

void bddBuilder::apply(synExp* pconst) {
if (functionName == "")
    adapter->applyLimit(pconst, stopNodes);
else
    adapter->applyLimit(functionName, pconst, stopNodes);
}

void bddBuilder::undo() {
    if (functionName == "")
        adapter->undoApply();
    else
        adapter->undoApply(functionName);
}

int  bddBuilder::nodecount() {
    if (functionName == "")
        return adapter->nodecount();
    else
        return adapter->nodecount(functionName);
}

void bddBuilder::exchange(int counter, int minCounter) {
    synExp* temp         = consList[counter];
    consList[counter]    = consList[minCounter];
    consList[minCounter] = temp;
}
