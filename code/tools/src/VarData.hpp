//
//  VarData.hpp
//  backValidation
//
//  Created by David Fernandez Amoros on 10/4/21.
//  Copyright © 2021 David Fernandez Amoros. All rights reserved.
//

#ifndef VarData_hpp
#define VarData_hpp

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>

typedef std::vector<bool> tProduct;

class VarData {
public:
    VarData(std::string name);
    void setAlias(std::string alias);
    void setType(std::string type);
    void setSign(int val);
    void setModule(int val);
    
    void setBit(int bit, int pos);
    std::string getName();
    std::string getAlias();
    std::string getType();
    bool getSign(const tProduct& prod);
    
    bool getMaster(const tProduct& prod);
    bool hasMaster();
    int  getMasterValue();
    bool hasSign();
    int  getSignValue();

    void setMaster(int i);
    
    int getMaxBit();
    int getBit(int bit);
    int getModule();
    std::map<std::string, int>& getValues();
    void setValue(std::string s, int i);
    void setValue(int i);
    int  getValue();
    void setEnv();
    bool getEnv();
    friend std::ostream& operator<<(std::ostream& os, const VarData& vd);

protected:
    bool env;
    std::string name;
    std::string alias;
    std::string type;
    bool sign;
    bool master;
    int maxBit;
    std::vector<int> bitValues;
    std::map<std::string, int> values;
    int signValue;
    int masterValue;
    int value;
    int moduleValue;
};

#endif /* VarData_hpp */
