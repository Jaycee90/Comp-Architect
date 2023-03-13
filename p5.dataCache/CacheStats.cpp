/***********************************************************************
 * Submitted by: Jean Claude Turambe, jnn56
 * CS 3339 - Fall 2022, Texas State University
 * Project 5 Data Cache
 * Copyright 2021, Lee B. Hinkle, all rights reserved
 * Based on prior work by Martin Burtscher and Molly O'Neil
 * Redistribution in source or binary form, with or without modification,
 * is *not* permitted. Use in source or binary form, with or without
 * modification, is only permitted for academic use in CS 3339 at
 * Texas State University.
 ************************************************************************/
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "CacheStats.h"
using namespace std;

CacheStats::CacheStats() {
	cout << "Cache Config: ";
	if(!CACHE_EN) {
		cout << "cache disabled" << endl;
  	} else {
    	cout << (SETS * WAYS * BLOCKSIZE) << " B (";
    	cout << BLOCKSIZE << " bytes/block, " << SETS << " sets, " << WAYS << " ways)" << endl;
   		cout << "  Latencies: Lookup = " << LOOKUP_LATENCY << " cycles, ";
    	cout << "Read = " << READ_LATENCY << " cycles, ";
    	cout << "Write = " << WRITE_LATENCY << " cycles" << endl;
  	}
    
    loads = 0;
    stores = 0;
    load_misses = 0;
    store_misses = 0;
    writebacks = 0;
    
    //initializing Data structures.
    cacheTag = vector<uint32_t>(SETS * WAYS, 0);
    oldBlock = vector<bool>(SETS * WAYS, CLEAN);
    validBit = vector<bool>(SETS * WAYS, VALID);
    nextWay = vector<int>(SETS, 0);
}

int CacheStats::access(uint32_t addr, ACCESS_TYPE type) {
    if(!CACHE_EN) { // latency if the cache is disabled
        return (type == LOAD) ? READ_LATENCY : WRITE_LATENCY;
    }

    // Extract index and Tag from the address
    uint32_t index = (addr >> 5) & 0x07;//3bits of index
    uint32_t oldWay = index * WAYS;
    uint32_t tag = (addr >>  8);//(32-3-3-2 = 24bits of tag)

    // Mark cache ready if it is hit
    bool hit = false;
    int i = 0;//starting idle
	while(i < WAYS && !hit){
		if (tag == cacheTag[oldWay + i]){
			hit = true;
		}else{
			i = i + 1;
		}
	}

    // Update cache on a hit
    //start by load word then store word
    if (hit) {
        if (type == LOAD) {
            ++loads;
            return LOOKUP_LATENCY;
        } else {
            ++stores;
            oldBlock[oldWay + i] = DIRTY;
            return LOOKUP_LATENCY;
        }
    } else {
        // which way to kick out?
        //Round robin bits point to the oldest way
        oldWay += nextWay[index];
        nextWay[index] = ( ++nextWay[index] ) % WAYS;
        cacheTag[oldWay] = tag;

        // Comparing Tags and update cache miss
        if (type == LOAD){
            ++loads;
            ++load_misses;
            //cache miss and old block is clean
            if (oldBlock[oldWay] == CLEAN) {
                validBit[oldWay] = VALID;
                return READ_LATENCY;
            } else { 
            //cache miss and old block is dirty
                writebacks++;
                oldBlock[oldWay] = CLEAN;//write old block to memory
                return WRITE_LATENCY  + READ_LATENCY;
            }
        } else {
            ++stores;
            ++store_misses;
            //cache miss and old block is clean
            if (oldBlock[oldWay] == CLEAN) {
                oldBlock[oldWay] = DIRTY;//allocate and read new block from memory
                validBit[oldWay] = VALID;
                return READ_LATENCY; 
            } else {
            	//cache miss and old block is dirty
                writebacks++;
                return WRITE_LATENCY  + READ_LATENCY;
            }
        }
    }
}

void CacheStats::printFinalStats() {
  //Draining the cache of writebacks 
    int wbDrain = 0;
    for (int i = 0; i < SETS * WAYS; i++){
    	if (oldBlock[i] == DIRTY){
    		wbDrain++;
		}
	}
	writebacks += wbDrain;
  	     
  int accesses = loads + stores;
  int misses = load_misses + store_misses;
  cout << "Accesses: " << accesses << endl;
  cout << "  Loads: " << loads << endl;
  cout << "  Stores: " << stores << endl;
  cout << "Misses: " << misses << endl;
  cout << "  Load misses: " << load_misses << endl;
  cout << "  Store misses: " << store_misses << endl;
  cout << "Writebacks: " << writebacks << endl;
  cout << "Hit Ratio: " << fixed << setprecision(1) << 100.0 * (accesses - misses) / accesses;
  cout << "%";
}
