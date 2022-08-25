//
//  smartspanSorter.h
//  myKconf
//
//  Created by David on 05/06/15.
//  Copyright (c) 2015 david. All rights reserved.
//

#ifndef __myKconf__smartspanSorter__
#define __myKconf__smartspanSorter__

#include <stdio.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>

#include "constraintSorter.hpp"
#include "synExp.hpp"


class smartspanSorter : public constraintSorter {
private:
    // Maps a literal to a set of variables
    std::unordered_map<int, std::unordered_set<int>> bff;
    // Maps a literal to a set of positions
    std::unordered_map<int, std::set<int>> remember;
    
public:
    int getLitNum(synExp* x) {
        if (x->isSymbol())
            return adapter->getVarIndex(x->getSymbol());
        
        return -adapter->getVarIndex(x->first()->getSymbol());
    }
    int getVarNum(synExp* x) {
        if (x->isSymbol())
            return adapter->getVarIndex(x->getSymbol());
        
        return adapter->getVarIndex(x->first()->getSymbol());
    }
    smartspanSorter(cuddAdapter* a,
                    std::vector<synExp*>& c) : constraintSorter(a, c) {
        int one, two;
        for(auto s : c)
            if ((s->isOr() || s->isImplies()) && s->first()->isLiteral() && s->second()->isLiteral()) {
                one = getLitNum(s->first());
                if (s->isImplies())
                    one = -one;
                two = getLitNum(s->second());
                bff[one].insert(abs(two));
                bff[two].insert(abs(one));
            }
    }

    int computeSmartSpan(const synExp* x, const std::vector<int>& myvar2pos);
                                                   
    bool operator()(const synExp *a, const synExp *b) {
       // return a->getMax() - a->getMin() < b->getMax() - b->getMin();
        int c, d, sa, sb;

        sa = a->getSmartSpan();
        sb = b->getSmartSpan();
        
        c = a->getMax();
        d = b->getMax();

        if ( sa != sb )
            return sa < sb;
        else
            return c > d;
    }
    
    void reorderConstraints(std::vector<synExp*>::iterator beg,
                            std::vector<synExp*>::iterator end,
                            bool lastOrder) {
        if (lastOrder) {
            const std::vector<int>& myvar2pos = adapter->var2pos();
            remember.clear();
            for(auto p : bff) {
                for(auto v : p.second)
                    remember[p.first].insert(myvar2pos[v]);
            }
            // Now that we have the order, we perform some operations on the expressions
            for(std::vector<synExp*>::iterator t = beg; t != end; t++) {
                (*t)->computeMaxMin(myvar2pos);
                (*t)->setSmartSpan(computeSmartSpan(*t, myvar2pos));
            }
            std::sort(beg, end, *this);
        }
    }

    
    std::string giveInfo(synExp* s) {
        std::ostringstream ost;
        ost << "[" << std::setw(5) << s->getSmartSpan();
        ost << " " << std::setw(4) << adapter->getNumComponents();
        ost << " " << std::setw(4) << adapter->getMaxComponent();
        ost << "]";
        return ost.str();
    }
    

};
#endif /* defined(__myKconf__smartspanSorter__) */
