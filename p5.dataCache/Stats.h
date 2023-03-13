/******************************
 * Copyright 2022, Lee B. Hinkle, all rights reserved
 * Based on prior work by Martin Burtscher and Molly O'Neil
 * Redistribution in source or binary form, with or without modification,
 * is *not* permitted. Use in source or binary form, with or without
 * modification, is only permitted for academic use in CS 3339 at
 * Texas State University.
 ******************************/
#ifndef __STATS_H
#define __STATS_H
#include <iostream>
#include <iomanip>
#include "Debug.h"
using namespace std;

enum PIPESTAGE { IF1 = 0, IF2 = 1, ID = 2, EXE1 = 3, EXE2 = 4, MEM1 = 5, 
                 MEM2 = 6, WB = 7, PIPESTAGES = 8 };

class Stats {
  private:
    long long cycles;
    int flushes;
    int stalls;
    int bubbles;

	  int memops;
    int branches;
    int taken;
	
	  int rawHazards[PIPESTAGES];//RAW hazards tracking
    int resultReg[PIPESTAGES];
    int resultStage[PIPESTAGES];//the mirror of resultReg []
    

  public:
    Stats();

    void clock(PIPESTAGE pipeStage);
    void stall(int stallCount);

    void flush(int count);

    void registerSrc(int r,PIPESTAGE needed);
    void registerDest(int r,PIPESTAGE available);

    void countMemOp() { memops++; }
    void countBranch() { branches++; }
    void countTaken() { taken++; }
	
    void showPipe();

    // getters
    long long getCycles() { return cycles; }
    int getFlushes() { return flushes; }
    int getStalls(){return stalls; }
    int getBubbles() { return bubbles; }
    int getMemOps() { return memops; }
    int getBranches() { return branches; }
    int getTaken() { return taken; }

    int getHazards(PIPESTAGE pipeStage){return rawHazards[pipeStage]; }//track the hazards into each pipeline
	  int getTotalHazards(){//report the total harzards occured into the pipeline
       return rawHazards[EXE1] + rawHazards[EXE2] + rawHazards[MEM1] + rawHazards[MEM2];
     }
	
  private:
    void bubble();
};

#endif
