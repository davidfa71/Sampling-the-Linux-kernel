//
//  smartspanSorter.cpp
//  myKconf
//
//  Created by David on 05/06/15.
//  Copyright (c) 2015 david. All rights reserved.
//

#include "smartspanSorter.hpp"

int smartspanSorter::computeSmartSpan(const synExp* x, const std::vector<int>& myvar2pos) {
    if ((x->isOr() || x->isImplies()) && x->first()->isLiteral() && x->second()->isLiteral()) {
        int one = x->getMin();
        int two = x->getMax();
        int rem = -1;
        int firstLit;
        synExp* l;
        int polarity;
        if (getVarNum(x->first()) < getVarNum(x->second())) {
            l = x->first();
            polarity = -1;
        }
        else {
            l = x->second();
            polarity = 1;
        }
        if (x->isOr())
            firstLit = getLitNum(l);
        else
            firstLit = polarity*getLitNum(l);
        
        if (remember.find(firstLit) != remember.end()) {
            for(auto p : remember[firstLit])
                if (p > one) {
                    if (p < two)
                        rem = p;
                    else break;
                }
        }
        
        if (rem != -1) {
            std::cerr << "ZZZ Computing smart span of " << (synExp*) x << ", one " << one << " two " << two
                << " firstLit " << firstLit << " rem " << rem << " span " << two - rem +1  << std::endl;
            return two - rem +1;
        }
        else {
            std::cerr << "ZZZ Computing smart span of " << (synExp*) x << ", one " << one << " two " << two
                << " firstLit " << firstLit << " rem " << rem << " span " << two - one  << std::endl;
           return  two - one;
        }
    }
    return x->getMax() - x->getMin();
}
