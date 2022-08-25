//
//  VariableTable.hpp
//  Kconfig2Logic
//
//  Created by David Fernandez Amoros on 11/03/2021.
//  Copyright © 2021 David Fernandez Amoros. All rights reserved.
//

#ifndef VariableTable_hpp
#define VariableTable_hpp

#include <stdio.h>
#include <map>

#include "VariableEntry.hpp"
class VariableTable {
public:
    static void         addVariable(std::string variable,
                                    std::string originalConfig);
    static std::string  getOriginalConfig(std::string variable);
    static synExp*      getValue(std::string variable);
    static void         setValue(std::string variable,
                                 synExp* value);
    static void         destroyTable();
    static std::set<std::string> getVariables();
    static bool         isDefined(std::string var);
    
    static void         add2Positive(std::string s,
                                     synExp* exp);
    static void         add2Negative(std::string s,
                                     synExp* exp);
    static void         del2Positive(std::string s,
                                     synExp* exp);
    static void         del2Negative(std::string s,
                                     synExp* exp);
    static std::set<synExp*>& getPositive(const std::string& s);
    static std::set<synExp*>& getNegative(const std::string& s);

protected:
    
    static std::unordered_map<std::string, VariableEntry*>           theMap;
    static std::set<synExp*>                                         emptySet;
    static std::unordered_map<std::string, std::set<synExp*>>        positive;
    static std::unordered_map<std::string, std::set<synExp*>>        negative;
};
#endif /* VariableTable_hpp */
