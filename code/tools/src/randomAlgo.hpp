

#ifndef randomAlgo_hpp
#define randomAlgo_hpp

#include <stdio.h>
#include <sstream>
#include <gmpxx.h>
#include <gmp.h>
#include <set>
#include "cuddAdapter.hpp"
#include "Traverser.hpp"

extern int verbose;

int                              compProbabilities(int verbose,
                                                    cuddAdapter* a,
                                                    unsigned int& seed);
int                              compProbabilitiesMP(int verbose,
                                                      int threads,
                                                      cuddAdapter* a,
                                                      unsigned int& seed,
                                                      bool fast);
void                              drawGraph(cuddAdapter* a);
void                              computeAverageAndVariance();
double                            getAverage();
double                            getVariance();
std::vector<bool>                 genRandomComputeProb(cuddAdapter* a);
bool                              validateProduct(cuddAdapter* a,
                                                  const std::vector<bool>& b);
std::vector<bool>                 genRandom(cuddAdapter* a);
void                              prepareSampleShared(cuddAdapter *a);
std::vector<bool>                 genRandomShared(cuddAdapter* a);
std::string                       nameRandom(cuddAdapter *a);
std::string                       nameRandomShared(cuddAdapter *a);
std::string                       nameProduct(cuddAdapter*a, std::vector<bool> v);

#endif /* randomAlgo_hpp */

