//
//  bddBuilder.hpp
//  myKconf
//
//  Created by david on 11/10/14.
//  Copyright (c) 2014 david. All rights reserved.
//

#ifndef __myKconf__bddBuilder__
#define __myKconf__bddBuilder__

#include <string>
#include <set>
#include <list>
#include <map>
#include <vector>
#include <iomanip>
#include <tuple>
#include <fstream>
#include <sstream>
#include <iostream>

#include "synExp.hpp"
#include "humanNums.hpp"
#include "mytime.hpp"
#include "constraintSorter.hpp"
#include "minspanSorter.hpp"
#include "smartspanSorter.hpp"
#include "expSorter.hpp"
#include "compSorter.hpp"
#include "Components.hpp"
#include "RememberSorter.hpp"

class bddBuilder {
    
    public:
    
    bddBuilder(std::vector<synExp*>& consList);
    ~bddBuilder();
    void setUndo(bool u)                        { doUndo               = u;    }
    void setStopNodes(int sn)                   { stopNodes            = sn;   }
    void setAdapter(cuddAdapter* f)             { adapter              = f;    }
    void setFunctionName(std::string f)         {
        functionName         = f;
        adapter->addStringHead(f);
    }
    void setVarFileName(std::string s)          { varFileName          = s;    }
    void setStatsFileName(std::string s)        { statsFileName        = s;    }
    void setTempBDDName(std::string s)          { tempBDDName          = s;    }
    void setAppliedExpFilename(std::string s)   { appliedExpFilename   = s;    }
    void setUnappliedExpFileName(std::string s) { unappliedExpFilename = s;    }
    void setReorderMethod(std::string s)        { reorderMethod        = s;
                            if (s == "noreorder") reorder = false;             }
    void setReorderInterval(int x)              { reorderInterval      = x;    }
    void setMinNodes(int x)                     { minNodes             = x;    }
    void setMaxNodes(int x)                     { maxNodes             = x;    }
    void setMinConstraint(int x)                { minConstraint        = x;    }
    void setReorderEvery(int x)                 { reorderEvery         = x;    }
    void setLineLength(int l)                   { lineLength           = l;    }
    void setVerboseOff()                        { verbose              = false;}
    void setConstraintSorter(constraintSorter* cs) { myOrder = cs; };

    void build(double inflation);
    bool syncOrder();
    void printPartOne(synExp* pconst, int counter, int origSize);
    void printPartTwo(int nn);
    void reorderComponents(int n);
    bool isReorderTime(int counter, int lastnodes, int nn, int constLastReorder, double inflation);
    bool isAlmostReorderTime(int lastNodes, int nn, double inflation);
    void reorderProtocol(std::vector<synExp*>::iterator itl);
    int  nodecount();
    void writeApplied();
    void writeUnApplied(std::vector<synExp*>::iterator itl);
    void apply(synExp* pconst);
    void undo();
    void exchange(int counter, int minCounter);
    
    private:
    
    void tallyHeads(std::vector<std::pair<int, std::string>>& h);
    int computeNodes(std::vector<std::pair<int, std::string>>& h);
    void reComputeHeap(std::vector<std::pair<int, std::string>>& h);
    std::string varfilename, statsfile, tempBDDName, appliedExpFilename, unappliedExpFilename;
    bool                        doUndo = true;
    std::vector<synExp*>        appliedConstraints;
    std::string                 functionName;
    bool                        verbose = true;
    std::map<std::string, int>  occurrences, originalPos;
    constraintSorter*           myOrder;
    template<class T> void      fastReorderConstraints(T beg, T end);
    std::set<std::string>       internal;
    bool                        reorder;
    void                        writeNameOrder(const std::vector<std::string>& ord);
    std::string                 varFileName, statsFileName, reorderMethod;
    int                         reorderInterval, reorderEvery, reorderFromConst = 0;
    int                         minNodes, maxNodes, minConstraint;
    int                         stopNodes = std::numeric_limits<int>::max();
    int                         lineLength = 95;
    cuddAdapter*                adapter;
    std::ofstream               varFile, statsFile;
    std::vector<synExp*>&       consList;
    
    void                        namevar(std::string name, int index);
    void                        printOrder(std::vector<int> ord);
    void                        readVarOrder();
    void                        showVarsPositions(synExp* s);
    
};
#endif /* defined(__myKconf__bddBuilder__) */
