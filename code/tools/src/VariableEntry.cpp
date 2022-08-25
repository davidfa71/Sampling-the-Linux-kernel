//
//  VariableEntry.cpp
//  Kconfig2Logic
//
//  Created by David Fernandez Amoros on 11/03/2021.
//  Copyright © 2021 David Fernandez Amoros. All rights reserved.
//

#include "VariableEntry.hpp"

VariableEntry::VariableEntry(std::string originalConfig, synExp* value) :
    originalConfig(originalConfig), value(value) {}
std::string VariableEntry::getOriginalConfig()   { return originalConfig; }
synExp*     VariableEntry::getValue()            { return value;          }
void        VariableEntry::setValue(synExp* val) {
        value = val;           }
