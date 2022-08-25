//
//  Dictionary.cpp
//  backValidation
//
//  Created by David Fernandez Amoros on 10/4/21.
//  Copyright © 2021 David Fernandez Amoros. All rights reserved.
//

#include "Dictionary.hpp"

bool startsWith(std::string st, std::string start) {
    return (st.rfind(start, 0) == 0);
}

std::string remove(std::string st, std::string start) {
    size_t pos;
    if ((pos = st.find(start)) != std::string::npos) {
        std::string res = st.substr(0, pos);
        res += st.substr(pos+start.length(), st.length() - start.length());
        return res;
    }
    return st;
}

tProduct Dictionary::encode(cuddAdapter* a,
                            std::map<std::string, std::string>& config) {
    tProduct product;
    product.resize(a->getNumVars());
    for(auto& p : dict) {
        std::string key   = p.first;
        VarData*    vd    = p.second;
        std::string value;
        if (vd->getType() == "boolean") {
            if (config.find(key) == config.end())
                product[vd->getValue()] = false;
            else
                if (config[key] == "y")
                    product[vd->getValue()] = true;
                else
                    if (config[key] == "n")
                        product[vd->getValue()] = false;
                    else {
                        throw std::logic_error("Wrong value "+config[key]+" for boolean "+key);
                    }
        }
        else
          if (vd->getType() == "tristate") {
              if (config.find(key) == config.end()) {
                  product[vd->getValue()] = false;
                  product[vd->getModule()] = false;

              }
              else
                  if (config[key] == "y") {
                      product[vd->getValue()]  = true;
                      product[vd->getModule()] = false;
                  }
              else
                  if (config[key] == "n") {
                      product[vd->getValue()]  = false;
                      product[vd->getModule()] = false;
                  }
              else
              if (config[key] == "m") {
                  product[vd->getValue()]  = false;
                  product[vd->getModule()] = true;
              }
              else
                  throw std::logic_error("Wrong value "+config[key]+" for tristate "+key);
          }
        else
            if (vd->getType() == "string" ||
                vd->getType() == "int"    ||
                vd->getType() == "hex") {
                if (config.find(key) == config.end()) {
                    // All are false
                    for(auto v : vd->getValues())
                        product[v.second] = false;
                }
                else {
                    if (vd->getType() == "hex" && !startsWith(config[key], "0x"))
                        config[key] = "0x"+config[key];
                    if (vd->getValues().find(config[key]) != vd->getValues().end())
                        product[vd->getValues()[config[key]]] = true;
                    else {
                        if (vd->getValues().find("OTHERVAL") != vd->getValues().end())
                            product[vd->getValues()["OTHERVAL"]] = true;
                        //else
                        //std::ostringstream ost;
                        //ost << "config " << key << " has value " << config[key]
                        //    <<  " but that is not in the BDD. Options are: ";
                        //for(auto p : vd->getValues())
                        //    ost << p.first << ", ";
                        //throw std::logic_error(ost.str());
                    }
                }
            }
        else
            if (vd->getType() == "BITMAP_int") {
                if (config.find(key) == config.end()) {
                  if (vd->hasMaster())
                      product[vd->getMasterValue()] = false;
                  if (vd->hasSign())
                      product[vd->getSignValue()] = false;
                    for(int i = 0; i < vd->getMaxBit(); i++)
                        product[vd->getBit(i)] = false;
                }
                else {
                    if (vd->hasMaster())
                        product[vd->getMasterValue()] = true;
                    long num;
                    if (config[key] == "OTHERVAL") {
                          if (vd->hasMaster())
                              product[vd->getMasterValue()] = true;
                        num = random();
                        if (vd->hasSign()) {
                            if (num % 2 == 1) {
                                product[vd->getSignValue()] = true;
                                num = num >> 1;
                                num = -num;
                            }
                            else
                                product[vd->getSignValue()] = false;
                        }
                    }
                    else
                        num = std::stoi(config[key]);
                    if (num < 0 && vd->hasSign()) {
                        product[vd->getSignValue()] = true;
                        num = -num;
                    }
                        
                    int bit = 0;
                    while (bit < vd->getMaxBit()) {
                        if (num % 2 == 0)
                            product[vd->getBit(bit)] = false;
                        else
                            product[vd->getBit(bit)] = true;
                        num = num >> 1;
                        bit++;
                    }
                    
                }
            }
            else
                if (vd->getType() == "BITMAP_hex") {
                    if (config.find(key) == config.end()) {
                      if (vd->hasMaster())
                          product[vd->getMasterValue()] = false;
                      if (vd->hasSign())
                          product[vd->getSignValue()] = false;
                        for(int i = 0; i < vd->getMaxBit(); i++)
                            product[vd->getBit(i)] = false;
                    }
                    else {
                        if (vd->hasMaster())
                            product[vd->getMasterValue()] = true;
                        int num;
                        if (config[key] == "OTHERVAL")
                            num = random();
                        else
                            num = std::stoi(config[key], NULL, 16);
                        if (num < 0 && vd->hasSign()) {
                            product[vd->getSignValue()] = true;
                            num = -num;
                        }
                        int bit = 0;
                        while (bit < vd->getMaxBit()) {
                            if (num % 2 == 0)
                                product[vd->getBit(bit)] = false;
                            else
                                product[vd->getBit(bit)] = true;
                            num = num >> 1;
                            bit++;
                        }
                        
                    }
                }
                
    }
    
    return product;
}



