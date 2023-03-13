/*************************************************************************
 * Copyright 2022, Lee B. Hinkle, all rights reserved
 * Based on prior work by Martin Burtscher and Molly O'Neil
 * Redistribution in source or binary form, with or without modification,
 * is *not* permitted. Use in source or binary form, with or without
 * modification, is only permitted for academic use in CS 3339 at
 * Texas State University.
 ************************************************************************/
#ifndef __CACHE_STATS_H
#define __CACHE_STATS_H

#include <cstdint>
#include "Debug.h"
#include <vector>
using namespace std;

#ifndef CACHE_EN
#define CACHE_EN 1
#endif

#ifndef BLOCKSIZE
#define BLOCKSIZE 32
#endif

#ifndef SETS
#define SETS 8
#endif

#ifndef WAYS
#define WAYS 4
#endif

#ifndef LOOKUP_LATENCY
#define LOOKUP_LATENCY 0
#endif

#ifndef READ_LATENCY
#define READ_LATENCY 30
#endif

#ifndef WRITE_LATENCY
#define WRITE_LATENCY 10
#endif

enum ACCESS_TYPE { LOAD, STORE };

class CacheStats {
  private:
    /* TODO: you probably want to add some member variables here to represent
     * the parts of the cache contents you need to model! */
     enum VALIDATION {INVALID, VALID};
     enum BLOCKSTATE {CLEAN, DIRTY};
     vector<uint32_t> cacheTag;
     vector<bool> oldBlock;
     vector<bool> validBit;
     vector<int> nextWay;

    int loads;
    int stores;
    int load_misses;
    int store_misses;
    int writebacks;

  public:
    CacheStats();
    int access(uint32_t, ACCESS_TYPE);
    void printFinalStats();
};

#endif
