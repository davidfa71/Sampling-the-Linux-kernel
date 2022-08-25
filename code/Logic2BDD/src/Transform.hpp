// $Id: driver.h 17 2007-08-19 18:51:39Z tb $ 	
/** \file driver.h Declaration of the kconf::Driver class. */

#ifndef KCONF_TRANSFORM_H
#define KCONF_TRANSFORM_H

#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <tuple>

#include "PermScorer.hpp"
#include "OtherScorer.hpp"
#include "RandomScorer.hpp"
#include "forceBlockScorer.hpp"
#include "SiftScorer.hpp"
#include "forceScorer.hpp"
#include "maxOccSorter.hpp"
#include "minOccSorter.hpp"
#include "minmaxSorter.hpp"
#include "minspanSorter.hpp"
#include "nullSorter.hpp"
#include "RememberSorter.hpp"
#include "expSorter.hpp"
#include "compSorter.hpp"
#include "smartspanSorter.hpp"
#include "synExp.hpp"
#include "eval.hpp"
#include "cuddAdapter.hpp"
#include "MultiComponents.hpp"
#include "OneComponent.hpp"
#include "orderHelper.hpp"
#include "synExpDriver.hpp"
#include "bddBuilder.hpp"
#include "FastSifter.hpp"

#include "simplify.hpp"

class Transform
{

private:
    
    Components*                     pcomp;
    std::set<std::string>           existentialVars;
    std::vector<synExp*>            expList;
    std::unordered_map<std::string, int>      varMap;
    std::vector<std::string>        theVector, variables, ordering;
    std::vector<int>                pos2var, var2pos;
    void computeComponents();
    void checkErrors();
    bool fast = false;
    void configureBuilder(bddBuilder& b);
    std::string varfilename, statsfile, tempBDDName, appliedExpFilename, unappliedExpFilename;
    void setFileNames();
    void prepareContinueBuilding();
    void setUpAdapter(bool xverbose);
    void minimalTreatment();
  public:
    Transform();
    int                             stopNodes = std::numeric_limits<int>::max();
    bool                            doUndo    = false;
    std::string                     contBDD;

    std::string                     base;
    std::string                     blockFile;
    int                             window            = 0;
    double                          scoreThreshold    = 1.0;
    bool                            doBuildManyBDDs   = false;
    bool                            simplify          = true;
    bool                            static_components = true;
    bool                            dynamic_components= true;
    bool                            expand            = false;
    bool                            doCNF             = false;
    bool                            scoreReorder      = false;
    double                          inflation         = 10;
    bool                            simulate          = false;
    bool                            blocks            = true;
    std::string                     constReorder      = "none";
    std::string                     scoreMethod       = "none";
    std::string                     varOrderFile;
    bool                            pickup            = false;
    int                             reorderEvery      = 1;
    int                             reorderInterval   = 0;
    int                             minConstraint     = 0;
    int                             minNodes          = 0;
    int                             lineLength        = 60;
    int                             maxNodes          = std::numeric_limits<int>::max();
    double                          cacheMultiplier   = 1;

    int                             slidingWindow;
    std::string                     reorderMethod, suffix;
    std::string                     adapterType = "cudd";
    cuddAdapter                     *adapter;
    constraintSorter*               newCnstOrder(const std::string & s);
    void    coreBuild(std::vector<std::string>& vars, std::vector<synExp*>& exps);
    void    buildManyBDDs();
    bool    fastBuild(std::vector<std::string>& vars, std::vector<synExp*>& exps);
    void    varsAndExp(std::vector<std::string>& vars, std::vector<synExp*>& exps);
    void    readVars(std::string varFile);
    void    readExp(std::string expFile);
  
    void treatVarsAndConstraints();

    void buildBDD(bool verbose);
    void buildBDD();
    void addToForest(std::vector<synExp*>& exps,
                     std::string& varname,
                     int c);
    bool reorder = true;
    bool verbose = true;

    // Process default statements
    
};



#endif // KCONF_TRANSFORM_H
