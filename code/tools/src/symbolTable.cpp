//
//  symbolTable.cpp
//  myKconf
//
//  Created by david on 02/10/14.
//  Copyright (c) 2014 david. All rights reserved.
//

#include "symbolTable.hpp"
#include "configInfo.hpp"

std::unordered_map<std::string, configInfo*> symbolTable::table;
std::map<std::string, int>         symbolTable::mapTimesDeclared;
std::set<std::string>              symbolTable::undefined;
std::set<std::string>              symbolTable::defined;

int                      symbolTable::size()  { return int(table.size());  }
std::set<std::string> symbolTable::getUndefined() { return undefined;}

bool symbolTable::isTristate(const std::string& name) {
    return configInfo::getVartype(name) == "tristate";
}

bool symbolTable::isBooleanVar(const std::string& s) {
    return configInfo::getVartype(s) == "boolean";
}

int     symbolTable::timesDeclared(const std::string& s) {
    return mapTimesDeclared[s];
}

void symbolTable::clear() {
    for(auto p : table) {
        p.second->freeDefaults();
        p.second->destroyAlternatives();
    }
        
    table.clear();
    mapTimesDeclared.clear();
    undefined.clear();
}
void symbolTable::declareSymbol(const std::string& s, configInfo* ci) {
    ci->setHasConfig();
    mapTimesDeclared[s]++;
    if (undefined.find(ci->getName()) != undefined.end())
        undefined.erase(ci->getName());
    
    // Always do the assignment because the symbol might have been added but not defined
    // and it is important that it is defined
    if (table.find(s) != table.end() &&
        !table[s]->getHasConfig()) {
        delete table[s];
    }
    table[s] = ci;
}
void symbolTable::delUndefined(std::string s) {
    undefined.erase(s);
}
void symbolTable::addSymbol(const std::string& s,configInfo* ci) {
    if (table.find(s) == table.end()) {
        if (defined.find(s) == defined.end())
            undefined.insert(s);
    }
    else
        delete table[s];
    
    table[s] = ci;
}
configInfo *symbolTable::getSymbol(const std::string& s) {
    if (table.find(s) != table.end()) {
        return table[s];
    }
    return NULL;
}

//synExp* symbolTable::getValue(const std::string& s) {
//    if (table.find(s) != table.end()) {
//        return table[s]->getValue();
//    }
//    return NULL;
//}

//void symbolTable::setValue(const std::string& s, synExp* v) {
//    if (table.find(s) != table.end()) {
//        return table[s]->setValue(v);
//    }
//    if (s.length() > 2 && s[s.length()-2] == '_' && s[s.length()-1] == 'M') {
//        std::string tristateName = std::string(s, 0, s.length()-2);
//        if (table.find(tristateName) != table.end())
//            return table[tristateName]->setValueModule(v);
//    }
//}

void    symbolTable::deleteSymbol(const std::string& s) {
    //std::cerr << "Deleting symbol " << s << std::endl;

    configInfo* ci = getSymbol(s);
    delete(ci);
    table.erase(s);
}

void symbolTable::setDefined(std::string sym) {
    defined.insert(sym);
}

bool symbolTable::isUndefined(const std::string& sym) {
    return undefined.find(sym) != undefined.end();
}
