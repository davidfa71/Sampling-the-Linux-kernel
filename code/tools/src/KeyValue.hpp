//
//  KeyValue.hpp
//  backValidation
//
//  Created by David Fernandez Amoros on 10/4/21.
//  Copyright © 2021 David Fernandez Amoros. All rights reserved.
//

#ifndef KeyValue_hpp
#define KeyValue_hpp

#include <stdio.h>
#include <string>

class KeyValue {
public:
    KeyValue(std::string key,
            std::string value);
    std::string getKey();
    std::string getValue();
protected:
    std::string key;
    std::string value;
};


#endif /* KeyValue_hpp */
