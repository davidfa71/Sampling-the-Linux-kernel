//
//  FastSifter.hpp
//  fastOrder
//
//  Created by David Fernandez Amoros on 07/10/2019.
//  Copyright © 2019 David Fernandez Amoros. All rights reserved.
//

#ifndef FastSifter_hpp
#define FastSifter_hpp

#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include "synExp.hpp"
#include "synExpDriver.hpp"
#include "MultiComponents.hpp"
#include "mytime.hpp"
#include "humanNums.hpp"
#include "eval.hpp"

typedef long score_t;
class FastSifter {
  public:
    FastSifter(bool verbose,
               std::string varFile,
               std::string expFile,
               std::string _outputFile,
               std::string _histogram,
               std::string blocks,
               double _threshold,
               bool nosimplify,
               bool noGrouping,
               bool noSubExp,
               int _window = 0);
    FastSifter(std::vector<std::string>&        _variables,
               std::vector<synExp*>&            _expList,
               double                           _threshold,
               bool                             _nosimplify,
               bool                             _noGrouping,
               bool                             _noSubExp,
               int                              _window);
    void goSifting();
    void goPerm();
    void writeResults();
    void init();
    std::vector<std::string> giveResults();
  private:
    int  deleted = 0;
    void addSubExpressions(synExp* e);
    void findEquivalent();
    void substituteVariable(std::string toDelete, std::string other);
    void fillReverseIndex();
    std::set<std::pair<std::string, std::string>> equiSet;
    void aImpliesB(std::string a, std::string b);
    std::unordered_map<std::string, std::vector<std::string>>         rewriteRules;
    std::unordered_map<std::string, std::set<std::string>>            xRelatedVars;
    std::unordered_map<std::string, std::set<std::set<std::string>*>> xReverseIndex;
    std::unordered_map<std::string, std::string>                      representedBy;
    bool                            stillExists(std::string v);
    void                            groupVariables();
    std::string                     outputFile;
    void                            prepVarOrdering();
    void                            performPerm();
    void                            performSifting();
    std::unordered_set<int>         compVectors;
    bool                            nosimplify;
    bool                            noGrouping;
    bool                            noSubExp;
    double                          threshold;
    std::vector<std::string>        variables;
    std::vector<synExp*>            expList, origExpList;
    std::vector<int>                index2pos, pos2index, vSpan, bestvSpan, bestIndex2pos, bestPos2Index;
    std::unordered_map<std::string, int>      varMap;
    std::vector<std::unordered_set<int>>     expressions, expressionsVar, relatedVars;
    MultiComponents               *pcomp;
    score_t                        score;
    std::unordered_map<int, std::unordered_map<int, std::unordered_set<int>>> intersection;
    // minVector[pos] : expressions whose minimum position is pos
    std::vector<std::unordered_set<int>*>    minVector, maxVector, bestminVector, bestmaxVector;
    std::vector<score_t>                     scoreTable;
    bool                                     verbose;
    int                                      window, members;
    unsigned long                            numSwaps;
    // Left move is the biggest movement to left in a particular permutation
    std::vector<std::vector<int>>           permSeq,  leftMove;
    std::vector<std::vector<std::vector<int>>>          pathBack;
    std::string                                 histogram;
    std::string                                 blocks;
    
    void    readInfo(std::string varFile, std::string expFile);
    void    makeExpressionsVar();
    void    computeComponents();
    void    computeIntersections();
    score_t computeAllScore();
    score_t computeScore(std::unordered_set<int>& vexp);
    score_t computeScore(int exp);
    score_t giveScore(int span);
    int     computeSpan(int exp);
    void    initSwap();
    void    siftComponent(int start, int length);
    void    sift(int pos, int start, int length);
    score_t swapPos(int pos);
    void    printExpressionsVar(int var);
    void    printmaxVector(int pos);
    void    generateSequence(int n);
    void    sliding_window(int start, int length);
    int     applyBestPerm(int pos);
    int     applyPerms(int pos, score_t& bestScore, int& bestPermNum);
    void    checkSpan();
    int     maxLeftMove(std::vector<int>& v);
    std::vector<synExp*> simplifyConstraints(std::vector<synExp*> pendingConstraints);
    void    addSymbols2Table(std::vector<std::string> vars);
    void    computeRelatedVars();
    void    goBackToBest(int bestPermNum, int startPos);
    void    printVector(std::vector<int>& v);
    std::vector<int> goBackSeq(std::vector<int> lastPerm,
                               const std::vector<int>& thisPerm);
    bool    lookForVariable(std::string var);

};
#endif /* FastSifter_hpp */
