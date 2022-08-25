//
//  KeyValue.cpp
//  backValidation
//
//  Created by David Fernandez Amoros on 10/4/21.
//  Copyright © 2021 David Fernandez Amoros. All rights reserved.
//

#include "KeyValue.hpp"

KeyValue::KeyValue(std::string key,
                   std::string value) : key(key), value(value) {};

std::string KeyValue::getKey()   { return key;  }
std::string KeyValue::getValue() { return value;}
