//
//  MultiComponents.hpp
//  myKconf
//
//  Created by David on 01/12/15.
//  Copyright ?? 2015 david. All rights reserved.
//

#ifndef MultiComponents_hpp
#define MultiComponents_hpp

#include <stdio.h>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <sstream>
#include <cmath>

#include "synExp.hpp"
#include "humanNums.hpp"
#include "orderHelper.hpp"
#include "Components.hpp"

class MultiComponents : public Components {
    
    public  :
    
    MultiComponents();
    MultiComponents(std::vector<int>& var2pos, std::vector<int>& pos2var);
    void init(int num);
    void changeOrder(std::vector<int>  pos2var);
    std::vector<int>  getOrder();
    void newVariable();
    
    //void                                        replayConstraints();
    int            getComponent(int x);
    int          getStart(int x);
    int      getCompSize(int x);
    int         getNumComponents();
    int            getMaxLength();
    void                                        reorder();
    bool                                        isOutOfSync();
    void                                        sync();
    bool                                        makeUnion(int x, int y, bool changeOrder = true);
    bool                                        alreadyJoined(int x, int y);
    float                                       joinEntropy(int x, int y);
    int                                         find(int x);
    bool                                        join(synExp* s, bool changeOrder = true);
    // first is component start, second is component size;
    virtual std::set<std::pair<int, int> >      listComponents();
    std::set<std::pair<int, int> >              listChangedComponents();
    void                                        checkLengths();
    void                                        showInfo(int x);
    void                                        check();
    void                                        setUnchanged(int x);
    int                                         getLength(int x);
    float		                                computeEntropy();
    void                                        printComponents();
    void                                        vectorJoin(std::unordered_set<int>& vec);

    protected :
    
    bool                                        internalJoin(synExp* s, bool changeOrder);
    void                                        flip(int x);
    void                                        makeSet(int x);
    int                                         numMultiComponents, maxLength;
    std::vector<int>                            rank, parent, start, length, changed;
    std::vector<int>                            var2pos, pos2var;
    bool                                        outofsync;
};
#endif /* MultiComponents_hpp */
