//
//  VarData.cpp
//  backValidation
//
//  Created by David Fernandez Amoros on 10/4/21.
//  Copyright © 2021 David Fernandez Amoros. All rights reserved.
//

#include "VarData.hpp"

VarData::VarData(std::string name) : name(name), sign(false), master(false), maxBit(0), env(false) {
    bitValues.resize(32);
};

void VarData::setAlias(std::string alias) {
    this->alias = alias;
}

std::string  VarData::getAlias() {
    return alias;
}

void VarData::setType(std::string type) {
    this->type = type;
};

void  VarData::setSign(int val)   { sign = true; signValue   = val; }
void  VarData::setModule(int val) { moduleValue = val; }

void VarData::setBit(int bit, int pos) {
    bitValues[bit] = pos;
    if (bit >= maxBit)
        maxBit = bit+1;
}

int VarData::getBit(int bit) {
    return bitValues[bit];
}

int  VarData::getValue() {
    return value;
};

std::string VarData::getName() { return name; }

bool VarData::getSign(const tProduct& prod) {
    if (!sign)
        return false;
    return prod[signValue];
}
bool VarData::hasMaster() { return master; }
bool VarData::hasSign()   { return sign;   }
bool VarData::getMaster(const tProduct& prod) {
    if (!master)
        return true;
    return prod[masterValue];
}

int VarData::getMasterValue() {
    return masterValue;
}
int  VarData::getSignValue() {
    return signValue;
}

void VarData::setMaster(int i) {
    master = true;
    masterValue = i;
}

std::string VarData::getType() {
    return type;
}

std::map<std::string, int>& VarData::getValues() {
    return values;
}

int VarData::getMaxBit() {
    return maxBit;
}

int VarData::getModule() {
    return moduleValue;
}

void VarData::setValue(std::string s, int i) {
    values[s] = i;
}

void VarData::setValue(int i) { value = i; }

void VarData::setEnv() { env = true; }
bool VarData::getEnv() { return env; }

std::ostream& operator<<(std::ostream& os, const VarData& vd) {
    std::cerr << "Name:        " << vd.name        << std::endl;
    std::cerr << "Type:        " << vd.type        << std::endl;
    std::cerr << "Value:       " << vd.value       << std::endl;
    std::cerr << "Env:         " << vd.env         << std::endl;
    std::cerr << "Sign:        " << vd.sign        << std::endl;
    std::cerr << "Master:      " << vd.master      << std::endl;
    std::cerr << "MaxBit:      " << vd.maxBit      << std::endl;
    std::cerr << "SignValue:   " << vd.signValue   << std::endl;
    std::cerr << "MasterValue: " << vd.masterValue << std::endl;
    std::cerr << "ModuleValue: " << vd.moduleValue << std::endl;
    
    for(auto& p: vd.values) 
      std::cerr << "value : " << std::setw(60) << p.first << " val " << p.second << std::endl;    
    return os;
}

