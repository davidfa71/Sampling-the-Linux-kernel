//
//  cuddAdapter.h
//  myKconf
//
//  Created by david on 15/12/14.
//  Copyright (c) 2014 david. All rights reserved.
//

#ifndef __myKconf__cuddAdapter__
#define __myKconf__cuddAdapter__

#include "OneComponent.hpp"
#include "MultiComponents.hpp"
#include "humanNums.hpp"
#include "mytime.hpp"
#include "humanNums.hpp"

#include <fstream>
#include <mtr.h>
#include <limits>
#include <cuddObj.hh>
#include <cudd.h>
#include <dddmp.h>
#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <exception>

using namespace cudd;

class BddTooBig: virtual public std::exception {
    
protected:

    long bddSize;
    
public:

    /** Constructor (C++ STL string, int, int).
     *  @param msg The error message
     *  @param err_num Error number
     *  @param err_off Error offset
     */
    explicit
    BddTooBig(long s):
        bddSize(s)
        {}

    /** Destructor.
     *  Virtual to allow for subclassing.
     */
    virtual ~BddTooBig() throw () {}


    long size() const throw () {
       return bddSize;
    }
};
class cuddAdapter {
    public :
    
                                cuddAdapter();
                                cuddAdapter(double cacheMultiplier);
                                ~cuddAdapter();

    void                        useComponents();
    void                        useComponents(Components* pcomp);
    void                        useComponents(std::vector<int> var2pos,
                                              std::vector<int> pos2var);
    void                        init();
    int                         getNumComponents();
    int                         getMaxComponent();
    float                       getComponentEntropy();
    int                         getComponent(int x);
    int                         getCompSize(int x) ;
    int                         getNumVars();
    std::vector<int>            pos2var();
    std::vector<int>            var2pos();
    void                        changeOrder(std::vector<int>& pos2var);
    DdNode*                     getBDD();
    DdNode*                     getZero();
    DdNode*                     getOne();
    cudd::BDD                   getBddOne();
    bool                        isZero(const std::string& s);
    Cudd                        getCudd();
    int                         getLevel(DdNode const * node);
    int                         getLevel(DdNode* node);
    int                         varAtPos(int pos) const;
    int                         posOfVar(int var) const;
    void                        destroyQuantified(int n);
    void                        setValue(const std::string&, synExp*);
    void                        reorderComponent(std::vector<std::string>& ss,
                                                 std::string rmethod);
    void                        printBDD();
    void                        reorder(std::string reorderMethod);
    int                         addblock(std::string x);
    void                        initblocks();
    std::vector<std::string>    giveOrder();
    void                        namevar(std::string name, int index);
    void                        newVar(const std::string& var);
    void                        newVar(const std::string& var, int pos);
    const cudd::BDD             applyNonSupport(synExp* s);
    void                        apply(synExp* s);
    void                        applyLimit(synExp* s, long limitNodes);
    cudd::BDD                   apply(const std::string& s, synExp * exp);
    void                        dropUndo();
    void                        applyLimit(const std::string& s, synExp * exp, long limitNodes);
    int                         getSize();
    void                        doAnd(const std::string& s1,
                                    const std::string& s2,
                                    const std::string& s3);
    void                        andLimit(const std::string& s, BDD second, long limit);

    const   int                 nodecount();
    const   int                 nodecount(const std::string& s);
    void                        saveBDD(const std::string& base,
                                        const std::string &suffix);
    void                        saveBDDArray(const std::string& base,
                                             const std::string &suffix);
    void                        saveBDD(const std::string& id,
                                        const std::string& b,
                                        const std::string &s);
    void                        readBDD(const std::string& b,
                                        const std::string &s);
    void                        readBDD(const std::string& id,
                                        const std::string& b,
                                        const std::string &s);
    void                        readArrayBDD(const std::string& base,
                                             const std::string &suffix);
    bool                        sameBDD(const std::string& s1,
                                        const std::string& s2);
    static  int MAXVAR;
    Cudd    mgr;
    const   cudd::BDD           process(synExp* exp, long stopNodes = 0);
    const cudd::BDD             process_rec(synExp* exp);

    int                         getLevelNodes(int level) const;
    int                         getNumComponents()       const;
    void                        shuffle(const std::vector<int>& order);
    void                        shuffle(const std::vector<std::string>& order);
    void                        existentialQ(const std::set<std::string>& v);
    void                        destroyInternal(const std::set<std::string>& v);
    std::string                 getVarName(int x);
    int                         getVarIndex(const std::string& var);
    void                        reorderComponents(int nreorder);
    void                        checkComponents();
    void 			            checkOrder();
    void                        syncOrder();
    void                        setBlocks(const std::string& fileBlocks);
    cudd::BDD                   getVar(const std::string& var);
    void                        setVar(std::string bddName, cudd::BDD bdd);
    cudd::BDD                   getStorage(const std::string& var);
    void                        deleteBdd(const std::string& var);
    bool                        isDefined(const std::string& var);
    std::map<std::string, Cudd_ReorderingType> reorderMap;
    const std::vector<DdNode*>& getFunctionHeads();
    void                        updateFunctionHeads();
    void                        addStringHead(std::string head);
    const std::vector<std::string>& getStringHeads();
    void                        clearStringHeads();
    void                        undoApply();
    void                        undoApply(const std::string& s);
    cudd::BDD                   getNULL();
    bool                        join(synExp* s);
private:
    
    void                                auxXOR(synExp* exp);
    void                                internalRefs();
    std::map<std::string, cudd::BDD>    vars;
    std::map<std::string, int>          indices;
    std::vector<std::string>            inVars;
    std::vector<std::set<int>>          groups;
    std::vector<bool>                   startComponent;
    int                                 numComponents;
    bool                                withComponents;
    int                                 numVars;
    std::unordered_map<std::string, cudd::BDD>    storage;
    int                                 reorderCounter;
    void                                setComponentBlocks(MtrNode* root,
                                                           int start,
                                                           int length);
    void                                readVarsAndComps(const std::string& filename);
    cudd::BDD                           auxReadBDD(const std::string& base,
                                                   const std::string &suffix);
    int                                 nroots;
    Components*                         pcomp;
    std::vector<int>                    levelnodes;
    int                                 countVar = 0;
    cudd::BDD                           theBDD, oldBDD;
    std::vector<DdNode*>                functionHeads;
    std::vector<std::string>            stringHeads;
    long                                maxNodes = std::numeric_limits<long>::max();;
};

#endif /* defined(__myKconf__cuddAdapter__) */
