//
//  eval.h
//  myKconf
//
//  Created by david on 15/10/14.
//  Copyright (c) 2014 david. All rights reserved.
//

#ifndef __myKconf__eval__
#define __myKconf__eval__

#include <stdio.h>

#include "synExp.hpp"
#include "VariableTable.hpp"


class TableExp {
    
typedef std::unordered_map<synExp*, std::pair<int,synExp*>> tabType;

    
public:
    void    showTable();
    void    add(synExp* s);
    void    del(synExp* s);
    int     refs(synExp* s);
    synExp* find(synExp* s);
    
private:
     tabType tab;
};

synExp* eval(synExp* exp);

#endif /* defined(__myKconf__eval__) */
