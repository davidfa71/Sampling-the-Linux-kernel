//
//  Dictionary.hpp
//  backValidation
//
//  Created by David Fernandez Amoros on 10/4/21.
//  Copyright © 2021 David Fernandez Amoros. All rights reserved.
//

#ifndef Dictionary_hpp
#define Dictionary_hpp

#include <stdio.h>
#include <iostream>
#include <VarData.hpp>
#include <KeyValue.hpp>
#include <cuddAdapter.hpp>

class Dictionary {
public:
    VarData* get(std::string name);
    std::vector<KeyValue> decode(const tProduct& product,
                                       std::string& environ);
    tProduct encode(cuddAdapter* a,
                    std::map<std::string, std::string>& config);
    void gatherVariableInfo(cuddAdapter* adapter);
protected:
    std::map<std::string, VarData*> dict;
    std::string checkType(std::string& var,
                          std::string t);
    void checkAlias(std::string& read);
    void deal_bitmapint(std::string& read,
                        int i);
    void deal_bitmaphex(std::string& read,
                        int i);

};
    
#endif /* Dictionary_hpp */
