//
//  symbolTable.hpp
//  myKconf
//
//  Created by david on 02/10/14.
//  Copyright (c) 2014 david. All rights reserved.
//

#ifndef __myKconf__symbolTable__
#define __myKconf__symbolTable__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include <unordered_set>
#include <string>
#include <algorithm>

class configInfo;
class synExp;

class symbolTable {
    
  private:
    
    static std::unordered_map<std::string, configInfo*> table;
    static std::map<std::string, int>         mapTimesDeclared;
    static std::set<std::string>              undefined;
    static std::set<std::string>              defined;
  public:
    typedef std::list<configInfo*>::iterator iterator;

    static bool        isTristate(const std::string& name);
    //static synExp*     getValue(const std::string& s);
    //static void        setValue(const std::string& s, synExp* v);
    static bool        isBooleanVar(const std::string& s);
    static iterator    getIterator(const std::string& s);
    static void        clear();
    static int         size();
    static  int        timesDeclared(const std::string& s);
    static  void       declareSymbol(const std::string& s, configInfo* p);
    static  void       addSymbol(const std::string& s, configInfo* p);
    static  void       delUndefined(std::string s);
    static  void       deleteSymbol(const std::string& s);
    static  void       moveToFront(const std::string& sym);
    static  void       moveAfter(const std::string& sym, const std::string& bef);
    static  void       moveCloserTo(const std::string& sym,
                                    const std::set<std::string>& support);
    static void        setDefined(std::string sym);
    static bool        isUndefined(const std::string& sym);
    static configInfo *getSymbol(const std::string& s);
    static std::set<std::string> getUndefined();

    
};

#endif /* defined(__myKconf__symbolTable__) */