VarData* Dictionary::get(std::string name) {
    if (dict.find(name) != dict.end())
        return dict[name];
    else {
        dict[name] = new VarData(name);
        return dict[name];
    }
}

std::string Dictionary::checkType(std::string& var,
                                  std::string t) {
    if (startsWith(var, t+"_")) {
        var = remove(var, t+"_");
        return t;
    }
    return "";
}

void Dictionary::deal_bitmapint(std::string& read, int i) {
    std::string type = "BITMAP_int";
    read = remove(read, "BITMAP_int_");
    if (startsWith(read, "BITMAP_INT_MASTER_")) {
        read = remove(read, "BITMAP_INT_MASTER_");
        checkAlias(read);
        get(read)->setMaster(i);
        return;
    }
    if (startsWith(read, "SIGN_")) {
        read = remove(read, "SIGN_");
        checkAlias(read);
        get(read)->setSign(i);
        return;
    }
    if (startsWith(read, "BIT_")) {
        read = remove(read, "BIT_");
        size_t p = read.find_first_of('_');
        int bit = std::atoi(read.substr(0, p).c_str());
        read = read.substr(p+1, read.length()-p-1);
        checkAlias(read);
        get(read)->setBit(bit, i);
    }
}

void Dictionary::deal_bitmaphex(std::string& read, int i)
{
    std::string type = "BITMAP_hex";
    read = remove(read, "BITMAP_hex_");
    if (startsWith(read, "BITMAP_HEX_MASTER_")) {
        read = remove(read, "BITMAP_HEX_MASTER_");
        checkAlias(read);
        get(read)->setMaster(i);
        return;
    }
    if (startsWith(read, "SIGN_")) {
        read = remove(read, "SIGN_");
        checkAlias(read);
        get(read)->setSign(i);
        return;
    }
    if (startsWith(read, "BIT_")) {
        read = remove(read, "BIT_");
        size_t p = read.find_first_of('_');
        int bit = std::atoi(read.substr(0, p).c_str());
        read = read.substr(p+1, read.length()-p-1);
        checkAlias(read);
        get(read)->setBit(bit, i);
    }
}
void Dictionary::checkAlias(std::string& read) {
    size_t s = read.find("_ALIAS_");
    if (s != std::string::npos) {
        std::string alias = read.substr(s+7, read.length() - s - 7);
        read = read.substr(0, s);
        get(read)->setAlias(alias);
    }
}
void Dictionary::gatherVariableInfo(cuddAdapter* adapter) {

    
    for(int i = 0; i < adapter->getNumVars(); i++) {
        bool env = false;
        std::string type;
        std::string read = adapter->getVarName(i);
        std::string originally = read;
        if (startsWith(read, "ENV_")) {
            env  = true;
            read = remove(read, "ENV_");

        }
        if ((type = checkType(read, "boolean")) != "") {
            checkAlias(read);
            get(read)->setValue(i);
        }
        else
            if ((type = checkType(read, "tristate")) != "") {
                size_t p = read.rfind("_M");
                if (read.length() > 2 && p == read.length()-2) {
                    read = read.substr(0, read.length()-2);
                    checkAlias(read);
                    get(read)->setModule(i);
                }
                else {
                    checkAlias(read);
                    get(read)->setValue(i);
                }
            }
            else
                if ((type = checkType(read, "string")) != "") {
                    size_t p;
                    if ((p = read.find("_EQ_")) != std::string::npos) {
                        std::string value = read.substr(p+4, read.length()-p-4);
                        read = read.substr(0, p);
                        checkAlias(read);
                        get(read)->setValue(value, i);
                    }
                }
                else
                    if ((type = checkType(read, "int")) != "") {
                        size_t p;
                        if ((p = read.find("_EQ_")) != std::string::npos) {
                            std::string value = read.substr(p+4, read.length()-p-4);
                            read = read.substr(0, p);
                            checkAlias(read);
                            get(read)->setValue(value, i);
                        }
                    }
                    else
                        if ((type = checkType(read, "hex")) != "") {
                            size_t p;
                            if ((p = read.find("_EQ_")) != std::string::npos) {
                                std::string value = read.substr(p+4, read.length()-p-4);
                                read = read.substr(0, p);
                                checkAlias(read);
                                get(read)->setValue(value, i);
                            }
                        }
                        else
                            // It is a BITMAP
                            if (read.find("BITMAP_int_") != std::string::npos) {
                                type = "BITMAP_int";
                                deal_bitmapint(read, i);
                            }
                            else
                                if (read.find("BITMAP_hex_") != std::string::npos) {
                                    type = "BITMAP_hex";
                                    deal_bitmaphex(read, i);
                                }
        if (type == "") {
            throw std::logic_error("variable "+read+ ", originally "+originally+" has no type");
        }
        get(read)->setType(type);
        if (env)
            get(read)->setEnv();
    }
}

