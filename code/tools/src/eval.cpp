//
//  eval.cpp
//  myKconf
//
//  Created by david on 15/10/14.
//  Copyright (c) 2014 david. All rights reserved.
//

#include "eval.hpp"

void TableExp::showTable() {
    for(auto p : tab)
        std::cerr << std::setw(20) << p.first
                  << " (" << (void*) p.first << ") = "
        << p.second.first << " refs " << p.second.second << std::endl;
}

int  TableExp::refs(synExp* s) {
    tabType::iterator it = tab.find(s);
    if (it != tab.end())
        return it->second.first;
    return 0;
}

void TableExp::add(synExp* exp) {
    //std::cerr << "adding " << std::setw(50) << exp << " to table " << std::endl;
    if (exp == synTrue)
        return;
    tabType::iterator it = tab.find(exp);
    
    if (it != tab.end()) {
        tab[exp] = std::pair<int, synExp*>(it->second.first+1, it->second.second);
        return;
    }
    
    tab[exp] = std::pair<int, synExp*>(1, synTrue);
    
    if (exp->isNot()) {
        tab[exp->first()] = std::pair<int, synExp*>(1, synFalse);
    }
    if (exp->isAnd()) {
        add(exp->first());
        add(exp->second());
    }
    if (exp->isLiteral()) {
        if (exp->isSymbol())
            for(auto x : VariableTable::getPositive(exp->getSymbol()))
                add(x);
        else
            for(auto x : VariableTable::getNegative(exp->first()->getSymbol()))
                add(x);
    }
}
void TableExp::del(synExp* exp) {
    //std::cerr << "deleting " << std::setw(50) << exp << " from table " << std::endl;
    tabType::iterator it = tab.find(exp);
    if (it == tab.end()) {
        return;
    }
    if (it->second.first == 1)
        tab.erase(exp);
    else
        tab[exp] = std::pair<int, synExp*>(it->second.first-1,
                                           it->second.second);
    
    if (exp->isNot()) {
        tab.erase(exp->first());
    }
    if (exp->isAnd()) {
        del(exp->first());
        del(exp->second());
    }
    if (exp->isLiteral()) {
        if (exp->isSymbol())
            for(auto x : VariableTable::getPositive(exp->getSymbol()))
                del(x);
        else
            for(auto x : VariableTable::getNegative(exp->first()->getSymbol()))
                del(x);
    }
}

synExp* TableExp::find(synExp* s) {
    for(auto& p : tab)
        if (equal(s, p.first))
            return p.second.second;
    if (s->isLiteral()) {
        std::string var;
        bool negated = false;
        if (s->isSymbol())
            var = s->getSymbol();
        else {
            var = s->first()->getSymbol();
            negated = true;
        }
        
        synExp* val = VariableTable::getValue(var);
        if (val != NULL && val->isConst()) {
            if (!negated)
                return val;
            else
                return makeNot(val->copy_exp());
        }
        
    }
    return NULL;
}

