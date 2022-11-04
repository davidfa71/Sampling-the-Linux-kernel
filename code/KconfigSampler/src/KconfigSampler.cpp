//
//  genrandom.cpp
//  myKconf
//
//  Created by david on 19/06/2018.
//  Copyright © 2018 david. All rights reserved.
//

#include "randomAlgo.hpp"
#include "cuddAdapter.hpp"
#include "Dictionary.hpp"

double first, last, now;

void usage(std::string name) {
        std::ostringstream ost;
        ost << name <<  ": Generates a random sampling of a BDD representing Kconfig files"                      << std::endl;
        ost << "Usage: " << name << " [-macros <file>] [-no-decoding] [-prob] [-seed num] [-v] <number of examples> <bdd file>" << std::endl;
        ost << "-macros <file> : Read the values in file as key=value fixed values, to enter the values corresponding to macro definitions" << std::endl;
        ost << "-no-decoding   : Perform random sampling only, no decoding to complex Kconfig types"             << std::endl;
        ost << "-prob          : Compute the probability of each data point"                                     << std::endl;
        ost << "-seed <num>    : Use <num> as a random seed (to reproduce results)"                              << std::endl;
        ost << "-v             : Verbose output (node probability progress)"                                     << std::endl;
        std::cerr << ost.str();
}

void writeProd(std::vector<bool>& prod) {
    for(auto b : prod)
      std::cout << b << " ";
    std::cout << std::endl;
}
void writeConfigFile(std::vector<KeyValue>& content) {
    for(auto kv : content) {
        std::string key   = kv.getKey();
        std::string value = kv.getValue();
        
        // boolean "n" values should not be used in a config file
        // because then the symbol would be defined in the C preprocessor
        // This will cause unnecessary compiling errors
        
        // On the other hand, if the conf interpreter is to be used,
        // then the symbol with value n should be written as comments,
        // because the conf interpreter reads also commented out lines
        // to see if a symbol is n.
        
        // But that of course would break the format of one .config per line
        // which is very practical.

	// Use the script extractConfig.sh for that

        //if (value != "n") {
            std::cout << "CONFIG_" << key << "=" << value << " ";
        //}
    }
    std::cout << std::endl;
}


int main(int argc, char** argv) {
    first = get_cpu_time();
    if (argc < 2 || argc > 9) {
        usage(argv[0]);
        exit(-1);
    }
    cuddAdapter *adapter         = new cuddAdapter();
    bool         computeProb     = false;
    int          verbose         = 0;
    unsigned int seed            = 0;
    int          i               = 1;
    bool         decoding        = true;
    std::string  macroValues     = "";
    while (i < argc-2) {
        if (std::string(argv[i]) == "-macros") {
            i++;
            macroValues = argv[i++];
            continue;
        }
        if (std::string(argv[i]) == "-no-decoding") { 
            decoding = false;
            i++;
            continue;
        }
	if (std::string(argv[i]) == "-v") {
            verbose = 1;
            i++;
            continue;
        }
        else
        if (std::string(argv[i]) == "-prob") {
            computeProb = true;
            i++;
            continue;
        }
        if (std::string(argv[i]) == "-seed") {
            seed  = atoi(argv[++i]);
            i++;
            continue;
        }
        throw std::logic_error("Unknown option "+std::string(argv[i]));
    }
    if (argc != i+2) {
        std::cerr <<  argc-i << " arguments. Two were expected" << std::endl;
        usage(argv[0]);
        exit(-1);
    }
    int num = atoi(argv[i++]);
#ifdef _DEBUG_
    std::cerr << "Reading BDD... " << argv[i];
#endif
    adapter->readBDD(argv[i], "");
#ifdef _DEBUG_
    std::cerr << " done" << std::endl;
#endif
    Dictionary dic;
    dic.gatherVariableInfo(adapter);
    tProduct prod;
    if (macroValues != "") {
#ifdef _DEBUG_
        std::cerr << "Reading macro values from " << macroValues << " ...";
#endif
        std::map<std::string, std::string> theMap = dic.readConfigFile(macroValues);
#ifdef _DEBUG_
	std::cerr << " done" << std::endl;
#endif
        prod = dic.encode(adapter, theMap);
    }
    else
        prod.resize(adapter->getNumVars());
    compProbabilities(verbose, adapter, seed);
    std::cerr << "Seed      : " << seed << std::endl;
    std::vector<KeyValue> toWrite;

    // The combination function that computes the probabilites relies on a side effect to
    // insert in a std::map which is not thread-safe, so multithreading is
    // out of the question
    prepareSampleShared(adapter, prod);
    for(int x = 0; x < num;x++) {
#ifdef _DEBUG_
	std::cerr << "Generating sample #" << x << std::endl;
#endif
        std::vector<bool> prod;
        if (computeProb)
            prod = genRandomComputeProb(adapter);
        else {
            prod = genRandomShared(adapter);
        }
        std::string environ;
        if (decoding) {
          toWrite = dic.decode(prod, environ);
          writeConfigFile(toWrite);
        }
        else
          writeProd(prod);
    }
    if (computeProb) {
        computeAverageAndVariance();
//            std::cerr << "Probability average:" << getAverage()
//                    << ", variance: " << getVariance() << ", typical deviation: " << sqrt(getVariance()) << std::endl;
    }
    std::cerr << "Total time: " << showtime(get_cpu_time() - first) << std::endl;
}