std::vector<KeyValue> Dictionary::decode(const tProduct& product,
                                         std::string& environ) {
std::vector<KeyValue> toWrite;
std::set<std::string> keys;
    for(auto& p : dict) {
        std::string key   = p.first;
        std::string value;
        VarData*    vd  = p.second;
        if (vd->getType() == "boolean") {
            if (product[vd->getValue()])
                value = "y";
            // YOU HAVE TO WRITE THIS OR BACKVALIDATION WON'T WORK
            else
                value = "n";
        }
        else
            if (vd->getType() == "tristate") {
                bool val  = product[vd->getValue()];
                bool mval = product[vd->getModule()];
                
                if (val && !mval)
                    value = "y";
                else
                    if (!val && mval)
                        value = "m";
                    // Write nothing to .config
                    //else
                    //    if (!val && !mval)
                    //        value = "n";
            }
            else
                if (vd->getType() == "string" ||
                    vd->getType() == "int"    ||
                    vd->getType() == "hex") {
                    // Look for value
                    bool firstTime = true;
                    std::string firstValue;
                    for(auto& v : vd->getValues()) {
                        if (product.at(v.second)) {
                            if (firstTime) {
                                firstValue = v.first;
                                firstTime  = false;
                            }
                            else {
                                std::ostringstream ost;
                                ost << "Two values for variable " << vd->getName();
                                ost << ": "<< firstValue << " and " << v.first << std::endl;
                                throw std::logic_error(ost.str());
                            }
                            // Generates a random numerical value. 
                            if ((vd->getType() == "int" ||
                                 vd->getType() == "hex") && v.first == "OTHERVAL") {
                                if (vd->getType() == "hex") {
                                    bool right = false;
                                    int v;
                                    while (!right) {
                                        v = random();
                                        if (v % 2 == 1) {
                                            v = v >> 1;
                                            v = -v;
                                        }
                                        std::stringstream stream;
                                        stream << std::hex << v;
                                        value = "0x"+stream.str();
                                        right = vd->getValues().find(value) == vd->getValues().end();
                                        //std::cerr << "1 Generated " << value
                                        //        << " for " << vd->getName()
                                        //        << ", which is " << right << std::endl;
                                    }
                                }
                                else {
                                    bool right = false;
                                    while (!right) {
                                        value = std::to_string(random());
                                        right = vd->getValues().find(value) == vd->getValues().end();
                                        //std::cerr << "2 Generated " << value
                                        //<< " for " << vd->getName() << ", which is " << right << std::endl;
                                    }
                                }
                            }
                            else // string
                                value = v.first;
                            //break;
                        }
                    }

			
	            // It's OK for a string to have no value. Nothing needs to be output, as conf would
                    //if (firstValue == "")
                    //    std::cerr << "No value for " << key << std::endl;
                }
                else
                    if (vd->getType() == "BITMAP_int") {
                        if (vd->getMaster(product)) {
                            long num = 0;
                            for(int x = vd->getMaxBit()-1; x >= 0; x--) {
                                num *= 2;
                                if (product[vd->getBit(x)])
                                    num++;
                            }
                            if (num < 0)
                                throw std::logic_error("num is negative ");
                            if (vd->getSign(product))
                                num = -num;
                            
                            value = std::to_string(num);
                        }
                    }
                    else
                        if (vd->getType() == "BITMAP_hex") {
                            if (vd->getMaster(product)) {
                                int num = 0;
                                for(int x = vd->getMaxBit()-1; x >= 0; x--) {
                                    num *= 2;
                                    if (product[vd->getBit(x)])
                                        num++;
                                }
                                if (vd->getSign(product))
                                    num = -num;
                                std::ostringstream ost;
                                ost << "0x" << std::hex << num;
                                value = ost.str();
                            }
                        }
        
        if (value != "") {
            if (value == "EMPTY")
                value = "";
            if (keys.find(key) != keys.end())
                throw std::logic_error("Repeated key "+key+ " in decode");
            keys.insert(key);
            if (vd->getEnv())
                environ += " "+vd->getAlias()+"=\""+value+"\"";
            else { // Environment variables are not written to the file
                if (vd->getType() == "string")
                    toWrite.push_back(KeyValue(key, "\""+value+"\""));
                else
                    toWrite.push_back(KeyValue(key, value));
            }
        }
	// It's OK if there is no value for strings and the like. Nothing to output here.
    // Also bool and tristate write nothing to .config when value is n
       // else {
       //     std::ostringstream ost;
       //     ost << "No value produced for key " << key         << std::endl;
       //     ost << "VarData:" << std::endl      << *(p.second) << std::endl;
       //     throw std::logic_error(ost.str());
       // }
    }
    return toWrite;
}