bool eval_recur(synExp*& exp, TableExp& tab) {
    synExp* tabres = tab.find(exp);
    if (tabres != NULL) {
            exp = tabres;
            return true;
        }
    if (exp == NULL) {
        return false;
    }
    switch (exp->get_type()) {
        case synExp_Const :  { return false; }
        case synExp_Symbol : {
            std::string theSymbol    = exp->getSymbol();
            synExp*    value       = VariableTable::getValue(theSymbol);
            if (value == NULL) {
                std::ostringstream ost;
                ost << "WARNING: Symbol " << theSymbol << " not found in variable table" << std::endl;
                value = synFalse;
                //throw std::logic_error(ost.str());
            }
            // We don't simplify string vars with a value bc
            // they make no sense as formulas
            if (exp->isString()) {
                return false;
            }
            if (!equal(value, exp)) {
                std::cerr   << "Symbol " << exp << " different from table value "
                            << value << std::endl;
                exp = value->copy_exp();
                return true;
            }
            return false;
            
        }
        case synExp_Compound :
        {
            synExp* one   = exp->first();
            synExp* two   = exp->second();
            synExp* three = exp->third();
            bool res1, res2, res3;
            bool res = false;
            switch (exp->get_op()) {
                case synNot        : {
                    if (eval_recur(one, tab)) {
                        exp = makeNot(one);
                        return true;
                    }
                    exp = makeNot(one->copy_exp());
                    return false;
                }
                case synAnd        : {
                    std::set<synExp*> ss, newSS;
                    synExp::flattenAnd(exp->copy_exp(), ss);
                    std::unordered_set<synExp*> set;
                    for(auto x : ss) {
                        if (x->isLiteral() && tab.find(x) == NULL) {
                            set.insert(x);
                            tab.add(x);
                        }
                    }
                    std::unordered_map<synExp*, bool> mres;
                    for(auto x: ss)
                        if (set.find(x) == set.end()) {
                            mres[x] = eval_recur(x, tab);
                            newSS.insert(x);
                            res |= mres[x];
                        }
                        else {
                            if (tab.refs(x) > 1) {
                                tab.del(x);
                                set.erase(x);
                                x->destroy();
                            }
                        }
                            
                    for(auto s : set) {
                        tab.del(s);
                        newSS.insert(s);
                    }
                    
                    if (res) {
                        exp = synTrue;
                        for(auto x: newSS)
                            exp = makeAnd(exp, x);
                        
                        return res;
                    }
                    for(auto x : newSS)
                        x->destroy();
                    
                    return res;
                }
                case synOr         : {
                    std::set<synExp*> ss, negSet, newSS;
                    synExp::flattenOr(exp->copy_exp(), ss);
                    std::unordered_map<synExp*, synExp*> toNeg;
                    for(auto x : ss) {
                        if (x->isLiteral() && tab.find(x) == NULL) {
                            synExp* neg = makeNot(x->copy_exp());
                            // Save it to delete it later
                            toNeg[x] = neg;
                            tab.add(neg);
                        }
                    }
                    std::unordered_map<synExp*, bool> mres;
                    for(auto x: ss)
                        if (toNeg.find(x) == toNeg.end()) {
                            mres[x] = eval_recur(x, tab);
                            newSS.insert(x);
                            res |= mres[x];
                        }
                    
                    for(auto p : toNeg) {
                        tab.del(p.second);
                        p.second->destroy();
                        newSS.insert(p.first);
                    }
                    
                    if (res) {
                        exp = synFalse;
                        for(auto x: newSS)
                            exp = makeOr(exp, x);
                        
                        return res;
                    }
                    for(auto x : newSS)
                        x->destroy();
                    
                    return res;

                }
                case synEqual      : {
                    res1 = eval_recur(one, tab);
                    res2 = eval_recur(two, tab);
                    res = res1 | res2;
                    // If one did not change, it has to be copied
                    // so that the original expression can be
                    // safely destroyed later
                    if (res && !res1)  one = one->copy_exp();
                    if (res && !res2)  two = two->copy_exp();
                    if (res)           exp = makeEqual(one, two, true);
                    
                    return res;
                }
                case synImplies    : {
                    res1 = eval_recur(one, tab);
                    tab.add(exp->first());
                    res2 = eval_recur(two, tab);
                    tab.del(exp->first());
                    
                    res = res1 | res2;
                    if (res && !res1)  one = one->copy_exp();
                    if (res && !res2)  two = two->copy_exp();
                    if (res)           exp = makeImplies(one, two);
                    return res;
                }
                case synIfThenElse : {
                    synExp *toDestroy = makeNot(one->copy_exp());
                    res1 = eval_recur(one, tab);
                    tab.add(exp->first());
                    res2 = eval_recur(two, tab);
                    tab.del(exp->first());
                    tab.add(toDestroy);
                    res3 = eval_recur(three, tab);
                    tab.del(toDestroy);
                    res = res1 | res2 | res3;
                    
                    
                    if (res && !res2)  two   = two->copy_exp();
                    if (res && !res3)  three = three->copy_exp();
                    if (res && !res1)  one   = one->copy_exp();
                    toDestroy->destroy();
                    
                    if (two == synTrue) {
                        one = one->copy_exp();
                        exp = makeImplies(makeNot(one->copy_exp()),
                                          three);
                    }
                    else {
                        if (res && !res1)
                            one   = one->copy_exp();
                        exp = makeIfThenElse(one,
                                             two,
                                             three,
                                             true);
                    }
                    return res;
                }
                case synXor: {
                    throw std::logic_error("An XOR cannot happpen here");
                    break;
                }
            }
        }
        case synExp_String : { return false; }
            
        case synExp_XOR : {
            bool res = false;
            std::vector<synExp*> evaluated, newParms;
            for(synExp* s :exp->get_parms()) {
                if (eval_recur(s,tab)) {
                    res = true;
                }
                evaluated.push_back(s);
            }
            int counter = 0;
            for(synExp* ev : evaluated) {
                if (ev == synTrue) {
                    counter++;
                    res = true;
                }
                else
                    if (ev != synFalse) {
                        newParms.push_back(ev->copy_exp());
                    }
            }
            if (counter > 1) {
                exp = synFalse;
                return true;
            }
            if (counter == 1) {
                exp = synTrue;
                for(synExp* ev : newParms) {
                    if (ev != synTrue) {
                        exp = makeAnd(exp, makeNot(ev));
                    }
                }
                return true;
            }
            if (newParms.size() == 1) {
                exp = newParms[0];
                return true;
            }
            if (evaluated.size() == 0) {
                exp = synTrue;
                return true;
            }
            if (res)
                exp = new synXOR(newParms);
            
            return res;
        }
        case synExp_Unknown : break;
        case synExp_Comment : break;
        case synExp_Macro   : break;
    }
    return false;
}

synExp* eval(synExp* exp) {
    TableExp tab;
    // Repeat while anything changes
    synExp *old = exp;
    while (eval_recur(exp, tab)) {
        old->destroy();
        old = exp;
    }    
   return exp;
}

