/* This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Written by David Fernandez Amoros 2021

#include "randomAlgo.hpp"
//#include <random>
int verbose = 0;
// For each node, the probability to reach the true node choosing the then child
std::unordered_map<DdNode*, unsigned long>                                 probabilities;

void getProducts(int plevel, int tlevel, int elevel, const mpz_class& tr, const mpz_class& er, mpz_class& thenPart, mpz_class& elsePart) {
    // thenPart = 2 ^ (tlevel-plevel-1) * tr
    mpz_ui_pow_ui(thenPart.get_mpz_t(), 2, tlevel - plevel -1);
    thenPart *= tr;
    
    // elsePart = 2 ^(elevel - plevel -1) * er
    mpz_ui_pow_ui(elsePart.get_mpz_t(), 2, elevel - plevel -1);
    elsePart *= er;
}

mpz_class _probabilities(int plevel, int tlevel, int elevel, const mpz_class& tr, const mpz_class& er, DdNode* node) {
    mpz_class thenPart, elsePart;
    getProducts(plevel, tlevel, elevel, tr, er, thenPart, elsePart);

    // If we set probability to 1, comparison with a
    // random number equal to one will fail as in
    // if random < probability then choose the high child
    // This is important for generating random samples
    if (elsePart == 0)
        probabilities[node] = (RAND_MAX>>1)+1;
    else {
        mpz_class x = (RAND_MAX>>1)*thenPart / (thenPart + elsePart);
        probabilities[node] = x.get_ui();

    }
    return (thenPart + elsePart);
}

int compProbabilitiesMP(int verbose, int threads, cuddAdapter* a, unsigned int& seed, bool fast) {
    if (seed == 0)
     seed = (unsigned int) get_cpu_time() % 1000000;
    srandom(seed);
    traverseMP(verbose, threads, a, mpz_class(0), mpz_class(1), &_probabilities, fast);
    return seed;
}

int compProbabilities(int verbose, cuddAdapter* a, unsigned int& seed) {
    if (seed == 0)
     seed = (unsigned int) get_cpu_time() % 1000000;
    srandom(seed);
    traverse(verbose, a, mpz_class(0), mpz_class(1), &_probabilities);
    return seed;
}

bool fiftyfifty() {
    //float randNum = (float)random()/(float)RAND_MAX;
    //return (randNum < 0.5);
    return (random() & 1) == 0;
}

std::string nameProduct(cuddAdapter*a, std::vector<bool> v) {
    std::ostringstream ost;
    int var = 0;
    for(bool b : v) {
        std::string nameVar = a->getVarName(var++);
        if (b)
            ost <<  "    " << nameVar << " ";
        else
            ost << "not " << nameVar << " ";
    }
    return ost.str();
}

std::string nameRandom(cuddAdapter *a) {
    std::ostringstream ost;
    return nameProduct(a, genRandom(a));
}

std::vector<bool> genRandom(cuddAdapter* a) {
    // We assume the probabilities have been computed already
    DdNode *g1, *g0;
    int index;
    std::vector<bool> exemplar(a->getNumVars());
    DdNode *one  = a->getOne();
    DdNode *zero = a->getZero();
    DdNode *trav = a->getBDD();

    int    pos   = 0;
    if (trav == zero)
        return exemplar;
    
    index = a->getLevel(trav);
    for(int i = 0; i < index; i++) {
        exemplar[a->varAtPos(pos++)] = fiftyfifty();
        //std::cerr << "Assigning at fifty fifty value to variable "
        //    << a->getVarName(a->varAtPos(pos-1))
        //    << exemplar[a->varAtPos(pos-1)]  << std::endl;
    }
    
    while (trav != one) {
        cuddGetBranches(trav, &g1, &g0);
        //std::cerr << "Assigning value to variable " << a->getVarName(a->varAtPos(pos))
        //    << " with probability " << probabilities[trav]/double(RAND_MAX>>1);
        if (random()>>1 < probabilities[trav]) {
            trav = g1;
            //std::cerr << " 1" << std::endl;
            exemplar[a->varAtPos(pos++)] = true;
        }
        else {
            exemplar[a->varAtPos(pos++)] = false;
            //std::cerr << " 0" << std::endl;
            trav = g0;
        }
        for(int i = index+1; i < a->getLevel(trav); i++) {
            exemplar[a->varAtPos(pos++)] = fiftyfifty();
//            std::cerr << "Assigning at fifty fifty value to variable "
//                << a->getVarName(a->varAtPos(pos-1))
//                << exemplar[a->varAtPos(pos-1)]  << std::endl;
        }
        
        index = a->getLevel(trav);
    }
    return exemplar;
}

std::string nameRandomShared(cuddAdapter *a) {
    return nameProduct(a, genRandomShared(a));
}

int nheads;
std::vector<int>       posHead;
std::vector<bool>      exemplar;
std::vector<mpf_class> productProbabilities;
mpf_class              probAverage, probVariance;
DdNode *one;
DdNode *zero;

void prepareSampleShared(cuddAdapter *a) {
    one  = a->getOne();
    zero = a->getZero();
    nheads = a->getFunctionHeads().size();
    exemplar.resize(a->getNumVars());
    if (nheads == 1) // One bdd, has to start with 0 
                     // but because of reorders it may not
       posHead.push_back(0);
    else // Many bdds, will also start with 0 anyway
      for(const auto& head : a->getStringHeads())
          posHead.push_back(a->posOfVar(a->getVarIndex(head)));
    
    // Add one off the limit, to use it with the last head
    posHead.push_back(a->getNumVars());

    
    
    posHead.push_back(a->getLevel(one));
}
void computeAverageAndVariance() {
    probAverage = probAverage / productProbabilities.size();
    for(auto pp : productProbabilities)
        probVariance += (probAverage - pp)*(probAverage - pp);
                                           
    probVariance = probVariance / productProbabilities.size();
    std::cerr << "Probability average " << probAverage
        << " probability variance " << probVariance
            << " probability typical deviation " << sqrt(probVariance) << std::endl;
}

double getAverage() {
    return probAverage.get_d();
}

double getVariance() {
    return probVariance.get_d();
}

bool validateProduct(cuddAdapter* a,
                     const std::vector<bool>& b) {
    DdNode *g1, *g0;
    std::vector<DdNode*>::const_iterator itTrav    = a->getFunctionHeads().begin();
    DdNode* trav;
    one  = a->getOne();
    zero = a->getZero();
    int c = 0;
    while (itTrav != a->getFunctionHeads().end()) {
        //std::cerr << "Head " << c+1;
        
        //if (a->getStringHeads().size() > c)
        //    std::cerr << " name " << std::setw(50) << a->getStringHeads()[c] << std::endl;
        //else
        //    std::cerr << std::endl;
        
        trav = *itTrav;
        while (trav != one && trav != zero) {
            //std::cerr << "Reading variable "
            //    << std::setw(50) << a->getVarName(a->varAtPos(a->getLevel(trav))) << " ("
            //    << a->getLevel(trav) << ") : ";
            cuddGetBranches(trav, &g1, &g0);
            if (b[a->varAtPos(a->getLevel(trav))]) {
                //std::cerr << " 1" << std::endl;
                trav = g1;
            }
            else {
                //std::cerr << " 0" << std::endl;
                trav = g0;
            }
        }
        if (trav == zero)  {
            std::cerr << "Validation failed in head number " << c+1;
            if (a->getStringHeads().size() > c)
                std::cerr << ", " << a->getStringHeads()[c] << std::endl;
            else
                std::cerr << std::endl;
            std::cerr << "Product: ";
            int d = 0;
            for(auto x : b)
                std::cerr << a->getVarName(a->varAtPos(d++)) << ": " << x << " ";
            std::cerr << std::endl;
            return false;
        }
        itTrav++;
        c++;
    }
    return true;
}

std::vector<bool> genRandomComputeProb(cuddAdapter* a) {
    // We assume the probabilities have been computed already
    DdNode *g1, *g0;
    DdNode* trav;
    int pos     = 0;
    mpf_class prodProb = 1;
    std::vector<DdNode*>::const_iterator itTrav    = a->getFunctionHeads().begin();
    std::vector<int>::iterator           itPosHead = posHead.begin();
    //int c = 0;
    while (itTrav != a->getFunctionHeads().end()) {
        trav = *itTrav;
        pos  = *itPosHead;
        itPosHead++;
//        std::cerr << "head " << std::setw(40) << a->getStringHeads()[c++] << " ("
//                        << std::setw(5) <<  pos << ") until " << lastPos-1
//                        << " trav pos " << a->getLevel(trav) << std::endl;

        // Phase one: Use the values that are already decided
        while (a->getLevel(trav) < pos) {
            //std::cerr << "Reading variable " << a->getVarName(a->varAtPos(a->getLevel(trav)));
            // Get children
            cuddGetBranches(trav, &g1, &g0);
            if (exemplar[a->varAtPos(a->getLevel(trav))]) {
                //std::cerr << " 1" << std::endl;
                trav = g1;
            }
            else {
                //std::cerr << " 0" << std::endl;
                trav = g0;
            }
        }
        // Phase two: Use probabilities to decide the variables
        // in this group
        while (trav != one && trav != zero) {
            // Toss a coin if there are jumps. 
            while (pos < a->getLevel(trav)) {
                //std::cerr << "Assigning fifty fifty value to "
                //          << a->getVarName(a->varAtPos(pos));
                exemplar[a->varAtPos(pos++)] = (random() & 1) == 0;
                prodProb /= 2;
                //std::cerr << exemplar[a->varAtPos(pos-1)] << std::endl;
            }
            cuddGetBranches(trav, &g1, &g0);
            //std::cerr << "Assigning random value to " << a->getVarName(a->varAtPos(pos));
            if (random()>>1 < probabilities[trav]) {
                prodProb = prodProb*probabilities[trav]/(double)(RAND_MAX >> 1);
                exemplar[a->varAtPos(pos++)] = true;
                trav = g1;
                //std::cerr << " 1" << std::endl;
             }
             else {
                prodProb = prodProb*((RAND_MAX >> 1) - probabilities[trav])/(double)(RAND_MAX >> 1);
                exemplar[a->varAtPos(pos++)] = false;
                trav = g0;
                //std::cerr << " 0" << std::endl;
             }
        }
        while (pos < a->getLevel(trav)) {
           //std::cerr << "Assigning fifty fifty value to "
           //          << a->getVarName(a->varAtPos(pos));
           exemplar[a->varAtPos(pos++)] = (random() & 1) == 0;
           prodProb /= 2;
            //std::cerr << exemplar[a->varAtPos(pos-1)] << std::endl;
        }
        if (trav == zero) {
            std::ostringstream ost;
            std::cerr << "trav " << trav << std::endl;
            ost << std::endl << "Zero node reached while generating at position "
                << pos << " variable " << a->getVarName(a->varAtPos(pos))
                << " value " << exemplar[a->varAtPos(pos)] << std::endl;
            throw std::logic_error(ost.str());
        }
        itTrav++;
    }
    
    productProbabilities.push_back(prodProb);
    probAverage += prodProb;
    return exemplar;
}
std::vector<bool> genRandomShared(cuddAdapter* a) {
    // We assume the probabilities have been computed already
    DdNode *g1, *g0;
    DdNode* trav;
    int pos     = 0;
    int lastPos = 0;
//    std::cerr << "Poshead: ";
//    for(auto q : posHead)
//        std::cerr << q << " ";
//    std::cerr << std::endl;
    std::vector<DdNode*>::const_iterator itTrav    = a->getFunctionHeads().begin();
    std::vector<int>::iterator           itPosHead = posHead.begin();
    //int c = 0;
    while (itTrav != a->getFunctionHeads().end()) {
        trav = *itTrav;
        pos  = *itPosHead;
        itPosHead++;
        lastPos = *itPosHead;
//        std::cerr << "head " << std::setw(40) << a->getStringHeads()[c++] << " ("
//                    	<< std::setw(5) <<  pos << ") until " << lastPos-1
//                        << " trav pos " << a->getLevel(trav)
//                        << " index " << a->varAtPos(pos) << std::endl;

        // Phase one: Use the values that are already decided
        while (a->getLevel(trav) < pos) {
            //std::cerr << "Reading variable " << a->getVarName(a->varAtPos(a->getLevel(trav)));
            // Get children
            cuddGetBranches(trav, &g1, &g0);
            if (exemplar[a->varAtPos(a->getLevel(trav))]) {
                //std::cerr << " 1" << std::endl;
                trav = g1;
            }
            else {
                //std::cerr << " 0" << std::endl;
                trav = g0;
            }
        }
        // Phase two: Use probabilities to decide the variables
        // in this group
        while (pos < lastPos) {
            // Toss a coin if there are jumps
            while (pos < std::min(a->getLevel(trav), lastPos)) {
                //std::cerr << "Assigning fifty fifty value to "
                //          << a->getVarName(a->varAtPos(pos));
                exemplar[a->varAtPos(pos++)] = (random() & 1) == 0;
                //std::cerr << " " << exemplar[a->varAtPos(pos-1)] << std::endl;
            }
            if (pos < lastPos) {
                cuddGetBranches(trav, &g1, &g0);
                //std::cerr << "Assigning random value to " << a->getVarName(a->varAtPos(pos))
                //<< " with probability " << probabilities[trav]/double(RAND_MAX >>1);
                if (random()>>1 < probabilities[trav]) {
                    trav = g1;
                    exemplar[a->varAtPos(pos++)] = true;
                    //std::cerr << " 1" << std::endl;
                }
                else {
                    exemplar[a->varAtPos(pos++)] = false;
                    trav = g0;
                    //std::cerr << " 0" << std::endl;
                }
            }
        }
        if (trav == zero) {
            std::ostringstream ost;
            ost << std::endl << "Zero node reached while generating at position "
                << pos << " of " << a->getNumVars() << std::endl;
            if (pos >= 0 && pos < a->getNumVars())
               ost << " variable " << a->getVarName(a->varAtPos(pos))
                   << " value " << exemplar[a->varAtPos(pos)] << std::endl;
            throw std::logic_error(ost.str());
        }
        itTrav++;
    }
    return exemplar;
}
