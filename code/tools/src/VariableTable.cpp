//
//  VariableTable.cpp
//  Kconfig2Logic
//
//  Created by David Fernandez Amoros on 11/03/2021.
//  Copyright © 2021 David Fernandez Amoros. All rights reserved.
//


#include "VariableTable.hpp"

std::unordered_map<std::string, VariableEntry*>           VariableTable::theMap;
std::set<synExp*>                                         VariableTable::emptySet;
std::unordered_map<std::string, std::set<synExp*>>        VariableTable::positive;
std::unordered_map<std::string, std::set<synExp*>>        VariableTable::negative;

bool         VariableTable::isDefined(std::string var) {
    return theMap.find(var) != theMap.end();
}
void         VariableTable::addVariable(std::string variable,
                                        std::string originalConfig) {
    //std::cerr << "VariableTable adding " << variable << " orig " << originalConfig << std::endl;
    if (theMap.find(variable) != theMap.end())
        delete theMap[variable];
    theMap[variable] = new VariableEntry(originalConfig,
                                         new synSymbol(variable));
}
std::string  VariableTable::getOriginalConfig(std::string variable) {
    if (theMap.find(variable) != theMap.end())
        return theMap[variable]->getOriginalConfig();
    else {// It is an original string or bitmap config or a choice
        return variable;
    }
}
synExp*      VariableTable::getValue(std::string variable) {
    if (theMap.find(variable) != theMap.end())
        return theMap[variable]->getValue();
    else
        return NULL;
}
void         VariableTable::setValue(std::string variable,
                                     synExp* value) {
    if (theMap.find(variable) != theMap.end()) {
        if ((theMap[variable]->getValue() == synTrue && value == synFalse) ||
            (theMap[variable]->getValue() == synFalse && value == synTrue))
            throw std::logic_error(variable+" is true and false at the same time");
    }
    std::cerr << "Setting " << variable << " to " << value << " in variable table " << std::endl;
    try {
      theMap.at(variable)->setValue(value);
    }
    catch (std::exception e) {
      std::ostringstream ost;
      ost << "Variable " << variable << " was not found at theMap " << std::endl;
      throw std::logic_error(ost.str());
    }
} 
void  VariableTable::destroyTable() {
    for(auto p : theMap) {
        
        p.second->getValue()->destroy();
    }
}

std::set<std::string>   VariableTable::getVariables() {
    std::set<std::string> res;
    for(auto p : theMap)
        res.insert(p.first);
    return res;
}

void   VariableTable::add2Positive(std::string s,
                                   synExp* exp) {
    bool found = false;
    for(auto x : positive[s])
        if (equal(x, exp)) {
            found = true;
            break;
        }
    if (!found)
        positive[s].insert(exp);
}
void   VariableTable::add2Negative(std::string s,
                                   synExp* exp) {
    bool found = false;
       for(auto x : negative[s])
           if (equal(x, exp)) {
               found = true;
               break;
           }
       if (!found)
           negative[s].insert(exp);
    
}
void  VariableTable::del2Positive(std::string s,
                                  synExp* exp) {
    for(auto x : positive[s])
        if (equal(x, exp)) {
            positive[s].erase(x);
            return;
        }
}
void   VariableTable::del2Negative(std::string s,
                                   synExp* exp) {
    for(auto x : negative[s])
        if (equal(x, exp)) {
            negative[s].erase(x);
            return;
        }
    
}
std::set<synExp*>& VariableTable::getPositive(const std::string& s) {
    std::unordered_map<std::string, std::set<synExp*>>::iterator
    itm = positive.find(s);
    if (itm != positive.end())
        return itm->second;
    
    return emptySet;
}
std::set<synExp*>& VariableTable::getNegative(const std::string& s) {
    std::unordered_map<std::string, std::set<synExp*>>::iterator
    itm = negative.find(s);
    if (itm != negative.end())
        return itm->second;
    
    return emptySet;
}

